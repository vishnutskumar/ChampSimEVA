#!/bin/bash
./config.sh  champsim_config.json
make
bin/champsim --warmup_instructions 5000000 --simulation_instructions 10000000 ../traces/ls_trace.champsim.gz
cd analysis
python3 plot_AgevEVA.py
