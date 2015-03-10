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
