#!/usr/bin/env bash

sudo nvprof -f --export-profile ${1-some_run}.nvvp ../grid
nvprof --import-profile ${1-some_run}.nvvp
