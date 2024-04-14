#!/bin/sh

# Protein
P=BBBBBBBBBBBBWBWBWWBBWWBBWWBWWBBWWBBWWBWWBBWWBBWWBWBWBBBBBBBBBBBB

# Mutation
m=0.025 

# Crossover
c=0.90  

# Energy evaluations
e=1000  

# Minimun energy
m=-40  

# Bootstrap
PARENT=$(dirname "$(pwd)")
LOG=$PARENT/scripts/log

rm $LOG/*

cd $PARENT

# Needed flags
FLAGS="-lGL -lGLU -lglut -lboost_system -lboost_thread -pg -O3" 

# Compiling with vectorization
g++ *.cpp *.hpp -o hpfolder $FLAGS

for i in 16 32 64 128 256 512 1024; do
    # Compile code
    echo "Compiling with vectorization (Population = $i)"

    # Execute code
    echo "Executing code"
    ./hpfolder -e $e -y $m -p $P -s $i -m $m -c $c 

    # Analysis with gprof
    OUTPUT_GPROF=$LOG/gprof_$i.txt
    echo "Profiling in progress"
    gprof --ignore-non-functions ./hpfolder > $OUTPUT_GPROF

    # Analysis with gprof2dot
    echo "Generating graph"
    gprof2dot -s -w -n0 -e0 -z "calculation(Population*)" $OUTPUT_GPROF | dot -Tpng -o $LOG/graph_calc_$i.png
    gprof2dot -s -w -n0 -e0 -z "Population::Population(int, Protein, float, float)" $OUTPUT_GPROF | dot -Tpng -o $LOG/graph_pop_$i.png

    printf "\n"
done

echo "Got shit done"