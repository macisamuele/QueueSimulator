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
maximum_simulation_time=1000000
if [ $# -ge 1 ]; then
	maximum_simulation_time=$1
fi
lambda=1
n_rho=9
if [ $# -ge 2 ]; then
	n_rho=$2
fi
max_log_task2=3
if [ $# -ge 3 ]; then
	max_log_task2=$3
fi
if [ $max_log_task2 -lt 0 ]; then
    max_log_task2=$((-$max_log_task2))
fi
result_folder="results_$(echo $maximum_simulation_time)"
exported_format="pdf"
n_parallel=$(cat /proc/cpuinfo | grep -c processor)
n_simulations_CL_level=40
if [ $# -ge 4 ]; then
	n_simulations_CL_level=$4
fi
if [ $n_simulations_CL_level -lt 30 ]; then
    n_simulations_CL_level=30
fi
start_rho="0.96"    #min rho in the zoomed plot
end_rho="0.99"      #max rho in the zoomed plot
#############
# Functions #
#############
remove_final_zeros() { #$1=number
    str=$(echo $1 | perl -pe 's{\.\d*?\K0*(\||$)}{$1}g')
    i=$((${#str}-1));
    if [ "${str:$i:1}" == "." ]; then
        echo ${str:0:$i}
    else
        echo $str
    fi
}
evaluate_parameters_M() { #$1=rho $2=number_of_servers $3=lambda
    echo "M $(remove_final_zeros $(echo "$3/($2*$1)" | bc -l))"
}
bc_power() { #$1=base $2=exponent
    if [ "$(echo "$2/1==$2"  | bc)" == "1" ]; then
        echo "$1^$2"
    else
        echo "e($2*l($1))"
    fi
}
generate_log_space() { #$1=log10(min) $2=log10(max) $3=number
    if [ "$1" == "$2" ]; then
        echo "$1"
    else
        if [ $3 -eq 1 ]; then
            echo "$1"
        else
            exponents=()
            exponents[${#exponents[@]}]=$(bc_power 10 $(remove_final_zeros $(echo "($1)" | bc -l)) | bc -l)
            for i in $(seq 1 $(($3-1))); do
                exponents[${#exponents[@]}]=$(bc_power 10 $(remove_final_zeros $(echo "($1)+(($2)-($1))/($(($3-1))/$i)" | bc -l)) | bc -l)
            done
            echo ${exponents[@]}
        fi
    fi
}
generate_lin_space() { #$1=log10(min) $2=log10(max) $3=number
    if [ "$1" == "$2" ]; then
        echo "$1"
    else
        if [ $3 -eq 1 ]; then
            echo "$1"
        else
            values=()
            values[${#values[@]}]=$(remove_final_zeros $(echo "($1)" | bc -l))
            for i in $(seq 1 $(($3-1))); do
                values[${#values[@]}]=$(remove_final_zeros $(echo "($1)+(($2)-($1))/($(($3-1))/$i)" | bc -l))
            done
            echo ${values[@]}
        fi
    fi
}
separate_field_with_underscore() {
    for par in "${@}"; do
        echo -n "_$(remove_final_zeros $par)"
    done
    echo ""
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
build_summary() { #$1=Task $2=Part $3="queue numbers"
    if [ "$1" == "I" -a "$2" == "2" ]; then
        echo "rho average_E[T] variance_E[T] theoretic_E[T] error_E[T]" > "$result_folder/Summary_Task_I_2"
        while read line; do
            if [ ${#line} -gt 0 ]; then
                extract_rho_avg_var_th_err $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)_Summary
            fi
        done < "$result_folder/Simulation_Parameters_Task_I_2" >> "$result_folder/Summary_Task_I_2"
    else
        file="$result_folder/Summary_Task_$1_$2"
        echo -n "simulated_average_response_time_QN theoretic_average_response_time_QN error_average_response_time_QN simulated_average_number_of_customers_QN theoretic_average_number_of_customers_QN error_average_number_of_customers_QN" > $file
        for queue in $3; do
            echo -n " rho_Q$queue simulated_average_response_time_Q$queue theoretic_average_response_time_Q$queue error_average_response_time_Q$queue simulated_average_number_of_customers_Q$queue theoretic_average_number_of_customers_Q$queue error_average_number_of_customers_Q$queue" >> $file
        done
        echo >> $file
        while read line; do
            for device in N $3; do
                if [ $device != "N" ]; then
                    rho=$(cat "$result_folder/Simulations_Task_$1_$2/results$(separate_field_with_underscore $line)" | grep "Rho" | grep "Q$device" | grep "Theoretic" | cut -d'=' -f2)
                    echo -n " $(trim $rho)" >> $file
                fi
                avg_response_time=$(cat "$result_folder/Simulations_Task_$1_$2/results$(separate_field_with_underscore $line)" | grep "Q$device" | grep "response" | grep "Simulated" | cut -d'=' -f2)
                th_avg_response_time=$(cat "$result_folder/Simulations_Task_$1_$2/results$(separate_field_with_underscore $line)" | grep "Q$device" | grep "response" | grep "Theoretic" | cut -d'=' -f2)
                error_avg_response_time=$(cat "$result_folder/Simulations_Task_$1_$2/results$(separate_field_with_underscore $line)" | grep "Q$device" | grep "response" | grep "Error" | cut -d'=' -f2 | cut -d'%' -f1)
                avg_number_customers=$(cat "$result_folder/Simulations_Task_$1_$2/results$(separate_field_with_underscore $line)" | grep "Q$device" | grep "customers" | grep "Simulated" | cut -d'=' -f2)
                th_avg_number_customers=$(cat "$result_folder/Simulations_Task_$1_$2/results$(separate_field_with_underscore $line)" | grep "Q$device" | grep "customers" | grep "Theoretic" | cut -d'=' -f2)
                error_avg_number_customers=$(cat "$result_folder/Simulations_Task_$1_$2/results$(separate_field_with_underscore $line)" | grep "Q$device" | grep "customers" | grep "Error" | cut -d'=' -f2 | cut -d'%' -f1)
                echo -n " $(trim $avg_response_time) $(trim $th_avg_response_time) $(trim $error_avg_response_time) $(trim $avg_number_customers) $(trim $th_avg_number_customers) $(trim $error_avg_number_customers)" >> $file
            done
            echo >> $file
        done < "$result_folder/Simulation_Parameters_Task_$1_$2"
    fi
}
trim() { #$1=string
    echo $1 | sed -e 's/^ *//' -e 's/ *$//'
}
simulation_CL() { #$@ = line of the simulation
    mkdir $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)
    ./simulator $line > $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)/results_n=1 2> /dev/null
    last_seed="$(cat $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)/results_n=1 | grep "Last seed" | cut -d':' -f2)"
    for indice in $(seq 2 $n_simulations_CL_level); do
        ./simulator $line $last_seed > $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)/results_n=$indice 2> /dev/null
        last_seed="$(cat $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)/results_n=$indice | grep "Last seed" | cut -d':' -f2)"
    done
    for indice in $(seq 1 $n_simulations_CL_level); do
        rho=$(cat $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)/results_n=$indice | grep "Rho" | grep "Theoretic" | grep "Q0" | cut -d'=' -f2)
        avg_response=$(cat $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)/results_n=$indice | grep "response" | grep "Simulated" | grep "QN" | cut -d'=' -f2)
        th_avg_response=$(cat $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)/results_n=$indice | grep "response" | grep "Theoretic" | grep "QN" | cut -d'=' -f2)
        echo "$(trim $rho) $(trim $avg_response) $(trim $th_avg_response)"
    done > $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)/Summary
    cp $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)/Summary $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)_Summary
    zip -jrq $result_folder/Simulations_Task_I_2/raw_results$(separate_field_with_underscore $line).zip $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)/*
    rm -rf $result_folder/Simulations_Task_I_2/results$(separate_field_with_underscore $line)
}
extract_rho_avg_var_th_err() { #$1=file
    n_lines=0
    avg_str="(0"
    var_str="(0"
    th_str=""
    while read line; do
        measure="$(echo $line | cut -d' ' -f2)"
        rho="$(echo $line | cut -d' ' -f1)"
        if [ "$th_str" == "" ]; then
            th_str="$(echo $line | cut -d' ' -f3)"
        fi
        avg_str="$avg_str+$measure"
        var_str="$var_str+($measure-avg)^2"
        n_lines=$(($n_lines+1))
    done < $1
    avg_str="$avg_str)/($n_lines)"
    var_str="$var_str)/($n_lines)"
    var_str="avg=$avg_str; $var_str"
    avg=$(echo "$avg_str" | bc -l)
    var=$(echo "$var_str" | bc -l)
    echo "$rho $avg $var $th_str $(echo "define abs(x) { if(x<0) return (-x); return(x); }; 100*abs(1-$avg_str/$th_str)" | bc -l)"
}
to_percentage() { #$1=value in [0, 1]
    remove_final_zeros $(echo "$1*100" | bc -l)
}
C_Interval_size() { #$1=percentage in [0, 1]
    trim "$(cat table.txt | grep "CL = $(echo $1)0* " | cut -d'=' -f4)*sqrt(\$3)/sqrt($n_simulations_CL_level)"
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
echo -e "Simulation of:\n\tmaximum simulation time: $maximum_simulation_time\n\tnumber of servers: ${number_of_servers[@]}\n\tlambda: ${lambda[@]}\n\trho: ${rho[@]}\n\tnumber of simulations for CL: $n_simulations_CL_level"
###########
# WARNING #
###########
echo -e "REMEMBER: the folder \"$result_folder\" will be created if not exists or all the file will be deleted\nPress ENTER to continue"
read
rm -rf $result_folder/* 2> /dev/null
if [ ! -d "$result_folder" ]; then
    mkdir $result_folder
fi
######################################################################################
# Generation of the simulation parameters (for all the simulations that we will run) #
######################################################################################
########################
# Simulations Task I.1 #
########################
for current_rho in ${rho[@]}; do
    server_parameter="1 $(evaluate_parameters_M $current_rho 1 1)"
    echo "$lambda 2 $server_parameter $server_parameter $maximum_simulation_time"
done > "$result_folder/Simulation_Parameters_Task_I_1"
########################
# Simulations Task I.2 #
########################
for current_rho in ${rho[@]}; do
    server_parameter="1 $(evaluate_parameters_M $current_rho 1 1)"
    echo "$lambda 2 $server_parameter $server_parameter $maximum_simulation_time"
done > "$result_folder/Simulation_Parameters_Task_I_2"
#########################
# Simulations Task II.1 #
#########################
for current_ratio in $(generate_log_space $((-1*$max_log_task2)) $max_log_task2 $n_rho); do #ratio=mu1/mu2
    mu2=$(echo "$lambda*2*$(bc_power 10 $max_log_task2)" | bc -l)
    mu1=$(echo "$current_ratio*$mu2" | bc -l)
    echo "$lambda 2 1 M $mu1 1 M $mu2 $maximum_simulation_time"
done > "$result_folder/Simulation_Parameters_Task_II_1"
#########################
# Simulations Task II.2 #
#########################
for current_ratio in $(generate_log_space $((-1*$max_log_task2)) $max_log_task2 $n_rho); do #ratio=mu1/mu2
    mu2=$(echo "$lambda*2*$(bc_power 10 $max_log_task2)" | bc -l)
    mu1=$(echo "$current_ratio*$mu2" | bc -l)
    echo "$lambda 2 1 D $mu1 1 D $mu2 $maximum_simulation_time"
done > "$result_folder/Simulation_Parameters_Task_II_2"
#######################
# Run the simulations #
#######################
########################
# Simulations Task I.1 #
########################
mkdir $result_folder/Simulations_Task_I_1
pids=""
o_terminated=0
o_size=$n_rho
while read line; do
    if [ ${#line} -gt 0 ]; then
        ./simulator $line > $result_folder/Simulations_Task_I_1/results$(separate_field_with_underscore $line) 2> /dev/null &
        pids="$pids ${!}"
        if [ "$n_parallel" == "$(size $pids)" ]; then
            wait_for_all_process_in_background $pids
            pids=""
            o_terminated=$(($o_terminated+$n_parallel))
        fi
    fi
done < "$result_folder/Simulation_Parameters_Task_I_1"
wait_for_all_process_in_background $pids
echo "Ended Simulations of Task I.1"
########################
# Simulations Task I.2 #
########################
mkdir $result_folder/Simulations_Task_I_2
pids=""
o_terminated=0
o_size=$n_rho
while read line; do
    if [ ${#line} -gt 0 ]; then
        simulation_CL $line &
        pids="$pids ${!}"
        if [ "$n_parallel" == "$(size $pids)" ]; then
           wait_for_all_process_in_background $pids
            pids=""
            o_terminated=$(($o_terminated+$n_parallel))
        fi
    fi
done < "$result_folder/Simulation_Parameters_Task_I_2"
wait_for_all_process_in_background $pids
echo "Ended Simulations of Task I.2"
#########################
# Simulations Task II.1 #
#########################
mkdir $result_folder/Simulations_Task_II_1
pids=""
o_terminated=0
o_size=$n_rho
while read line; do
    if [ ${#line} -gt 0 ]; then
        ./simulator $line > $result_folder/Simulations_Task_II_1/results$(separate_field_with_underscore $line) 2> /dev/null &
        pids="$pids ${!}"
        if [ "$n_parallel" == "$(size $pids)" ]; then
           wait_for_all_process_in_background $pids
            pids=""
            o_terminated=$(($o_terminated+$n_parallel))
        fi
    fi
done < "$result_folder/Simulation_Parameters_Task_II_1"
wait_for_all_process_in_background $pids
echo "Ended Simulations of Task II.1"
#########################
# Simulations Task II.2 #
#########################
mkdir $result_folder/Simulations_Task_II_2
pids=""
o_terminated=0
o_size=$n_rho
while read line; do
    if [ ${#line} -gt 0 ]; then
       ./simulator $line > $result_folder/Simulations_Task_II_2/results$(separate_field_with_underscore $line) 2> /dev/null &
        pids="$pids ${!}"
        if [ "$n_parallel" == "$(size $pids)" ]; then
            wait_for_all_process_in_background $pids
            pids=""
            o_terminated=$(($o_terminated+$n_parallel))
        fi
    fi
done < "$result_folder/Simulation_Parameters_Task_II_2"
wait_for_all_process_in_background $pids
echo "Ended Simulations of Task II.2"
echo "Ended ALL Simulations"
#############################
# Building of the Summaries #
#############################
o_terminated=0
o_size=4
pids=""
####################
# Summary Task I.1 #
####################
build_summary I 1 "0 1" &
pids="$pids ${!}"
####################
# Summary Task I.2 #
####################
build_summary I 2 "0 1" &
pids="$pids ${!}"
#####################
# Summary Task II.1 #
#####################
build_summary II 1 "0 1" &
pids="$pids ${!}"
#####################
# Summary Task II.2 #
#####################
build_summary II 2 "0 1" &
pids="$pids ${!}"
wait_for_all_process_in_background $pids
echo "Ended the building of the summaries"
#########
# Plots #
#########
#################
# Plot Task I.1 #
#################
#x-axis: rho     y-axis: E[T]  theoretic AND simulated for the M/M/1 M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_simulated_theoretic_M-M-1_M-M-1.$exported_format\"; set title \"Performance of Network of Queues (M/M/1 - M/M/1)\"; set xlabel \"Rho\"; set ylabel \"Average Queueing Delay, E[T]   [time unit]\"; set key top left; plot \"$result_folder/Summary_Task_I_1\" u 7:1 title \"Simulated\" with lines, \"$result_folder/Summary_Task_I_1\" u 7:3 title \"Theoretic\" with lines lc rgb \"blue\";"
gnuplot -e "$plot_cmd" #2> /dev/null
#x-axis: rho     y-axis: E[T]  simulated for the M/M/1 M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_simulated_M-M-1_M-M-1.$exported_format\"; set title \"Performance of Network of Queues (M/M/1 - M/M/1)\"; set xlabel \"Rho\"; set ylabel \"Average Queueing Delay, E[T]   [time unit]\"; set key top left; plot \"$result_folder/Summary_Task_I_1\" u 7:1 title \"Simulated\" with lines;"
gnuplot -e "$plot_cmd" #2> /dev/null
#x-axis: rho     y-axis: E[T]  theoretic for the M/M/1 M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_theoretic_M-M-1_M-M-1.$exported_format\"; set title \"Performance of Network of Queues (M/M/1 - M/M/1)\"; set xlabel \"Rho\"; set ylabel \"Average Queueing Delay, E[T]   [time unit]\"; set key top left; plot \"$result_folder/Summary_Task_I_1\" u 7:2 title \"Theoretic\" with lines;"
gnuplot -e "$plot_cmd" #2> /dev/null
#x-axis: rho     y-axis: E[T]  error for the M/M/1 M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_error_M-M-1_M-M-1.$exported_format\"; set title \"Performance of Network of Queues (M/M/1 - M/M/1)\"; set xlabel \"Rho\"; set ylabel \"Relative Error: Average Queueing Delay   [%]\"; set key top left; plot \"$result_folder/Summary_Task_I_1\" u 7:3 title \"Relative Error\" with lines;"
gnuplot -e "$plot_cmd" #2> /dev/null
#################
# Plot Task I.2 #
#################
#x-axis: rho     y-axis: E[T]  confidence level for the M/M/1 M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_confidence_level_M-M-1_M-M-1.$exported_format\"; set title \"Performance of Network of Queues (M/M/1 - M/M/1)\"; set xlabel \"Rho\"; set ylabel \"Average Queueing Delay, E[T]   [time unit]\"; set key top left; plot \"$result_folder/Summary_Task_I_2\" u 1:2 title \"Average Measure\" with lines lw 2 lc rgb \"black\", \"$result_folder/Summary_Task_I_2\" u 1:(\$2-$(C_Interval_size 0.95)) title \"Confidence $(to_percentage 0.95)%\" with lines lw 4 lt 0 lc rgb \"red\", \"$result_folder/Summary_Task_I_2\" u 1:(\$2+$(C_Interval_size 0.95)) notitle with lines lw 4 lt 0 lc rgb \"red\", \"$result_folder/Summary_Task_I_2\" u 1:(\$2-$(C_Interval_size 0.99)) title \"Confidence $(to_percentage 0.99)%\" with lines lw 4 lt 0 lc rgb \"blue\", \"$result_folder/Summary_Task_I_2\" u 1:(\$2+$(C_Interval_size 0.99)) notitle with lines lw 4 lt 0 lc rgb \"blue\";"
gnuplot -e "$plot_cmd" #2> /dev/null
#x-axis: rho     y-axis: E[T]  percentage size of the interval for the M/M/1 M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_confidence_interval_size_M-M-1_M-M-1.$exported_format\"; set title \"Performance of Network of Queues (M/M/1 - M/M/1)\"; set xlabel \"Rho\"; set ylabel \"Interval Size of E[T] respect the Average,    [%]\"; set key top left; plot \"$result_folder/Summary_Task_I_2\" u 1:(100*$(C_Interval_size 0.95)/\$2) title \"Size of the Confidence Interval of $(to_percentage 0.95)%\" with lines, \"$result_folder/Summary_Task_I_2\" u 1:(100*$(C_Interval_size 0.99)/\$2) title \"Size of the Confidence Interval of $(to_percentage 0.99)%\" with lines lc rgb \"blue\";"
gnuplot -e "$plot_cmd" #2> /dev/null
#x-axis: rho     y-axis: E[T]  confidence level for the M/M/1 M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_confidence_level_M-M-1_M-M-1_zoom.$exported_format\"; set title \"Performance of Network of Queues (M/M/1 - M/M/1)\"; set xlabel \"Rho\"; set ylabel \"Average Queueing Delay, E[T]   [time unit]\"; set key top left; set xrange [$start_rho:$end_rho]; set xtics 0.1,$(remove_final_zeros $(echo "1.0/($n_rho+1)" | bc -l)),0.99; plot \"$result_folder/Summary_Task_I_2\" u 1:2 title \"Average Measure\" with linespoints lw 2 pt 7 ps 0.375 lc rgb \"black\", \"$result_folder/Summary_Task_I_2\" u 1:(\$2-$(C_Interval_size 0.95)) title \"Confidence $(to_percentage 0.95)%\" with lines lw 4 lt 0 lc rgb \"red\", \"$result_folder/Summary_Task_I_2\" u 1:(\$2+$(C_Interval_size 0.95)) notitle with lines lw 4 lt 0 lc rgb \"red\", \"$result_folder/Summary_Task_I_2\" u 1:(\$2-$(C_Interval_size 0.99)) title \"Confidence $(to_percentage 0.99)%\" with lines lw 4 lt 0 lc rgb \"blue\", \"$result_folder/Summary_Task_I_2\" u 1:(\$2+$(C_Interval_size 0.99)) notitle with lines lw 4 lt 0 lc rgb \"blue\";"
gnuplot -e "$plot_cmd" #2> /dev/null
#x-axis: rho     y-axis: E[T]  theoretic AND simulated for the M/M/1 M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_averageSimulated_theoretic_M-M-1_M-M-1.$exported_format\"; set title \"Performance of Network of Queues (M/M/1 - M/M/1)\"; set xlabel \"Rho\"; set ylabel \"Average Queueing Delay, E[T]   [time unit]\"; set key top left; plot \"$result_folder/Summary_Task_I_2\" u 1:2 title \"Average Simulated\" with lines, \"$result_folder/Summary_Task_I_2\" u 1:5 title \"Theoretic\" with lines lc rgb \"blue\";"
gnuplot -e "$plot_cmd" #2> /dev/null
#x-axis: rho     y-axis: E[T]  simulated for the M/M/1 M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_averageSimulated_M-M-1_M-M-1.$exported_format\"; set title \"Performance of Network of Queues (M/M/1 - M/M/1)\"; set xlabel \"Rho\"; set ylabel \"Average Queueing Delay, E[T]   [time unit]\"; set key top left; plot \"$result_folder/Summary_Task_I_2\" u 1:2 title \"Average Simulated\" with lines;"
gnuplot -e "$plot_cmd" #2> /dev/null
#x-axis: rho     y-axis: E[T]  error for the M/M/1 M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[T]_error_averageSimulated_M-M-1_M-M-1.$exported_format\"; set title \"Performance of Network of Queues (M/M/1 - M/M/1)\"; set xlabel \"Rho\"; set ylabel \"Relative Error: Average Queueing Delay   [%]\"; set key top left; plot \"$result_folder/Summary_Task_I_2\" u 1:5 title \"Relative Error\" with lines;"
gnuplot -e "$plot_cmd" #2> /dev/null
##################
# Plot Task II.1 #
##################
#rho1=l/(c*mu1) rho2=l/(c*mu2) rho2/rho1=(l/(c*mu2))/(l/(c*mu1))=mu1/mu2
#x-axis: mu1/mu2=rho2/rho1     y-axis: E[L_i]  simulated for the M/M/1 M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[L_i]_simulated_M-M-1_M-M-1.$exported_format\"; set title \"Performance of Network of Queues (M/M/1 - M/M/1)\"; set xlabel \"Rho2/Rho1\"; set ylabel \"Average Customers in the System, E[L(i)]\"; set key top right; set mxtics 2; set mytic 2; set mxtics 2; set format y \"%.0e\"; set format x \"%.0e\"; set grid xtics mxtics ytics mytics lw 0.5; set logscale xy; plot \"$result_folder/Summary_Task_II_1\" u (\$14/\$7):4 title \"Simulated (Queue Network)\" with lines lc rgb \"black\", \"$result_folder/Summary_Task_II_1\" u (\$14/\$7):11 title \"Simulated (Queue 1)\" with lines lc rgb \"red\", \"$result_folder/Summary_Task_II_1\" u (\$14/\$7):18 title \"Simulated (Queue 2)\" with lines lc rgb \"blue\";"
gnuplot -e "$plot_cmd" #2> /dev/null
#x-axis: mu1/mu2=rho2/rho1     y-axis: E[L_i]/E[L]  simulated for the M/M/1 M/M/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_percentage_E[L_i]_simulated_M-M-1_M-M-1.$exported_format\"; set title \"Performance of Network of Queues (M/M/1 - M/M/1)\"; set xlabel \"Rho2/Rho1\"; set ylabel \"Average Customers in the System\"; set key bottom center; set mytics 2; set mxtics 2; set format y \"%.0e\"; set format x \"%.0e\"; set grid xtics mxtics ytics mytics lw 0.5; set logscale xy; plot \"$result_folder/Summary_Task_II_1\" u (\$14/\$7):(\$11/\$4) title \"Percentage on Queue 1\" with lines lc rgb \"red\", \"$result_folder/Summary_Task_II_1\" u (\$14/\$7):(\$18/\$4) title \"Percentage on Queue 2\" with lines lc rgb \"blue\";"
gnuplot -e "$plot_cmd" #2> /dev/null
##################
# Plot Task II.2 #
##################
#rho1=l/(c*mu1) rho2=l/(c*mu2) rho2/rho1=(l/(c*mu2))/(l/(c*mu1))=mu1/mu2
#x-axis: mu1/mu2=rho2/rho1     y-axis: E[L_i]  simulated for the M/D/1 M/D/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_E[L_i]_simulated_M-D-1_M-D-1.$exported_format\"; set title \"Performance of Network of Queues (M/D/1 - M/D/1)\"; set xlabel \"Rho2/Rho1\"; set ylabel \"Average Customers in the System, E[L(i)]\"; set key top right; set mxtic 2; set mytic 2; set mxtics 2; set format y \"%.0e\"; set format x \"%.0e\"; set grid xtics mxtics ytics mytics lw 0.5; set logscale xy; plot \"$result_folder/Summary_Task_II_2\" u (\$14/\$7):4 title \"Simulated (Queue Network)\" with lines lc rgb \"black\", \"$result_folder/Summary_Task_II_2\" u (\$14/\$7):11 title \"Simulated (Queue 1)\" with lines lc rgb \"red\", \"$result_folder/Summary_Task_II_2\" u (\$14/\$7):18 title \"Simulated (Queue 2)\" with lines lc rgb \"blue\";"
gnuplot -e "$plot_cmd" #2> /dev/null
#x-axis: mu1/mu2=rho2/rho1     y-axis: E[L_i]/E[L]  simulated for the M/D/1 M/D/1
plot_cmd="set terminal $exported_format; set output \"$result_folder/performance_percentage_E[L_i]_simulated_M-D-1_M-D-1.$exported_format\"; set title \"Performance of Network of Queues (M/D/1 - M/D/1)\"; set xlabel \"Rho2/Rho1\"; set ylabel \"Average Customers in the System\"; set key bottom center; set mytics 2; set mxtics 2; set format y \"%.0e\"; set format x \"%.0e\"; set grid xtics mxtics ytics mytics lw 0.5; set logscale xy; plot \"$result_folder/Summary_Task_II_2\" u (\$14/\$7):(\$11/\$4) title \"Percentage on Queue 1\" with lines lc rgb \"red\", \"$result_folder/Summary_Task_II_2\" u (\$14/\$7):(\$18/\$4) title \"Percentage on Queue 2\" with lines lc rgb \"blue\";"
gnuplot -e "$plot_cmd" #2> /dev/null
############
# Cleaning #
############
rm -rf $result_folder/Simulation_Parameters_* 2> /dev/null
cd $result_folder
zip -qR raw_results.zip $(ls -a *) 2> /dev/null
rm -rf $(ls -1 | grep -vE "raw_results.zip|*.pdf")
cd ..
echo "Ended the compression of the results"
