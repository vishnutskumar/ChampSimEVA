#!/bin/bash

dir=$(pwd)
g++ -O0 -o check $dir/tracer/pin/check.cpp
echo "Compiled Check.cpp"
parentdir="$(dirname "$dir")"
trace_dir=$parentdir/traces
$PIN_ROOT/pin -t $dir/tracer/pin/obj-intel64/champsim_tracer.so -o $trace_dir/ls_trace.champsim -t 50000000 -- $dir/tracer/pin/check
echo "Traced check"
gzip $trace_dir/ls_trace.champsim
echo "Gzip Done"
