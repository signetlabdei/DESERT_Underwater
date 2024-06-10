#!/bin/bash

#rm result.csv

# Iterate over the parameters and execute the command for each one
for rng in $(seq 36 1 37)
do
    rm log/error_log.csv
    rm log/position_log.csv
    rm log/position_log_a.csv
    rm log/true_error_log.csv
    rm log.out

    ns test_uwauv_error_basic.tcl 0.001 $2 $3 $rng >> log.out
    #ns test_uwauv_error.tcl 0.001 0.01 0.1 0
    python metric_comp_4.py 0.001 $2 $3 $rng >> result_2.csv

done

for rng in $(seq 36 1 37)
do
    rm log/error_log.csv
    rm log/position_log.csv
    rm log/position_log_a.csv
    rm log/true_error_log.csv
    rm log.out

    ns test_uwauv_error_basic.tcl 0.01 $2 $3 $rng >> log.out
    #ns test_uwauv_error.tcl 0.001 0.01 0.1 0
    python metric_comp_4.py 0.01 $2 $3 $rng >> result_2.csv

done

for rng in $(seq 36 1 37)
do
    rm log/error_log.csv
    rm log/position_log.csv
    rm log/position_log_a.csv
    rm log/true_error_log.csv
    rm log.out

    ns test_uwauv_error_basic.tcl 0.05 $2 $3 $rng >> log.out
    #ns test_uwauv_error.tcl 0.001 0.01 0.1 0
    python metric_comp_4.py 0.05 $2 $3 $rng >> result_2.csv

done


for rng in $(seq 36 1 37)
do
    rm log/error_log.csv
    rm log/position_log.csv
    rm log/position_log_a.csv
    rm log/true_error_log.csv
    rm log.out

    ns test_uwauv_error_basic.tcl 0.25 $2 $3 $rng >> log.out
    #ns test_uwauv_error.tcl 0.001 0.01 0.1 0
    python metric_comp_4.py 0.25 $2 $3 $rng >> result_2.csv

done


for rng in $(seq 27 1 31)
do
    rm log/error_log.csv
    rm log/position_log.csv
    rm log/position_log_a.csv
    rm log/true_error_log.csv

    ns test_uwauv_error_basic.tcl 0.1 $2 $3 $rng >> log.out
    #ns test_uwauv_error.tcl 0.001 0.01 0.1 0
    python metric_comp_4.py 0.1 $2 $3 $rng >> result_2.csv

done

python result_compute.py 

