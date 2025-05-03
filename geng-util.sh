#!/bin/bash

geng 6 > 6-graphs.g6

geng 9 > 9-graphs.g6

for x in $(seq 0 91); do
	geng 14 $x:$x | head -15000 >> 14-graphs.g6
done

for x in $(seq 0 153); do
	geng 18 $x:$x | head -10000 >> 18-graphs.g6
done