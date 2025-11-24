#!/bin/bash

echo "hello"     | nc -w 1 127.0.0.1 1234
echo ""
echo "/stats"    | nc -w 1 127.0.0.1 1234
echo ""
echo "/time"     | nc -w 1 127.0.0.1 1234
echo ""
echo "/shutdown" | nc -w 1 127.0.0.1 1234
echo ""

