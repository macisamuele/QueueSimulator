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
