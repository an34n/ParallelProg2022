#!/bin/bash

rm trace.txt

for threads in {1..12}
do
    echo -e "\nRUNNING ON THREADS $threads"
    for seed in {1000..300..-7}
    do
        echo -n "."
        mpicc lab5.cpp -o lab5.out -D RANDOM_SEED=$seed
        mpirun -np $threads ./lab5.out
    done
    echo "" >> trace.txt
done
echo "Done?"