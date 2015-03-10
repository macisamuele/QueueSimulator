/*
 * Copyright 2014 Samuele Maci (macisamuele@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "event.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errors_warnings_management.h"
#include "simulation_time.h"

#ifdef DBG
static int n_id=0;
#endif

struct event_t {
	Event next;
#ifdef DBG
	int id;
#endif
	Time scheduled, arrival;
	EventType type;
};
struct eventlist_t {
	Event head;
};

///useful to reduce the number of malloc calls => speedup the global execution
static Event event_bin = NULL;

/**
 * Description: initialize the event setting the scheduled time and the type
 * Return: the new event
 */
Event event_init(Time scheduled, EventType type) {
	Event event;
	if (event_bin != NULL) {
		event = event_bin;
		event_bin = event_bin->next;
	} else {
		_alloc(event, Event, 1);
	}
#ifdef DBG
	event->id = n_id++;
#endif
	event->scheduled = scheduled;
	if(type==ARRIVAL) {
    	event->arrival = scheduled;
	}
	else {
    	event->arrival = 0.0;
	}
	event->type = type;
	event->next = NULL;
	return event;
}
/**
 * Description: set the scheduled time for the event
 */
void event_setScheduled(Event event, Time scheduled) {
	return_if_null("event", event,);
	event->scheduled = scheduled;
}
/**
 * Description: set the arrival time for the event
 */
void event_setArrival(Event event, Time arrival) {
	return_if_null("event", event,);
	event->arrival = arrival;
}
/**
 * Description: set the type for the event
 */
void event_setType(Event event, EventType type) {
	return_if_null("event", event,);
	event->type = type;
}
/**
 * Return: the scheduled time for the event
 */
Time event_getScheduled(Event event) {
	return_if_null("event", event, WRONG_TIME);
	return event->scheduled;
}
/**
 * Return: the arrival time for the event
 */
Time event_getArrival(Event event) {
	return_if_null("event", event, WRONG_TIME);
	return event->arrival;
}
/**
 * Return: the type for the event
 */
EventType event_getType(Event event) {
	return_if_null("event", event, UNKNOWN);
	return event->type;
}
/**
 * Return: -1 if event1 is previous of event2
 * 			0 if event1 and event2 are equal
 * 			1 if event1 is following of event2
 * 	REMARK: event1 if previous of event2 if it was scheduled before,
 * 		in case in which the events have the same scheduled time
 * 		event1 is previous if it arrived before
 * 		in case in which the events have the same arrival time
 * 		event1 is previous if is a DEPARTURE
 */
int event_compare(Event event1, Event event2) {
	int res;
	return_if_null("event1", event1, 0);
	return_if_null("event2", event2, 0);
	res = precedence(event1->scheduled, event2->scheduled);
	if (res == 0) {
		res = precedence(event1->arrival, event2->arrival);
	}
	if (res == 0) {
		if (event1->type == DEPARTURE) {
			res = -1;
		} else if (event2->type == DEPARTURE) {
			res = 1;
		} else {
			res = 0;
		}
	}
	return res;
}
/**
 * Return: event_compare((Event)event1, (Event)event2)
 */
int event_compare_void(void *event1, void *event2) {
	return event_compare((Event)event1, (Event)event2);
}
/**
 * Description: delete the event, after the operation is forbidden to use the event
 */
void event_free(Event event) {
	return_if_null("event", event,);
	event->next = event_bin;
#ifdef DBG
	event->id = -1;
#endif
	event->scheduled = WRONG_TIME;
	event->arrival = WRONG_TIME;
	event_bin = event;
}
/**
 * Description: call event_free((Event)event)
 */
void event_free_void(void *event) {
	event_free((Event) event);
}
/**
 * Description: release all the memory required for the events managing,
 * not release all the events but release the events previously freed
 */
void event_free_memory() {
	static Event event;
	while ((event = event_bin) != NULL) {
		event_bin = event_bin->next;
		free(event);
	}
}
/**
 * Description: print on the stream the event
 */
void event_print(FILE *stream, Event event) {
	return_if_null("event", event,);
	if (stream == NULL || stream == stdin) {
		stream = stdout;
	}
#ifdef DBG
	fprintf(stream, "ID: %3d | "event->id);
#endif
	fprintf(stream, "Type: %s | Scheduled: %.6f | Arrival: %.6f",
			event->type == ARRIVAL ?
					"A" : (event->type == DEPARTURE ? "D" : "U"),
			event->scheduled, event->arrival);
}
///////////////////////////////////////////////////////////
/**
 * Description: initialize a new ordered list of events
 * Return: the new list
 */
EventList eventlist_init() {
	EventList eventlist;
	_alloc(eventlist, EventList, 1);
	eventlist->head = NULL;
	return eventlist;
}
/**
 * Description: insert the event on the eventlist
 */
void eventlist_insert(EventList eventlist, Event event) {
	return_if_null("eventlist", eventlist,);
	return_if_null("event", event,);
	static Event tmp;
	if (eventlist->head == NULL) {
		eventlist->head = event;
	} else {
		if (event_compare(event, eventlist->head) < 0) {
			event->next = eventlist->head;
			eventlist->head = event;
		} else {
			tmp = eventlist->head;
			while (tmp->next != NULL && event_compare(event, tmp->next) > 0) {
				tmp = tmp->next;
			}
			event->next = tmp->next;
			tmp->next = event;
		}
	}
}
/**
 * Description: extract the first event from eventlist
 * Return: the extracted event
 */
Event eventlist_extract(EventList eventlist) {
	static Event result;
	return_if_null("eventlist", eventlist, NULL);
	if (eventlist->head == NULL) {
		return NULL;
	}
	result = eventlist->head;
	eventlist->head = eventlist->head->next;
	result->next = NULL;
	return result;
}
/**
 * Description: release all the events of the list, after the operation is forbidden to use the eventlist
 */
void eventlist_free(EventList eventlist) {
	static Event event;
	return_if_null("eventlist", eventlist,);
	while ((event = eventlist->head) != NULL) {
		eventlist->head = eventlist->head->next;
		event_free(event);
	}
	free(eventlist);
}
/**
 * Description: call eventlist_free((Event)eventlist)
 */
void eventlist_free_void(void *eventlist) {
	eventlist_free((EventList) eventlist);
}
/**
 * Description: release all the memory required for the eventlist managing,
 * not release all the eventlists but release the eventlist previously freed
 */
void eventlist_free_memory() {
	event_free_memory();
}
/**
 * Description: check if the eventlist is empty
 * Return: 1 if the list is not allocated or not contains event, 0 otherwise
 */
int eventlist_is_empty(EventList eventlist) {
	return eventlist == NULL || eventlist->head == NULL ? 1 : 0;
}
/**
 * Description: print on the stream the eventlist
 */
void eventlist_print(FILE *stream, EventList eventlist) {
	static Event event;
	return_if_null("eventlist", eventlist,);
	event = eventlist->head;
	while (event != NULL) {
		event_print(stream, event);
		fprintf(stream, "\n");
		event = event->next;
	}
}
