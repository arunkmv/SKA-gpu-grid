#!/usr/bin/sh

make \
ARCH=sm_75 \
MOVING_WINDOW=1 \
CPU_CHECK=0 \
USERFLAGS=-pg
