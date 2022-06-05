#!/bin/bash
g++ Stress_Test/stress_test.cpp -o ./bin/stress  --std=c++11 -pthread  -fpermissive
./bin/stress