#!/bin/bash

make clean

cp Defines/${1-default}.h Defines.h

make \
ARCH=sm_75 \
MOVING_WINDOW=1 \
CPU_CHECK=1 \
USERFLAGS=-pg
