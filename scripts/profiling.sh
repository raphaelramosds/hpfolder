#!/bin/sh
PARENT=$(dirname "$(pwd)")

# Output
# OUTPUT_GPROF=$PARENT/scripts/log/gprof.txt

# touch $OUTPUT_GPROF
cd $PARENT

# Parameters
P=WWBWWBBWWBBWWWWWBBBBBBBBBBWWWWWWBBWWBBWWBWWBBBBB
m=0.025
c=0.90
e=1000
m=-23
s=100

# Needed flags
FLAGS="-lGL -lGLU -lglut -lboost_system -lboost_thread -pg" 

# Compile code
g++ *.cpp *.hpp -o hpfolder $FLAGS

# Execute code
./hpfolder -e $e -y $m -p $P -s $s -m $m -c $c 

# Analysis with gprof
gprof --ignore-non-functions ./hpfolder | gprof2dot -s -w | dot -Tpng -o ./scripts/log/graph.png
echo "Done"
