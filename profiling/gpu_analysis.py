#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import argparse
from itertools import cycle
import re

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
    conf_count = 0
    for line in file:
        if(re.match("^Error", line) != None):
            print("Error detected in profile output at {:d}_{:d}".format(conf_count, cur_run+1))
            exit(1)
        substr, pos = field_positions[read_state]
        if (line.find(substr) != -1):
            vals = line[:-1].split()
            if (read_state == "grans"):
                if(cur_run == 0):
                    run_gran = float(vals[pos])
                elif (run_gran != float(vals[pos])):
                    print("Granularities of subsequent runs don't match (at {:d}_{:d})... Exiting".format(conf_count, cur_run+1))
                    exit(1)
            elif (read_state == "memcpy"):
                tmp_memcpy = float(vals[pos])
            elif (read_state == "times"):
                tmp_times[cur_run] = float(vals[pos])
            elif (read_state == "gflops"):
                tmp_gflops[cur_run] = float(vals[pos])
            elif (read_state == "latency"):
                if (vals[pos][-2:] == "us"):
                    tmp_latency[cur_run] = float(vals[pos][:-2]) * 1e+3
                elif (vals[pos][-2:] == "ms"):
                    tmp_latency[cur_run] = float(vals[pos][:-2]) * 1e+6
                else:
                    print("Unknown latency unit at {:d}_{:d}".format(conf_count, cur_run+1))
                    exit(1)

                tmp_overhead[cur_run] = (tmp_memcpy * 1e+6) - tmp_latency[cur_run]
                if (cur_run == numtimes - 1):
                    grans.append(run_gran)
                    times.append(avg(tmp_times))
                    gflops.append(avg(tmp_gflops))
                    latency.append(avg(tmp_latency))
                    overhead.append(avg(tmp_overhead))
                    conf_count += 1
                cur_run = next(runs)
            read_state = next(states)

    if (cur_run != 0):
       print("Total runs is not a multiple of number of subsequent runs... Exiting")
       exit(1)

    print("{:d} total configurations detected".format(conf_count))
    file.close()
    return (np.array(grans).astype(np.float64),
            np.array(times).astype(np.float64),
            np.array(gflops).astype(np.float64),
            np.array(latency).astype(np.float64),
            np.array(overhead).astype(np.float64))

def plot(grans, times, gflops, latency, overhead, latency_per_byte):
    fig, axs = plt.subplots(2, 2, sharex=True)
    fig.tight_layout()

    axs[0, 0].scatter(grans, times, color='#00b3b3')
    axs[0, 0].set(ylabel='Time (ms)')
    axs[0, 0].set_title('Execution time')

    axs[0, 1].scatter(grans, gflops, color='#ff0000')
    axs[0, 1].set(ylabel='GFLOPS')
    axs[0, 1].set_title('GFLOPS')

    axs[1, 0].scatter(grans, overhead, color='#3bd80e')
    axs[0, 1].set(ylabel='Time (ns)')
    axs[1, 0].set_title("Overhead, $avg = {:.5f} ns$".format(avg(overhead)))

    axs[1, 1].scatter(grans, latency, color='#f38d12')
    axs[1, 1].set(ylabel='Time (ns)')
    axs[1, 1].set_title("Latency, $avg/B = {:.5f} ps/B$".format(avg(latency_per_byte)))

    plt.xlabel("Processed data (Bytes)")
    plt.show()

def main(args):
    # Read input file
    grans, times, gflops, latency, overhead = read_prof_file(args.file, args.numtimes)

    conv = np.vectorize((lambda t,f: t*(10**f)))
    latency_per_byte = np.divide(conv(latency, 3), grans)

    print("Calculated parameters:")
    print("Average latency per byte: {:.5f} ps/B".format(avg(latency_per_byte)))
    print("Average overhead: {:.5f} ns". format(avg(overhead)))

    if(args.plot): plot(grans, times, gflops, latency, overhead, latency_per_byte)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Analyser for the GPU gridding profile data')
    parser.add_argument('-f', '--file', action='store', default='prof_gpu.txt', help='Input profiling data file')
    parser.add_argument('-p', '--plot', action='store_true', help='To plot graphs')
    parser.add_argument('-n', '--numtimes', action='store', default=5, type=int, help='Number of subsequent runs with same configuration')
    args = parser.parse_args()
    main(args)
