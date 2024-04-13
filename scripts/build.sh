#!/bin/sh
PARENT=$(dirname "$(pwd)")

# Output
OUTPUT_VECT=$PARENT/scripts/log/vect.txt
OUTPUT_GPROF=$PARENT/scripts/log/gprof.txt

touch $OUTPUT_VECT $OUTPUT_GPROF
cd $PARENT

# Parameters
P=BBBBWWWWBBBBBBBBBBBBWWWWWWBBBBBBBBBBBBWWWBBBBBBBBBBBBWWWBBBBBBBBBBBBWWWBWWBBWWBBWWBWB
m=0.025
c=0.90
e=10
m=-52
s=10

# Needed flags
FLAGS="-lGL -lGLU -lglut -lboost_system -lboost_thread -pg -O3 -fopt-info-vec-missed" 

# Compile code
g++ *.cpp *.hpp -o hpfolder $FLAGS 2>&1 | tee > $(grep "couldn't vectorize loop" > $OUTPUT_VECT)

# Execute code
./hpfolder -e $e -y $m -p $P -s $s -m $m -c $c 

# Analysis with gprof
gprof ./hpfolder > $OUTPUT_GPROF

echo "Done"