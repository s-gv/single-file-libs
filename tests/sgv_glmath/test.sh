#!/bin/bash
gcc -Wall -O2 test.c -I../../ -o out -lm && ./out
