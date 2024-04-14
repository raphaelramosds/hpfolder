#!/bin/sh
PARENT=$(dirname "$(pwd)")

# Output
OUTPUT_VECT=$PARENT/scripts/log/vect.txt

touch $OUTPUT_VECT
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

echo "Done"