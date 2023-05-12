#!/bin/bash

files="../traces/CRC2_trace/*"
result_file="./res"

all_policies="lru eva drrip ship srrip"

test_policies="drrip lru eva"

rm $result_file/*

echo The traces scheduled to run are $files

for repl_policy in $test_policies
do
    echo Replacement policy is $repl_policy | tee $result_file/$repl_policy
    sed -i "s/\($(echo $all_policies | sed 's/ /\\|/g')\)/$repl_policy/" champsim_config.json
    cp champsim_config.json $result_file/champsim_config_$repl_policy.json

    ./config.sh champsim_config.json
    make

    for f in $files
    do
        echo running trace $f | tee -a $result_file/$repl_policy
        bin/champsim --warmup_instructions 1000000 --simulation_instructions 40000000 $f | tee -a $result_file/$repl_policy
    done
done

grep "LLC TOTAL" $result_file/*
