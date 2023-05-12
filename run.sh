#!/bin/bash
./config.sh champsim_config.json
make

bin/champsim --warmup_instructions 100000 --simulation_instructions 4000000 ../traces/CRC2_trace/astar_23B.trace.xz
cd analysis
python3 plot_AgevEVA.py
