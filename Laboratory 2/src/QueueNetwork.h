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

#ifndef QUEUENETWORK_H_INCLUDED
#define QUEUENETWORK_H_INCLUDED

#include "simulation_time.h"
#include "event.h"
#include "server.h"

typedef struct QueueNetwork_t *QueueNetwork;

QueueNetwork queuenetwork_init(double lambda, Time maximum, unsigned long number_of_queues, Server *servers, unsigned long *number_of_servers, int present_initial_seed, long initial_seed);
void queuenetwork_free(QueueNetwork queuenetwork);
void queuenetwork_run(QueueNetwork queuenetwork);
void queuenetwork_manage_arrival(QueueNetwork queuenetwork);
void queuenetwork_manage_departure(QueueNetwork queuenetwork, Event event);
QueueNetwork queuenetwork_init_from_command_line(int argc, char *argv[]);
QueueNetwork queuenetwork_keyboard_init();

void queuenetwork_outputs(FILE *stream, QueueNetwork queuenetwork);
void queuenetwork_inputs(FILE *stream, QueueNetwork queuenetwork);
#endif // QUEUENETWORK_H_INCLUDED
