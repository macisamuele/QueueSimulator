#include "Queue.h"
#include "event.h"
#include "server.h"

struct Queue_t {
//input parameters
	Server server;	//information about the kind of the server
					//allow to generate the correct service time
	unsigned long number_of_servers; //number of servers in the system
	Time *current_time;	//pointer of the current_time (value of the world)
	EventList *scheduled_events;	//events scheduled (on the world)
//internal variables
	EventList waiting_events;	//contains the events in the waiting line
	unsigned long busy_servers;	//number of server currently busy
	Time last_event_time;	//time in which occur the last event
	unsigned long total_users;	//number of users currently in the system
	Time last_arrival_time;	//time in which occur the last ARRIVAL event
//measures
	unsigned long number_of_users;	//number of users entered in the queue
	unsigned long number_of_services;	//number of ended services
	Time cumulative_inter_arrival_time;	//summation of all the inter arrival time
	Time cumulative_service_time;//summation of all the service time of all the events putted in service
	Time cumulative_response_time;//summation of all the waited+service time (queueing delay) of all the events putted in service
};

void queue_generate_departure(Queue queue, Event event);

/**
 * Description: allocate a M/G/k queue
 * Return: the new queue
 */
Queue queue_init(unsigned long number_of_servers, Server server, Time *current_time, EventList *scheduled_events) {
    return_if_null("current_time", current_time, NULL);
    return_if_null("scheduled_events", scheduled_events, NULL);
    return_if_null("server", server, NULL);
    if(number_of_servers==0) {
        return NULL;
    }
	Queue queue;
	_alloc(queue, Queue, 1);
	queue->server = server;
	queue->number_of_servers = number_of_servers;
	queue->current_time = current_time;
	queue->scheduled_events = scheduled_events;
	queue->waiting_events = eventlist_init();
    queue->last_arrival_time = 0.0;
	queue->total_users = 0;
	queue->busy_servers = 0;
	queue->last_event_time = 0.0;
	queue->number_of_services = 0;
	queue->cumulative_inter_arrival_time = 0.0;
	queue->cumulative_response_time = 0.0;
	queue->cumulative_service_time = 0.0;
	queue->number_of_users = 0;
	return queue;
}
/**
 * Description: release all the memory required from the queue
 */
void queue_free(Queue queue) {
	return_if_null("queue", queue,);
	eventlist_free(queue->waiting_events);
	event_free_memory();
	free(queue);
}

#define getCurrentTime(queue) (*queue->current_time)
#define schedule_event(queue, event) eventlist_insert(*(queue->scheduled_events), event)

/**
 * Description: manage the queue behavior in case of an arrival event
 */
Event queue_manage_arrival(Queue queue, Event event) {
	return_if_null("queue", queue, NULL);
	return_if_null("event", event, NULL);
	event_setQueueArrival(event, getCurrentTime(queue));
	queue->number_of_users++;
	queue->cumulative_inter_arrival_time += getCurrentTime(queue) - queue->last_arrival_time;
	queue->last_arrival_time = getCurrentTime(queue);
	queue->total_users++;
	if (queue->busy_servers < queue->number_of_servers) {
		queue_generate_departure(queue, event);
	} else {
		eventlist_insert(queue->waiting_events, event);
	}
	return event;
}
/**
 * Description: manage the queue behavior in case of a departure event
 */
Event queue_manage_departure(Queue queue, Event event) {
	return_if_null("queue", queue, NULL);
	return_if_null("event", event, NULL);
	queue->busy_servers--;
	queue->total_users--;
	queue->number_of_services++;
	event_setQueueArrival(event, getCurrentTime(queue));
	event_setType(event, EXIT);
	if (!eventlist_is_empty(queue->waiting_events)) {
		queue_generate_departure(queue, eventlist_extract(queue->waiting_events));
	}
	return event;
}
/**
 * Description: generate the departure of the event from the queue
 */
void queue_generate_departure(Queue queue, Event event) {
	return_if_null("queue", queue,);
	return_if_null("event", event,);
	static double duration;
	duration = server_getServiceTime(queue->server);
	event_setType(event, DEPARTURE);
	event_setScheduled(event, getCurrentTime(queue) + duration);
	schedule_event(queue, event);
	queue->busy_servers++;
	queue->cumulative_response_time += getCurrentTime(queue) - event_getQueueArrival(event) + duration;
	queue->cumulative_service_time += duration;
}
/**
 * Description: manage the queue behavior in case of a new event
 * Return: the managed event (NULL in case of error)
 */
Event queue_manage_event(Queue queue, Event event) {
	return_if_null("queue", queue, NULL);
	return_if_null("event", event, NULL);
	switch (event_getType(event)) {
	case ARRIVAL:
		return queue_manage_arrival(queue, event);
		break;
	case DEPARTURE:
		return queue_manage_departure(queue, event);
		break;
	default:
		return NULL;
	}
}
unsigned long queue_getNumberOfUsersGenerated(Queue queue) {
    return_if_null("queue", queue, 0);
    return queue->number_of_users;
}
unsigned long queue_getNumberOfServices(Queue queue) {
    return_if_null("queue", queue, 0);
    return queue->number_of_services;
}
double queue_getAverageInterArrivalTime(Queue queue) {
    return_if_null("queue", queue, -1);
    return queue->cumulative_inter_arrival_time/queue->number_of_users;
}
double queue_getAverageResponseTime(Queue queue) {
    return_if_null("queue", queue, -1);
    return queue->cumulative_response_time/(queue->number_of_services+queue->busy_servers);
}
double queue_getAverageServiceTime(Queue queue) {
    return_if_null("queue", queue, -1);
    return queue->cumulative_service_time/(queue->number_of_services+queue->busy_servers);
}
double queue_getRho(Queue queue) {
    return_if_null("queue", queue, -1);
    return (1.0 / queue_getAverageInterArrivalTime(queue)) / (queue->number_of_servers * (1.0 / queue_getAverageServiceTime(queue)));
}
double queue_getAverageNumberOfCustomers(Queue queue) {
	return_if_null("queue", queue, -1);
	return queue_getAverageResponseTime(queue)*(1.0/queue_getAverageInterArrivalTime(queue));
}
double queue_getTheoreticAverageNumberOfCustomers(Queue queue, double lambda) {
	return_if_null("queue", queue, -1);
	return queue_getTheoreticAverageResponseTime(queue, lambda)*lambda;
}
double queue_getTheoreticRho(Queue queue, double lambda) {
	return_if_null("queue", queue, -1);
    if(lambda<=0) {
        return -1;
    }
    return lambda / (queue->number_of_servers * server_getAverageServiceRate(queue->server));
}
double queue_getTheoreticAverageServiceTime(Queue queue) {
    return_if_null("queue", queue, -1);
    return server_getAverageOfServiceTime(queue->server);
}
double queue_getTheoreticAverageResponseTime(Queue queue, double lambda) {
    return_if_null("queue", queue, -1);
    return server_getAverageResponseTime(queue->server, queue->number_of_servers, queue_getTheoreticRho(queue, lambda));
}
/*struct Queue_t {
//input parameters
	Server server;	//information about the kind of the server
					//allow to generate the correct service time
	unsigned long number_of_servers; //number of servers in the system
	Time *current_time;	//pointer of the current_time (value of the world)
	EventList *scheduled_events;	//events scheduled (on the world)
//internal variables
	EventList waiting_events;	//contains the events in the waiting line
	unsigned long busy_servers;	//number of server currently busy
	Time last_event_time;	//time in which occur the last event
	unsigned long total_users;	//number of users currently in the system
//measures
	unsigned long number_of_users;	//number of users entered in the queue
	unsigned long number_of_services;	//number of ended services
	Time cumulative_inter_arrival_time;	//summation of all the inter arrival time
	Time cumulative_service_time;		//summation of all the service time
	Time cumulative_waiting_time;//summation of all the waited time of all the events putted in service
	Time cumulative_free_system_time;//summation of all the time in which the system was empty (0 user inside)
};
*/
