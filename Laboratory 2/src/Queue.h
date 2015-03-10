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
