<p align="center">
  <h1 align="center"> ChampSim EVA</h1>
  <p> ChampSim is a trace-based simulator for a microarchitecture study. We have modified the base implementation of Champsim to include the EVA replacement policy <p>
</p>

# Using ChampSim EVA

We have provided the following scripts for creating traces and running Champsim with the required variables:

* pintracer.sh - This script compiles check.c(in pin folder) and runs the PinTool to trace the instructions to feed into Champsim. The output trace is dumped in traces/ folder outside Champsim folder. **Ensure**  that the folder is created before running the tool. Provide 'y' command when prompted to compress the trace.

* run.sh - This script runs champsim on the trace stored in traces/ folder and then plots the EVA graph and creates a gif in the analysis folder. **Ensure** that two folders called img and img2 exist in the analysis folder. **Ensure**  that "eva" replacement policy is attached to the required cache in champsim_config.json.

* run_all.sh - This script runs multiple benchmark program across multiple replacement policies(LRU, DRRIP and EVA) for the LLC and outputs the cache statistics. Ensure that only 1 cache has explicitly declared a replacement policy, others default to LRU. Traces are to be placed in traces/CRC2_trace/ [Link](http://bit.ly/2t2nkUj) outside Champsim folder. **Ensure**  that the result folder is created before running the tool.
