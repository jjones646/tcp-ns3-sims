#!/bin/bash

# Run all of the simulation sets for Project 1 in parallel.

WINDOW_SIZES=(2000 8000 32000 64000)
QUEUE_LIMITS=(2000 8000 32000 64000)
SEGMENT_SIZES=(128 256 512)
NUM_FLOWS=(1 10)

pushd $NS3DIR >/dev/null

for n in "${NUM_FLOWS[@]}"; do
    for i in "${WINDOW_SIZES[@]}"; do
        for j in "${QUEUE_LIMITS[@]}"; do
            for k in "${SEGMENT_SIZES[@]}"; do
                WAF_CMD="p1 --segSize=$k --winSize=$i --queueSize=$j --nFlows=$n"
                $NS3DIR/waf --run \'$WAF_CMD\' &
            done
        done
        # wait here before starting up more processes
        wait
    done
done

wait

popd >/dev/null

