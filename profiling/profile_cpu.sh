#!/bin/bash

start_npts=100000
inc=10000
end_npts=200000
i=$start_npts; while [ $i -le $end_npts ];
do
    cd ..
    make -f Makefile.cpu clean
    make -f Makefile.cpu grid_cpu MOVING_WINDOW=1 USERFLAGS=-DNPOINTS=$i
    cd -
    ../grid_cpu -d 2>&1 >> prof_cpu.txt
    for j in {1..3}; do ../grid_cpu 2>&1 >> prof_cpu.txt; done
    echo >> prof_cpu.txt
    i=$(($i+$inc))
done

echo "Done profiling CPU"
