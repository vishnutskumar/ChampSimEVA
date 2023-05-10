#!/bin/bash

dir=$(pwd)
gcc -O0 -o check $dir/tracer/pin/check.c
echo "Compiled Check.c"
parentdir="$(dirname "$dir")"
trace_dir=$parentdir/traces
$PIN_ROOT/pin -t $dir/tracer/pin/obj-intel64/champsim_tracer.so -o $trace_dir/ls_trace.champsim -s 350000 -t 50000000 -- $dir/check
echo "Traced check"
gzip $trace_dir/ls_trace.champsim
echo "Gzip Done"
