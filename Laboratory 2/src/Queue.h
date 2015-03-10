#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include "event.h"
#include "server.h"

typedef struct Queue_t *Queue;

Queue queue_init(unsigned long number_of_servers, Server server,
		Time *current_time, EventList *scheduled_events);
void queue_free(Queue queue);
Event queue_manage_arrival(Queue queue, Event event);
Event queue_manage_departure(Queue queue, Event event);
Event queue_manage_event(Queue queue, Event event);


unsigned long queue_getNumberOfUsersGenerated(Queue queue);
unsigned long queue_getNumberOfServices(Queue queue);
double queue_getAverageInterArrivalTime(Queue queue);
double queue_getAverageResponseTime(Queue queue);
double queue_getAverageServiceTime(Queue queue);
double queue_getRho(Queue queue);
double queue_getAverageNumberOfCustomers(Queue queue);
double queue_getTheoreticAverageNumberOfCustomers(Queue queue, double lambda);
double queue_getTheoreticRho(Queue queue, double lambda);
double queue_getTheoreticAverageServiceTime(Queue queue);
double queue_getTheoreticAverageResponseTime(Queue queue, double lambda);

#endif // QUEUE_H_INCLUDED
