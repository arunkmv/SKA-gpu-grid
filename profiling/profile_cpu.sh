#!/bin/bash

vary_param='gcf'
filename='prof_cpu.txt'
num_times=5

start=1
inc=1
end=2
userflag=

if [ $vary_param == 'vis' ]
then
    start=10000
    inc=5000
    end=1600000
    userflag='USERFLAGS=-DNPOINTS'
elif [ $vary_param == 'gcf' ]
then
    start=8
    inc=8
    end=512
    userflag='USERFLAGS=-DGCF_DIM'
else
    echo "No such parameter to vary"
    exit 1
fi

i=$start; while [ $i -le $end ];
do
    cd ..
    make -f Makefile.cpu clean
    make -f Makefile.cpu grid_cpu MOVING_WINDOW=1 $userflag=$i
    cd -
    ../grid_cpu -d 2>&1 >> $filename
    for j in $(seq 1 1 $num_times); do ../grid_cpu 2>&1 >> $filename; done
    echo >> $filename
    i=$(($i+$inc))
done

echo "Done profiling CPU. Results in " $filename
