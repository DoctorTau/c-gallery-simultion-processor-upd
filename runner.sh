#!/bin/bash

# Start the server
./build/bin/server 5678 &

# Start 100 instances of the client
for ((i=0; i<100; i++))
do
    port=$((5679+i))
    ./build/bin/client 127.0.0.1 5678 $port &
done

# Wait for all clients to finish
wait