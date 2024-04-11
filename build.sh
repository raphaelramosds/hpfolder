#!/bin/sh
PROTEIN=BWBWWBBWBWWBWBBWWBWB

g++ *.cpp *.hpp -o hpfolder -lGL -lGLU -lglut -lboost_system -lboost_thread -pg

./hpfolder -e 100 -y -9 -p $PROTEIN -s 100 -m 0.05 -c 0.90 -g --ignore_rest
