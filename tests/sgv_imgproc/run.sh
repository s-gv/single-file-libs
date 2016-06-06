#!/bin/bash

gcc -std=c89 -pedantic -Wall -O2 -I../../ test.c -o out -lm
./out
rm -f out
