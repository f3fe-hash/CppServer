#!/bin/bash
URL="http://192.168.1.28:8080"
while true; do
    curl -s "$URL" > /dev/null &
done