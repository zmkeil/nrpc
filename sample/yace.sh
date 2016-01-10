#!/bin/bash

while [[ 1 -eq 1 ]]
do
    ./client 112.126.80.217 1>>./logs/client_access.log 2>&1
    sleep 60
done
