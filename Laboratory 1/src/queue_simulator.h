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

#ifndef QUEUE_SIMULATOR_H_INCLUDED
#define QUEUE_SIMULATOR_H_INCLUDED

#include <stdio.h>
#include "server.h"
#include "simulation_time.h"

typedef struct queue_simulator_t *Queue_Simulator;

Queue_Simulator queue_simulator_init(unsigned long number_of_servers, double lambda, Server server, Time maximum, unsigned long population_size, long waiting_line_size);
void queue_simulator_run(Queue_Simulator sim);
void queue_simulator_inputs(FILE *stream, Queue_Simulator sim);
void queue_simulator_outputs(FILE *stream, Queue_Simulator sim);
Queue_Simulator queue_simulator_keyboard_init();
Queue_Simulator queue_simulator_command_line_init(int argc, char *argv[]);
void queue_simulator_free(Queue_Simulator sim);

void queue_simulator_statistics(Queue_Simulator sim);
#endif // QUEUE_SIMULATOR_H_INCLUDED
