#!/bin/sh

# Protein
P=WWBWWBBWWBBWWWWWBBBBBBBBBBWWWWWWBBWWBBWWBWWBBBBB

# Mutation
m=0.025 

# Crossover
c=0.90  

# Energy evaluations
e=1000  

# Minimun energy
m=-23   

# Population size
s=100   

# Bootstrap
PARENT=$(dirname "$(pwd)")
LOG=$PARENT/scripts/log

# Outputs
OUTPUT_GPROF=$LOG/gprof.txt
OUTPUT_VECT=$LOG/vect.txt
OUTPUT_GRAPH=$LOG/graph.png

touch $OUTPUT_VECT $OUTPUT_GPROF
cd $PARENT

# Needed flags
FLAGS="-lGL -lGLU -lglut -lboost_system -lboost_thread -pg -O3 -fopt-info-vec-missed" 

# Compile code
echo "Compiling with vectorization"
g++ *.cpp *.hpp -o hpfolder $FLAGS 2>&1 | tee > $(grep "couldn't vectorize loop" > $OUTPUT_VECT)

# Execute code
echo "Executing code"
./hpfolder -e $e -y $m -p $P -s $s -m $m -c $c 

# Analysis with gprof
echo "Analysis with gprof"
gprof --ignore-non-functions ./hpfolder | tee $OUTPUT_GPROF| gprof2dot -s -w | dot -Tpng -o $OUTPUT_GRAPH

echo "Done"