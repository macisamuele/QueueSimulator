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
