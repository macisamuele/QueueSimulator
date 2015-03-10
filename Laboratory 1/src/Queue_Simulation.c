#include <stdio.h>
#include <stdlib.h>
#include "queue_simulator.h"
#include "server.h"

int main(int argc, char *argv[]) {
    Queue_Simulator sim;
    sim = queue_simulator_command_line_init(argc-1, argv+1);
    queue_simulator_run(sim);
    queue_simulator_inputs(stdout, sim);
    printf("\n");
    queue_simulator_outputs(stdout, sim);
    printf("\n");
    queue_simulator_statistics(sim);
    queue_simulator_free(sim);
	return 0;
}
