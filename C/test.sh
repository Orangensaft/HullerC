#!/bin/sh
./huller_$1 learn a1a.t2 124 >m
./huller_$1 classify a1a.t2 m 124 >o
