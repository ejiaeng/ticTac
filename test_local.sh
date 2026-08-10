#!/bin/bash

set -e

gcc server/serv.c server/handle.c -lpthread -o server/server
gcc client/client.c -o client/client

cd server
./server &
SERVER_PID=$!
cd ..

sleep 1

cd client
./client
cd ..

kill $SERVER_PID
wait $SERVER_PID 2>/dev/null
