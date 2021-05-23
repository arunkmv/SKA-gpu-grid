#!/usr/bin/env python3
#
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import argparse
from itertools import cycle

def avg(lst):
    return sum(lst)/len(lst)

def read_prof_file(filename, numtimes):
    file = open(filename, "r")
    grans = []
    times = []
    gflops = []
    latency = []
    overhead = []

    run_gran = 0
    tmp_memcpy = 0
    tmp_times = [None] * numtimes
    tmp_gflops = [None] * numtimes
    tmp_latency = [None] * numtimes
    tmp_overhead = [None] * numtimes

    field_positions = {
        "grans": ("data to be processed:", 4),
        "memcpy": ("memcpy time:", 2),
        "times": ("Processed", 5),
        "gflops": ("Gflops", 0),
        "latency": ("[CUDA memcpy HtoD]", 1)
    }

    states = cycle(field_positions.keys())
    runs = cycle(range(numtimes))

    read_state = next(states)
    cur_run = next(runs)
    for line in file:
        substr, pos = field_positions[read_state]
        if (line.find(substr) != -1):
            vals = line[:-1].split()
            if (read_state == "grans"):
                if(cur_run == 0):
                    run_gran = float(vals[pos])
                elif (run_gran != float(vals[pos])):
                    print("Granularities of subsequent runs don't match... Exiting")
                    exit(1)
            elif (read_state == "memcpy"):
                tmp_memcpy = float(vals[pos])
            elif (read_state == "times"):
                tmp_times[cur_run] = float(vals[pos])
            elif (read_state == "gflops"):
                tmp_gflops[cur_run] = float(vals[pos])
            elif (read_state == "latency"):
                tmp_latency[cur_run] = float(vals[pos][:-2])   # Get rid of that pesky 'ms' TODO Check if unit is always ms
                tmp_overhead[cur_run] = tmp_memcpy - tmp_latency[cur_run]
                if (cur_run == numtimes - 1):
                    grans.append(run_gran)
                    times.append(avg(tmp_times))
                    gflops.append(avg(tmp_gflops))
                    latency.append(avg(tmp_latency))
                    overhead.append(avg(tmp_overhead))
                cur_run = next(runs)
            read_state = next(states)

    if (cur_run != 0):
       print("Total runs is not a multiple of number of subsequent runs... Exiting")
       exit(1)

    file.close()
    return (np.array(grans).astype(np.float64),
            np.array(times).astype(np.float64),
            np.array(gflops).astype(np.float64),
            np.array(latency).astype(np.float64),
            np.array(overhead).astype(np.float64))

def plot():
    pass

def power_law(x, a, b):
    return a*np.power(x, b)

def main(args):
    # Read input file
    grans, times, gflops, latency, overhead = read_prof_file(args.file, args.numtimes)
    print("Calculated parameters:")
    # TODO Calculate latency per byte and overhead

    if(args.plot): plot(grans, times, gflops, latency, overhead)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Analyser for the GPU gridding profile data')
    parser.add_argument('-f', '--file', action='store', default='prof_gpu.txt', help='Input profiling data file')
    parser.add_argument('-p', '--plot', action='store_true', help='To plot graphs')
    parser.add_argument('-n', '--numtimes', action='store', default=5, type=int, help='Number of subsequent runs with same configuration')
    args = parser.parse_args()
    main(args)
