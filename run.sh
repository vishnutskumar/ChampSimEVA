#!/bin/bash
./config.sh  champsim_config.json
make
bin/champsim --warmup_instructions 1000000 --simulation_instructions 2000000 ../traces/ls_trace.champsim.gz
