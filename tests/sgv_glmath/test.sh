#!/bin/bash

gcc -std=c89 -pedantic -Wall -O2 test.c -I../../ -o out -lm && ./out
