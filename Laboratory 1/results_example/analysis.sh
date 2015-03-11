#!/bin/bash
#
# Copyright 2014 Samuele Maci (macisamuele@gmail.com)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

####################
# Input parameters #
####################
simulations=("1 M -1" "1 E2 -1" "1 H2 -1" "2 M -1" "2 E2 -1" "2 H2 -1" "2 D -1" "1 M 0" "1 E2 0" "1 D 0" "2 M 0" "2 M 1" "2 M 2")
maximum_simulation_time=1000000
if [ $# -ge 1 ]; then
	maximum_simulation_time=$1
fi
lambda=1
n_rho=9
if [ $# -ge 2 ]; then
	n_rho=$2
fi
result_folder="results_$(echo $maximum_simulation_time)"
exported_format="pdf"
n_parallel=$(cat /proc/cpuinfo | grep -c processor)
#############
# Functions #
#############
evaluate_parameters_M() { #$1=rho $2=number_of_servers $3=lambda
    echo "M $(echo "$3/($2*$1)" | bc -l)"
}
evaluate_parameters_H2() { #$1=rho $2=number_of_servers $3=lambda
#from rho evaluate the average service time (alpha*1/mu1+(1-alpha)*1/mu2)
    mu=$(echo "$3/($2*$1)" | bc -l)
    #fixed m1=0.7*mu and mu2=0.3*mu we have alpha=0.35
    #in general alpha=(mu2-mu)/(mu2-mu1)*mu1/mu
    mu1=$(echo "0.7*$mu" | bc -l)
    mu2=$(echo "1.3*$mu" | bc -l)
    alpha=$(echo "($mu2-$mu)/($mu2-$mu1)*$mu1/$mu" | bc -l)
    echo "H 2 $mu1 $mu2 $alpha"
}
evaluate_parameters_E2() { #$1=rho $2=number_of_servers $3=lambda
    mu=$(echo "$3/($2*$1)" | bc -l)
    #fixed m1 we have mu2=mu*mu1/(mu1-mu)
    #so fixed m1=1.3*mu we have mu2=mu*1.3/0.3
    mu1=$(echo "1.3*$mu" | bc -l)
    mu2=$(echo "$mu*1.3/0.3" | bc -l)
    echo "E 2 $mu1 $mu2"
}
evaluate_parameters_D() { #$1=rho $2=number_of_servers $3=lambda
    echo "D $(echo "$3/($2*$1)" | bc -l)"
}
remove_final_zeros() { #$1=number
    str=$(echo $1 | perl -pe 's{\.\d*?\K0*(\||$)}{$1}g')
    i=$((${#str}-1));
    if [ "${str:$i:1}" == "." ]; then
        echo ${str:0:$i}
    else
        echo $str
    fi
}
separate_field_with_underscore() {
    for par in "${@}"; do
        echo -n "_$(remove_final_zeros $par)"
    done
    echo ""
}
trim() { #$1=string
    echo $1 | sed -e 's/^ *//' -e 's/ *$//'
}
write_summary() { #$1=distribution $2=number_of_servers $3=waiting_line_size/E[L]
    foldername="$result_folder/simulation_$(echo $server)_$(echo $current_distribution)_$(echo $WL_size)"
    if [ -d $foldername ]; then
        p=$result_folder/summary_Distribution_$1_Server_$(echo $2)_WaitingLineSize_$(echo $3)
        server=$2
        for file in $(ls $foldername); do
            pf="$foldername/$file"
            th_rho=$(trim $(cat $pf | grep "rho" | grep "Theoretic" | cut -d'=' -f3))
            if [ ${#th_rho} -eq 0 ]; then th_rho="-1"; fi
            avg_inter_arrival_time=$(trim $(cat $pf | grep "inter arrival time" | grep "Simulated" | cut -d'=' -f2))
            th_avg_inter_arrival_time=$(trim $(cat $pf | grep "inter arrival time" | grep "Theoretic" | cut -d'=' -f2))
            if [ ${#th_avg_inter_arrival_time} -eq 0 ]; then th_avg_inter_arrival_time="-1"; fi
            error_avg_inter_arrival_time=$(trim $(cat $pf | grep "inter arrival time" | grep "Error" | cut -d'=' -f2 | cut -d'%' -f1))
            if [ ${#error_avg_inter_arrival_time} -eq 0 ]; then error_avg_inter_arrival_time="-1"; fi
            avg_waiting_time=$(trim $(cat $pf | grep "waiting time" | grep "Simulated" | cut -d'=' -f2))
            th_avg_waiting_time=$(trim $(cat $pf | grep "waiting time" | grep "Theoretic" | cut -d'=' -f2))
            if [ ${#th_avg_waiting_time} -eq 0 ]; then th_avg_waiting_time="-1"; fi
            error_avg_waiting_time=$(trim $(cat $pf | grep "waiting time" | grep "Error" | cut -d'=' -f2 | cut -d'%' -f1))
            if [ ${#error_avg_waiting_time} -eq 0 ]; then error_avg_waiting_time="-1"; fi
            avg_response_time=$(trim $(cat $pf | grep "response (service+waiting) time" | grep "Simulated" | cut -d'=' -f2))
            th_avg_response_time=$(trim $(cat $pf | grep "response (service+waiting) time" | grep "Theoretic" | cut -d'=' -f2))
            if [ ${#th_avg_response_time} -eq 0 ]; then th_avg_response_time="-1"; fi
            error_avg_response_time=$(trim $(cat $pf | grep "response (service+waiting) time" | grep "Error" | cut -d'=' -f2 | cut -d'%' -f1))
            if [ ${#error_avg_response_time} -eq 0 ]; then error_avg_response_time="-1"; fi
            avg_number_of_customers=$(trim $(cat $pf | grep "average number of customers" | grep "Simulated" | cut -d'=' -f2))
            th_avg_number_of_customers=$(trim $(cat $pf | grep "average number of customers" | grep "Theoretic" | cut -d'=' -f2))
            if [ ${#th_avg_number_of_customers} -eq 0 ]; then th_avg_number_of_customers="-1"; fi
            error_avg_number_of_customers=$(trim $(cat $pf | grep "average number of customers" | grep "Error" | cut -d'=' -f2 | cut -d'%' -f1))
            if [ ${#error_avg_number_of_customers} -eq 0 ]; then error_avg_number_of_customers="-1"; fi
            avg_busy_servers=$(trim $(cat $pf | grep "average of busy servers" | grep "Simulated" | cut -d'=' -f2))
            th_avg_busy_servers=$(trim $(cat $pf | grep "average of busy servers" | grep "Theoretic" | cut -d'=' -f2))
            if [ ${#th_avg_busy_servers} -eq 0 ]; then th_avg_busy_servers="-1"; fi
            error_avg_busy_servers=$(trim $(cat $pf | grep "average of busy servers" | grep "Error" | cut -d'=' -f2 | cut -d'%' -f1))
            if [ ${#error_avg_busy_servers} -eq 0 ]; then error_avg_busy_servers="-1"; fi
            loss_prob=$(trim $(cat $pf | grep "loss probability" | grep "Simulated" | cut -d'=' -f2))
            th_loss_prob=$(trim $(cat $pf | grep "loss probability" | grep "Theoretic" | cut -d'=' -f2))
            if [ ${#th_loss_prob} -eq 0 ]; then th_loss_prob="-1"; fi
            error_loss_prob=$(trim $(cat $pf | grep "loss probability" | grep "Error" | cut -d'=' -f2 | cut -d'%' -f1))
            if [ ${#error_loss_prob} -eq 0 ]; then error_loss_prob="-1"; fi
            prob_idle_system=$(trim $(cat $pf | grep "idle system" | grep "Simulated" | cut -d'=' -f2))
            loss_prob=$(trim $(cat $pf | grep "loss probability" | grep "Simulated" | cut -d'=' -f2))
            th_prob_idle_system=$(trim $(cat $pf | grep "idle system" | grep "Theoretic" | cut -d'=' -f2))
            if [ ${#th_prob_idle_system} -eq 0 ]; then th_prob_idle_system="-1"; fi
            error_prob_idle_system=$(trim $(cat $pf | grep "idle system" | grep "Error" | cut -d'=' -f2 | cut -d'%' -f1))
            if [ ${#error_prob_idle_system} -eq 0 ]; then error_prob_idle_system="-1"; fi
            echo "$th_rho $avg_inter_arrival_time $th_avg_inter_arrival_time $error_avg_inter_arrival_time $avg_waiting_time $th_avg_waiting_time $error_avg_waiting_time $avg_number_of_customers $th_avg_number_of_customers $error_avg_number_of_customers $avg_response_time $th_avg_response_time $error_avg_response_time $avg_busy_servers $th_avg_busy_servers $error_avg_busy_servers $loss_prob $th_loss_prob $error_loss_prob $prob_idle_system $th_prob_idle_system $error_prob_idle_system" >> $p
        done
        sort $p -o $p
        echo -e "rho simulated_avg_inter_arrival_time th_avg_inter_arrival_time error_avg_inter_arrival_time simulated_avg_waiting_time th_avg_waiting_time error_avg_waiting_time simulated_avg_number_of_customers th_avg_number_of_customers error_avg_number_of_customers simulated_avg_response_time th_avg_response_time error_avg_response_time simulated_avg_busy_servers th_avg_busy_servers error_avg_busy_servers simulated_loss_prob th_loss_prob error_loss_prob simulated_prob_idle_system th_prob_idle_system error__prob_idle_system\n$(cat $p)" > $p
    fi
}
extract_distribution_from_file_name() { #$1=filename
    echo "$1" | sed -n 's/.*_\([A-Z]\)_.*/\1/p'
}
extract_server_from_file_name() { #$1=filename
    echo "$1" | sed -n 's/.*Server_\([0-9]*\)_.*/\1/p'
}
extract_multiplicative_factor_of_waiting_line_from_name() { #$1=filename
    echo "$1" | sed -n 's/.*_\(-*[0-9]*\)_Waiting.*/\1/p'
}
wait_for_all_process_in_background() {
    terminated=0
    for pid in ${@}; do
        wait $pid
        terminated=$(($terminated+1))
        echo -en "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b$(printf "%10d" $(($o_terminated+$terminated)))/$o_size"
    done
    echo -en "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
}
size() {
    echo ${#@}
}
#############################
# Check if simulator exists #
#############################
if [ ! -f simulator ]; then
    echo "The simulator is not available"
    echo "Please put in the current folder the executable file of the simulator (name: simulator)"
    exit 1
fi
#########################
# Generation of the rho #
#########################
rho=()
for i in $(seq 1 $n_rho); do
    rho[${#rho[@]}]=$(remove_final_zeros $(echo "$i/$(($n_rho+1))" | bc -l))
done
###########################
# Start of the Simulation #
###########################
echo -e "Simulation of:\n\tmaximum simulation time: $maximum_simulation_time\n\tnumber of servers: ${number_of_servers[@]}\n\tlambda: ${lambda[@]}\n\trho: ${rho[@]}"
###########
# WARNING #
###########
echo -e "REMEMBER: the folder \"$result_folder\" will be created if not exists or all the file will be deleted\nPress ENTER to continue"
read
rm -rf $result_folder 2> /dev/null
if [ ! -d "$result_folder" ]; then
    mkdir $result_folder
fi
######################################################################################
# Generation of the simulation parameters (for all the simulations that we will run) #
######################################################################################
for simulation in "${simulations[@]}"; do
    eval "simulation=($simulation)"
    server=${simulation[0]}
    current_distribution=${simulation[1]}
    WL_size=${simulation[2]}
    filename="$result_folder/simulations_parameters_$(echo $server)_$(echo $current_distribution)_$(echo $WL_size)"
    for current_rho in ${rho[@]}; do
        echo "$server $lambda $(evaluate_parameters_$current_distribution $current_rho $server $lambda) $maximum_simulation_time 0 $WL_size"
    done > $filename
    echo -e "$(awk '!x[$0]++' $filename)" > $filename
done
#######################
# Run the simulations #
#######################
o_terminated=0
o_size=$(($n_rho*$(size "${simulations[@]}")))
for simulation in "${simulations[@]}"; do
    eval "simulation=($simulation)"
    server=${simulation[0]}
    current_distribution=${simulation[1]}
    WL_size=${simulation[2]}
    filename="$result_folder/simulations_parameters_$(echo $server)_$(echo $current_distribution)_$(echo $WL_size)"
    foldername="$result_folder/simulation_$(echo $server)_$(echo $current_distribution)_$(echo "$WL_size")"
    mkdir $foldername
    pids=""
    while read line; do
        if [ ${#line} -gt 0 ]; then
            ./simulator $line > $foldername/results$(separate_field_with_underscore $line) 2> /dev/null &
            pids="$pids ${!}"
            if [ "$n_parallel" == "$(size $pids)" ]; then
                wait_for_all_process_in_background $pids
                o_terminated=$(($o_terminated+$(size $pids)))
                pids=""
            fi
       fi
    done < $filename
    wait_for_all_process_in_background $pids
    o_terminated=$(($o_terminated+$(size $pids)))
    echo "Ended Simulations for $(printf "%1s" $server) servers $(printf "%2s" $current_distribution) distribuited with Waiting Line Size of $(printf "%2s" $WL_size)"
done
rm -rf $result_folder/simulations_parameters_* 2> /dev/null
echo "Ended ALL Simulations"
n_parallel=$(cat /proc/cpuinfo | grep -c processor)
#############################
# Building of the Summaries #
#############################
o_terminated=0
o_size=$(size "${simulations[@]}")
pids=""
for simulation in "${simulations[@]}"; do
    eval "simulation=($simulation)"
    server=${simulation[0]}
    current_distribution=${simulation[1]}
    WL_size=${simulation[2]}
    write_summary $current_distribution $server $WL_size &
    pids="$pids ${!}"
    if [ "$n_parallel" == "$(size $pids)" ]; then
        wait_for_all_process_in_background $pids
        pids=""
        o_terminated=$(($o_terminated+$n_parallel))
    fi
done
wait_for_all_process_in_background $pids
echo "Ended the building of the summaries"
#########
# Plots #
#########
##########
# Task I #
##########
#x-axis: rho     y-axis: E[Tw]  theoretic for the M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[Tw]_theoretic_MM1.$exported_format\"; set title \"Performance for M/M/1 System\"; set xlabel \"Rho\"; set ylabel \"Average Waiting Time, E[Tw]   [time unit]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:6 title \"Theoretic\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[Tw]  simulated for the M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[Tw]_simulated_MM1.$exported_format\"; set title \"Performance for M/M/1 System\"; set xlabel \"Rho\"; set ylabel \"Average Waiting Time, E[Tw]   [time unit]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:5 title \"Simulated\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[Tw]  error for the M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[Tw]_error_MM1.$exported_format\"; set title \"Performance for M/M/1 System\"; set xlabel \"Rho\"; set ylabel \"Relative Error: Average Waiting Time   [%]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:7 title \"Relative Error [abs(1-E[Tw_sim]/E[Tw_th])]\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[P[idle system]]  simulated AND theoretic for the M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[idle_system]_simulated_theoretic_MM1.$exported_format\"; set title \"Performance for M/M/1 System\"; set xlabel \"Rho\"; set ylabel \"Idle System Probability, pi0\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:20 title \"Simulated\" with lines, \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:21 title \"Theoretic\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[P[idle system]]  error for the M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[idle_system]_error_MM1.$exported_format\"; set title \"Performance for M/M/1 System\"; set xlabel \"Rho\"; set ylabel \"Relative Error: Idle System Probability   [%]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:22 title \"Relative Error [abs(1-E[pi0_sim]/E[pi0_th])]\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[T]/E[lambda]  theoretic AND simulated for the M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/verify_littles_result_MM1.$exported_format\"; set title \"Verifiy Little's Results: E[L]=E[T]/E[lambda]\"; set xlabel \"Rho\"; set ylabel \"Average Customers in the System, E[L]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:(\$11/\$2) title \"Simulated E[T]/E[lambda]\" with lines, \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:9 title \"Theoretic E[L]\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
###########
# Task II #
###########
#x-axis: rho     y-axis: E[Tw]  simulated for the M/M/1, M/H2/1, M/E2/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[Tw]_simulated_MM1_MH21_ME21.$exported_format\"; set title \"Performance for M/M/1, M/H-2/1, M/E-2/1 Systes\"; set xlabel \"Rho\"; set ylabel \"Average Waiting Time, E[Tw]   [time unit]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:5 title \"Simulated M/M/1\" with lines, \"$result_folder/summary_Distribution_H2_Server_1_WaitingLineSize_-1\" u 1:5 title \"Simulated M/H-2/1\" with lines, \"$result_folder/summary_Distribution_E2_Server_1_WaitingLineSize_-1\" u 1:5 title \"Simulated M/E-2/1\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[Tw]  simulated for the M/M/1, M/H2/1, M/E2/2 ZOOM
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[Tw]_simulated_MM1_MH21_ME21_zoom.$exported_format\"; set title \"Performance for M/M/1, M/H-2/1, M/E-2/1 Systes\"; set xlabel \"Rho\"; set ylabel \"Average Waiting Time, E[Tw]   [time unit]\"; set key top left; set xrange [0:0.5]; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:5 title \"Simulated M/M/1\" with lines, \"$result_folder/summary_Distribution_H2_Server_1_WaitingLineSize_-1\" u 1:5 title \"Simulated M/H-2/1\" with lines, \"$result_folder/summary_Distribution_E2_Server_1_WaitingLineSize_-1\" u 1:5 title \"Simulated M/E-2/1\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[Tw]  error for the M/M/1, M/H2/1, M/E2/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[Tw]_error_MM1_MH21_ME21.$exported_format\"; set title \"Performance for M/M/1, M/H-2/1, M/E-2/1 Systes\"; set xlabel \"Rho\"; set ylabel \"Relative Error: Average Waiting Time, E[Tw]   [%]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:7 title \"Simulated M/M/1\" with lines, \"$result_folder/summary_Distribution_H2_Server_1_WaitingLineSize_-1\" u 1:7 title \"Simulated M/H-2/1\" with lines, \"$result_folder/summary_Distribution_E2_Server_1_WaitingLineSize_-1\" u 1:7 title \"Simulated M/E-2/1\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[Tw]  error for the M/M/1, M/H2/1, M/E2/2 ZOOM
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[Tw]_error_MM1_MH21_ME21_zoom.$exported_format\"; set title \"Performance for M/M/1, M/H-2/1, M/E-2/1 Systes\"; set xlabel \"Rho\"; set ylabel \"Relative Error: Average Waiting Time, E[Tw]   [%]\"; set key top left; set xrange [0:0.5]; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:7 title \"Simulated M/M/1\" with lines, \"$result_folder/summary_Distribution_H2_Server_1_WaitingLineSize_-1\" u 1:7 title \"Simulated M/H-2/1\" with lines, \"$result_folder/summary_Distribution_E2_Server_1_WaitingLineSize_-1\" u 1:7 title \"Simulated M/E-2/1\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
############
# Task III #
############
#x-axis: rho     y-axis: E[T]    simulated for the M/M/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_simulated_MM2.$exported_format\"; set title \"Performace for M/M/2 System\"; set xlabel \"Rho\"; set ylabel \"Average Queueing Delay   [time unit]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_2_WaitingLineSize_-1\" u 1:11 title \"Simulated\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[T]    theoretic AND simulated for the M/M/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_simulated_theoretic_MM2.$exported_format\"; set title \"Performace for M/M/2 System\"; set xlabel \"Rho\"; set ylabel \"Average Queueing Delay   [time unit]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_2_WaitingLineSize_-1\" u 1:11 title \"Simulated\" with lines, \"$result_folder/summary_Distribution_M_Server_2_WaitingLineSize_-1\" u 1:12 title \"Theoretic\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[T]    theoretic for the M/M/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_theoretic_MM2.$exported_format\"; set title \"Performace for M/M/2 System\"; set xlabel \"Rho\"; set ylabel \"Average Queueing Delay   [time unit]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_2_WaitingLineSize_-1\" u 1:12 title \"Theoretic\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[T]    error for the M/M/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_error_MM2.$exported_format\"; set title \"Performace for M/M/2 System\"; set xlabel \"Rho\"; set ylabel \"Relative Error: Average Queueing Delay   [%]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_-1\" u 1:13 title \"Relative Error\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[busy server]    simulated for M/D/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[busy_server]_simulated_MD2.$exported_format\"; set title \"Performace for M/D/2 System\"; set xlabel \"Rho\"; set ylabel \"E[busy servers]\"; set key top left; plot \"$result_folder/summary_Distribution_D_Server_2_WaitingLineSize_-1\" u 1:14 title \"Simulated\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[busy server]    theoretic for M/D/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[busy_server]_theoric_MD2.$exported_format\"; set title \"Performace for M/D/2 System\"; set xlabel \"Rho\"; set ylabel \"E[busy servers]\"; set key top left; plot \"$result_folder/summary_Distribution_D_Server_2_WaitingLineSize_-1\" u 1:15 title \"Theoretic\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[busy server]    theoretic AND simulated for M/D/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[busy_server]_simulated_theoric_MD2.$exported_format\"; set title \"Performace for M/D/2 System\"; set xlabel \"Rho\"; set ylabel \"E[busy servers]\"; set key top left; plot \"$result_folder/summary_Distribution_D_Server_2_WaitingLineSize_-1\" u 1:14 title \"Simulated\" with lines, \"$result_folder/summary_Distribution_D_Server_2_WaitingLineSize_-1\" u 1:15 title \"Theoretic\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[busy server]    error for M/D/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[busy_server]_error_MD2.$exported_format\"; set title \"Performace for M/D/2 System\"; set xlabel \"Rho\"; set ylabel \"Relative error E[busy servers]\"; set key bottom left; plot \"$result_folder/summary_Distribution_D_Server_2_WaitingLineSize_-1\" u 1:16 title \"Relative Error\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[Tw]  simulated for the M/M/2, M/H-2/2, M/E-2/2, M/D/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[Tw]_simulated_MM2_MH22_ME22_MD2.$exported_format\"; set title \"Performance for M/M/2, M/H-2/2, M/E-2/2, M/D/2 Systems\"; set xlabel \"Rho\"; set ylabel \"Average Waiting Time, E[Tw]   [time unit]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_2_WaitingLineSize_-1\" u 1:5 title \"Simulated M/M/2\" with lines, \"$result_folder/summary_Distribution_H2_Server_2_WaitingLineSize_-1\" u 1:5 title \"Simulated M/H-2/2\" with lines, \"$result_folder/summary_Distribution_E2_Server_2_WaitingLineSize_-1\" u 1:5 title \"Simulated M/E-2/2\" with lines, \"$result_folder/summary_Distribution_D_Server_2_WaitingLineSize_-1\" u 1:5 title \"Simulated M/D/2\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis: E[Tw]  simulated for the M/M/2, M/H-2/2, M/E-2/2, M/D/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[Tw]_simulated_MM2_MH22_ME22_zoom.$exported_format\"; set title \"Performance for M/M/1, M/H-2/1, M/E-2/1 Systes\"; set xlabel \"Rho\"; set ylabel \"Average Waiting Time, E[Tw]   [time unit]\"; set key top left; set xrange [0:0.5]; plot \"$result_folder/summary_Distribution_M_Server_2_WaitingLineSize_-1\" u 1:5 title \"Simulated M/M/2\" with lines, \"$result_folder/summary_Distribution_H2_Server_2_WaitingLineSize_-1\" u 1:5 title \"Simulated M/H-2/2\" with lines, \"$result_folder/summary_Distribution_E2_Server_2_WaitingLineSize_-1\" u 1:5 title \"Simulated M/E-2/2\" with lines, \"$result_folder/summary_Distribution_D_Server_2_WaitingLineSize_-1\" u 1:5 title \"Simulated M/D/2\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
###########
# Task IV #
###########
#x-axis: rho     y-axis:  P('loss') for M/M/1/0, M/E2/0, M/D/1/0
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_loss_probability_simulated_theoric_M-M-1-0_M-E2-1-0_M-D-1-0.$exported_format\"; set title \"Performace of Systems without Waiting Line\"; set xlabel \"Rho\"; set ylabel \"Loss Probability, P{loss}\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_0\" u 1:17 title \"Simulated M/M/1/0\" with lines,  \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_0\" u 1:18 title \"Theoretic M/M/1/0\" with lines, \"$result_folder/summary_Distribution_E2_Server_1_WaitingLineSize_0\" u 1:17 title \"Simulated M/E2/1/0\" with lines, \"$result_folder/summary_Distribution_E2_Server_1_WaitingLineSize_0\" u 1:18 title \"Theoretic M/E2/1/0\" with lines, \"$result_folder/summary_Distribution_D_Server_1_WaitingLineSize_0\" u 1:17 title \"Simulated M/D/1/0\" with lines, \"$result_folder/summary_Distribution_D_Server_1_WaitingLineSize_0\" u 1:18 title \"Theoretic M/D/1/0\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_loss_probability_error_M-M-1-0_M-E2-1-0_M-D-1-0.$exported_format\"; set title \"Performace of Systems without Waiting Line\"; set xlabel \"Rho\"; set ylabel \"Relative Error: Loss Probability, P{loss}   [%]\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_1_WaitingLineSize_0\" u 1:19 title \"Relative Error M/M/1/0\" with lines, \"$result_folder/summary_Distribution_E2_Server_1_WaitingLineSize_0\" u 1:19 title \"Relative M/E2/1/0\" with lines, \"$result_folder/summary_Distribution_D_Server_1_WaitingLineSize_0\" u 1:19 title \"Relative M/D/1/0\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
#x-axis: rho     y-axis:  P('loss') for M/M/2/0 M/M/2/1 M/M/2/2
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_loss_probability_simulated_M-M-2-0_M-M-2-1_M-M-2-2.$exported_format\"; set title \"Performace of Systems\"; set xlabel \"Rho\"; set ylabel \"Loss Probability, P{loss}\"; set key top left; plot \"$result_folder/summary_Distribution_M_Server_2_WaitingLineSize_0\" u 1:17 title \"Simulated M/M/2/0\" with lines,  \"$result_folder/summary_Distribution_M_Server_2_WaitingLineSize_1\" u 1:17 title \"Simulated M/M/2/1\" with lines,  \"$result_folder/summary_Distribution_M_Server_2_WaitingLineSize_2\" u 1:17 title \"Simulated M/M/2/2\" with lines;"
gnuplot -e "$plot_cmd" 2> /dev/null
echo "Ended the making of the plots"
#############################
# Compress the output files #
#############################
for simulation in "${simulations[@]}"; do
    eval "simulation=($simulation)"
    server=${simulation[0]}
    current_distribution=${simulation[1]}
    zip -rq $result_folder/raw_results_$(echo $server)_$(echo $current_distribution) $result_folder/simulation_$(echo $server)_$(echo $current_distribution)_* 2> /dev/null
done
cd $result_folder
zip -j -q raw_results $(ls -1 | grep -v .pdf) 2> /dev/null
rm -rf $(ls -1 | grep -vE "raw_results.zip|*.pdf")
cd ..
echo "Ended the compression of the results"
