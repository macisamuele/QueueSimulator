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

#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED

#include <stdio.h>

#include "simulation_time.h"

typedef enum {
	UNKNOWN, ARRIVAL, DEPARTURE, EXIT
} EventType;
typedef struct event_t *Event;
typedef struct eventlist_t *EventList;

Event event_init(Time scheduled, EventType type, unsigned long queue_id);
void event_setScheduled(Event event, Time scheduled);
void event_setQueueArrival(Event event, Time arrival);
void event_setSystemArrival(Event event, Time arrival);
void event_setType(Event event, EventType type);
void event_setQueueID(Event event, unsigned long queue_id);
Time event_getScheduled(Event event);
Time event_getQueueArrival(Event event);
Time event_getSystemArrival(Event event);
unsigned long event_getQueueID(Event event);
EventType event_getType(Event event);
int event_compare(Event event1, Event event2);
int event_compare_void(void *event1, void *event2);
void event_free(Event event);
void event_free_void(void *event);
void event_free_memory();
void event_print(FILE *stream, Event event);

EventList eventlist_init();
void eventlist_insert(EventList eventlist, Event event);
Event eventlist_extract(EventList eventlist);
void eventlist_free(EventList eventlist);
void eventlist_free_void(void *eventlist);
void eventlist_free_memory();
int eventlist_is_empty(EventList eventlist);
void eventlist_print(FILE *stream, EventList eventlist);

#endif // EVENT_H_INCLUDED
