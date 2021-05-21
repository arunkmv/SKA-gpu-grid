#!/bin/bash

vary_param='NPOINTS'
num_times=5

NVPROF=nvprof
dump_dir='run_1'
filename='prof_gpu.txt'

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
mkdir -p $dump_dir

conf_count=0
i=$start; while [ $i -le $end ];
do
    cd ..
    make -f Makefile clean
    make -f Makefile grid \
        MOVING_WINDOW=1 \
        ARCH=sm_75 \
        CPU_CHECK=0 \
        USERFLAGS="-pg -D$vary_param=$i -D__PROF_MODE"
    cd -
    for j in $(seq 1 1 $num_times);
    do
        run_name="$conf_count"_"$j"
        echo $run_name >> $filename
        sudo $NVPROF -f --export-profile "$dump_dir/$run_name".nvvp ../grid >> $filename
        $NVPROF --import-profile "$dump_dir/$run_name".nvvp >> $filename
    done
    echo >> $filename
    i=$(($i+$inc))
    conf_count=$(($conf_count+1))
done

echo "Done profiling GPU. Results in " $filename
