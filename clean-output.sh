#!/bin/bash

STDOUT_RESULTS="$(readlink -m "$1")"

segSizes=(128 256 512)
winSizes=(2000 8000 32000 64000)
queueSizes=(2000 8000 32000 64000)

for i in "${segSizes[@]}"; do
    for j in "${winSizes[@]}"; do
        for k in "${queueSizes[@]}"; do
            cat "$STDOUT_RESULTS" | grep "tcp,0," | grep "segSize,${i}" | grep "windowSize,${j}" | grep "queueSize,${k}"  >> results-tahoe.csv
            cat "$STDOUT_RESULTS" | grep "tcp,1," | grep "segSize,${i}" | grep "windowSize,${j}" | grep "queueSize,${k}"  >> results-reno.csv
        done
    done
done

