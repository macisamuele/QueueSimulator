# QueueSimulator
QueueSimulator for the Network Modelling Course of the Polytechnic University of Turin, Italy

## Laboratory 1
Simulate a Queuing System M/M/1, M/G/k and M/G/k/B queues and plot the results of relevant measures. <br />
The whole text of the laboratory session is the Request.pdf file in the directory "Laboratory 1"
### Compile
To compile the sources you need to compile the code provided:

    bash compile.sh [STAT]

the STAT parameter allow to have some statistics printed on the stdout <br />
<b>REMARK 1</b>: the compile.sh script requires gcc installed <br />
<b>REMARK 2</b>: the statistics printing reduce the performance, but can helpful in the debugging phase <br />
### Use
To use the simulator you need to run the analysis.sh script that will call the execution of the simulator (executable obtained from the compiling phase)

    bash analysis.sh <maximum_simulation_time> <number_of_different_rhos>

**REMARK**: the anaysis.sh script requires gnuplot and zip installed <br />
### Results
Inside the Laboratory 1 folder are available the results of the execution of

    bash analysis.sh 1000000 19

In the directory you will find a lot of plots with a lot of different measures (all are provided from the analysis.sh script) and a zip file containing all the raw results used for the plots, to allows also other plots or different analysis.

### Code customization
The main modification that you can need to implement is the definition of a different server.<br />
In the current implementation are implemented Markovian, Erlang, HyperExponential and Deterministic.<br />
If you really need to modify the file server.c adding: <br />
 - in server_print function the summary of the server,
 - in server_getUsage function the help of usage of the new servering distribution
 - in server_init functions the correct initialization
 - in server_getServiceTime function the generation process of the serving time
 - in server_getAverageServiceRate the average service rate expected
 - in server_getVarianceOfServiceTime function the variance expected of the service rate

Apologize for the user <i>un-friendly</i> approach but the original simulator's request was really constrained.<br />
For sure you have to apply only few modification to the code.

Enjoy with this simulator.

## Laboratory 2
Simulate a simple Queuing Network (Two M/M/1 queues). <br />
The whole text of the laboratory session is the Request.pdf file in the directory "Laboratory 2"
### Compile
To compile the sources you need to compile the code provided:

    bash compile.sh [STAT]

the STAT parameter allow to have some statistics printed on the stdout <br />
**REMARK 1**: the compile.sh script requires gcc installed <br />
**REMARK 2**: the statistics printing reduce the performance, but can helpful in the debugging phase <br />
### Use
To use the simulator you need to run the analysis.sh script that will call the execution of the simulator (executable obtained from the compiling phase)

    bash analysis.sh <maximum_simulation_time> <number_of_different_rhos> <max(log10(mu1/mu2))> <number_of_simulations_for_the_confidence_level>

**REMARK**: the anaysis.sh script requires bc, gnuplot and zip installed <br />
### Results
**TODO**

