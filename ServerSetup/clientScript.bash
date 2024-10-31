#!/bin/bash

# Number of times to execute nc
num_executions=1258

for ((i = 1; i <= num_executions; i++)); do
    # Run nc in the background to avoid blocking
    nc localhost 8080 &
done

# Wait for all background jobs to complete
wait
