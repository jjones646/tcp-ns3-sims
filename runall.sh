#!/bin/bash

# Run all of the simulation sets for Project 1 in parallel.

kill_all_sims ()
{
    # Get our process group id
    PGID=$(ps -o pgid= $$ | grep -o [0-9]*)
    # Kill it in a new new process group
    setsid kill -- -$PGID

    on_exit 1
}

on_exit ()
{
    popd &>/dev/null

    if [ "$1" ]; then
        exit $1
    else
        exit 0
    fi
}

trap kill_all_sims SIGINT
trap on_exit EXIT


WINDOW_SIZES=(2000 8000 32000 64000)
QUEUE_LIMITS=(2000 8000 32000 64000)
SEGMENT_SIZES=(128 256 512)
NUM_FLOWS=(1 10)

pushd "$NS3DIR" &>/dev/null

for n in "${NUM_FLOWS[@]}"; do
    for i in "${WINDOW_SIZES[@]}"; do
        for j in "${QUEUE_LIMITS[@]}"; do
            for k in "${SEGMENT_SIZES[@]}"; do
                WAF_CMD="p1 --segSize=$k --winSize=$i --queueSize=$j --nFlows=$n"
                "$NS3DIR"/waf --run "$WAF_CMD" &
            done
        done
        # wait for these to finish before starting up another set of simulations
        wait
    done
done

