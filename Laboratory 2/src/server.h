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

#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include <stdio.h>
#include "errors_warnings_management.h"
#include "random_numbers.h"
#include "simulation_time.h"

typedef enum{M, H, E, D} Service_Distribution;
typedef struct server_t *Server;
void server_print(FILE *stream, Server server);
const char *server_getUsage();
Server server_command_line_init(int argc, char *argv[], int *readed);
Server server_keyboard_init();
Time server_getServiceTime(Server server);
double server_getAverageServiceRate(Server server);
double server_getAverageResponseTime(Server server, unsigned long number_of_servers, double rho);
double server_getAverageNumberOfUserInTheSystem(Server server, unsigned long number_of_servers, double rho);
double server_getSquareCoefficientOfVariation(Server server);
double server_getPI0(Server server, double rho, unsigned long c);
double server_get_loss_probability(Server server, unsigned long number_of_server, long waiting_line_size, double lambda);
void server_statistics(Server server);
double server_getAverageOfServiceTime(Server server);
double server_getVarianceOfServiceTime(Server server);
void server_free(Server server);

#endif // SERVER_H_INCLUDED
