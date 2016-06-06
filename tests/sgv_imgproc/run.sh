#!/bin/bash

gcc -std=c89 -pedantic -Wall -O2 -fsanitize=address -fno-omit-frame-pointer -I../../ test.c -o out -lm
./out
rm -f out
