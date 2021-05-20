#!/bin/bash

vary_param='GCF_DIM'
filename='prof_cpu.txt'
num_times=5

start=1
inc=1
end=2

if [ $vary_param == 'NPOINTS' ]
then
    start=10000
    inc=5000
    end=1600000
elif [ $vary_param == 'GCF_DIM' ]
then
    start=8
    inc=8
    end=512
else
    echo "No such parameter to vary"
    exit 1
fi

echo "# Varying $vary_param, start = $start inc = $inc end = $end" > $filename

i=$start; while [ $i -le $end ];
do
    cd ..
    make -f Makefile.cpu clean
    make -f Makefile.cpu grid_cpu MOVING_WINDOW=1 USERFLAGS="-D$vary_param=$i"
    cd -
    ../grid_cpu -d 2>&1 >> $filename
    for j in $(seq 1 1 $num_times); do ../grid_cpu 2>&1 >> $filename; done
    echo >> $filename
    i=$(($i+$inc))
done

echo "Done profiling CPU. Results in " $filename
