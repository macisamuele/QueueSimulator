#include <stdio.h>
#include "QueueNetwork.h"

int main(int argc, char *argv[]) {
	QueueNetwork queuenetwork;
	queuenetwork = queuenetwork_init_from_command_line(argc-1, argv+1);
  	queuenetwork_run(queuenetwork);
	queuenetwork_inputs(stdout, queuenetwork);fflush(stdout);
	queuenetwork_outputs(stdout, queuenetwork);
	queuenetwork_free(queuenetwork);
	return 0;
}
