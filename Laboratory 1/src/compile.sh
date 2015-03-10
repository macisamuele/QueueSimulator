#!/bin/bash
if [ $# -eq 0 ]
then
    gcc -Wall -Wextra *.c -o simulator -lm
else
    gcc -Wall -Wextra *.c -o simulator -lm -DSTAT
fi
