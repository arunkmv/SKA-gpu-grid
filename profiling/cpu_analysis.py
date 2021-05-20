#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import argparse

def read_prof_file(filename):
    file = open(filename, "r")
    grans = []
    times = []

    for line in file:
        if (line[0] != '#'):
            vals = line[:-1].split()
            num_times = len(vals) - 1
            granularity = vals[0]
            sum_times = 0
            for time in vals[1:]:
                sum_times += float(time)
                avg_time = sum_times/num_times
            grans.append(granularity)
            times.append(avg_time)

    file.close()
    return np.array(grans).astype(np.float64), np.array(times).astype(np.float64)

def plot(grans, times, params):
    fig, (ax1, ax2) = plt.subplots(2, sharex=True)

    ax1.scatter(grans, times, color='#00b3b3', label='Observed')
    ax1.plot(grans, power_law(grans, *params), color='black', linestyle='--', linewidth=1.5, label='Fit')
    ax2.scatter(grans, times - power_law(grans, *params), color='#ff0000')
    ax2.grid(axis='y', linestyle='--', linewidth=0.75)

    ax1.set_title("$C = {:.3f} ns/B \;\;\; \\beta = {:.3f}$".format(params[0] * 1e+9, params[1]))
    plt.xlabel('Processed data (Bytes)')
    plt.ylabel('Time (s)')
    ax2.set_title('Deviations')
    fig.tight_layout()
    ax1.legend(loc="lower right")
    plt.show()

def power_law(x, a, b):
    return a*np.power(x, b)

def main(args):
    # Read input file
    grans, times = read_prof_file(args.file)

    # Perform power regression curve fitting
    params, cov = curve_fit(f=power_law, xdata=grans,
                            ydata=times, p0=[0, 0], bounds=(-np.inf, np.inf), maxfev=1000)

    print("Calculated parameters:")
    print("Computational index: %f ns/B" % (params[0] * 1e+9))
    print("Power factor: %f" % params[1])
    if(args.plot): plot(grans, times, params)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Analyser for the CPU gridding profile data')
    parser.add_argument('-f', '--file', action='store', default='prof_cpu.txt', help='Input profiling data file')
    parser.add_argument('-p', '--plot', action='store_true', help='To plot graphs')
    args = parser.parse_args()
    main(args)
