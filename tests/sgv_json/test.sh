#!/bin/bash

gcc -std=c89 -Wall -Wpedantic -Wextra -O2 test.c -I../../ -o out && ./out
