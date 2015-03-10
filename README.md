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

<b>REMARK</b>: the anaysis.sh script requires gnuplot and zip installed <br />
### Results
<b>TODO</b>

## Laboratory 2
Simulate a simple Queuing Network (Two M/M/1 queues). <br />
The whole text of the laboratory session is the Request.pdf file in the directory "Laboratory 2"
### Compile
To compile the sources you need to compile the code provided:

    bash compile.sh [STAT]

the STAT parameter allow to have some statistics printed on the stdout <br />
<b>REMARK 1</b>: the compile.sh script requires gcc installed <br />
<b>REMARK 2</b>: the statistics printing reduce the performance, but can helpful in the debugging phase <br />
### Use
To use the simulator you need to run the analysis.sh script that will call the execution of the simulator (executable obtained from the compiling phase)

    bash analysis.sh <maximum_simulation_time> <number_of_different_rhos> <max(log10(mu1/mu2))> <number_of_simulations_for_the_confidence_level>

<b>REMARK</b>: the anaysis.sh script requires bc, gnuplot and zip installed <br />
### Results
<b>TODO</b>

