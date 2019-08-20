#!/bin/bash

g++ -O2 -FC -fpermissive -Wall -DDEVELOPMENT=0 -o unix_platform.out unix_platform.c -lm -lSDL2

exit 0