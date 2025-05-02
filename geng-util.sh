#!/bin/bash

output_file="graphs/14-graphs.graph6"
: > "$output_file"  # Truncate the output file

for x in $(seq 0 91); do
	/opt/homebrew/opt/coreutils/libexec/gnubin/timeout 2 geng 14 $x:$x 2>/dev/null | head -15000 >> "$output_file"
done
