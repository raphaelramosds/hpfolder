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
s=128

# Bootstrap
PARENT=$(dirname "$(pwd)")
LOG=$PARENT/scripts/log

# Outputs
OUTPUT_GPROF=$LOG/gprof.txt
OUTPUT_VECT=$LOG/vect.txt
OUTPUT_GRAPH_POP=$LOG/graph_pop.png
OUTPUT_GRAPH_CALC=$LOG/graph_calc.png

touch $OUTPUT_VECT $OUTPUT_GPROF
cd $PARENT

# Needed flags
FLAGS="-lGL -lGLU -lglut -lboost_system -lboost_thread -pg -O3 -fopt-info-vec-all" 

# Compile code
echo "Compiling with vectorization"
g++ *.cpp *.hpp -o hpfolder $FLAGS 2>&1 | tee > $(grep -v '^/usr' > $OUTPUT_VECT)

# Execute code
echo "Executing code"
./hpfolder -e $e -y $m -p $P -s $s -m $m -c $c 

# Analysis with gprof
echo "Analysis with gprof"
gprof --ignore-non-functions ./hpfolder > $OUTPUT_GPROF
gprof2dot -s -w -n0 -e0 -z "calculation(Population*)" $OUTPUT_GPROF | dot -Tpng -o $OUTPUT_GRAPH_CALC
gprof2dot -s -w -n0 -e0 -z "Population::Population(int, Protein, float, float)" $OUTPUT_GPROF | dot -Tpng -o $OUTPUT_GRAPH_POP

echo "Done"