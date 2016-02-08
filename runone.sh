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

function run_waf {
    CWD="$PWD"
    cd $NS3DIR >/dev/null
    ./waf --cwd="$CWD" "$@"
    cd - >/dev/null
}

trap kill_all_sims SIGINT
trap on_exit EXIT

# create directory for the results and make it our working directory
DIR_NAME="$(date +%Y_%m_%d_%H_%M_%S)_results"
mkdir "$DIR_NAME"
pushd "$DIR_NAME" &>/dev/null

# set the environment's log level
export NS_LOG=

# make sure everything is built first
run_waf --run "p1 --nFlowBytes=100" 

# set what values we iterate over here
WINDOW_SIZES=(64000 2000)
QUEUE_LIMITS=(64000)
SEGMENT_SIZES=(512)
NUM_FLOWS=(1 10)

BYTES_PER_FLOW=100000000

# tcp tahoe
TCP_TYPE=0
for n in "${NUM_FLOWS[@]}"; do
    for i in "${WINDOW_SIZES[@]}"; do
        for j in "${QUEUE_LIMITS[@]}"; do
            for k in "${SEGMENT_SIZES[@]}"; do
                OUTPUT_FILENAME_BASE="trace_tcp-${TCP_TYPE}_win-${i}_seg-${k}_queue-${j}_flows-${n}"
                WAF_CMD="p1 --segSize=$k --windowSize=$i --queueSize=$j --nFlows=$n --nFlowBytes=$BYTES_PER_FLOW --tcpType=$TCP_TYPE --trace=false --traceFile=$OUTPUT_FILENAME_BASE"
                run_waf --run "$WAF_CMD" &
            done
        done
    done
    # wait for these to finish before starting up another set of simulations
    wait
done
wait

