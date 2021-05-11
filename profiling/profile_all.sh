#!/bin/bash

configs=( default config_1 config_2 config_3 config_4 config_5 )


for config in "${configs[@]}"
do
    cd ..
    bash build.sh $config
    cd -
    bash profile.sh $config 2>&1 | tee ${config}.txt
done

echo "Done profiling all GPU input configurations"
