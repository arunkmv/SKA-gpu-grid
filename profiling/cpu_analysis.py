#!/usr/bin/env python3

import numpy as np
from scipy.optimize import curve_fit

def read_prof_output(filename):
    prof_file = open(filename, "r")
    grans = []
    times = []

    for line in prof_file:
        vals = line[:-1].split()
        num_times = len(vals) - 1

        granularity = vals[0]
        exe_time = 0
        for time in vals[1:]:
            exe_time += float(time)
        avg_exe_time = exe_time/num_times

        grans.append(granularity)
        times.append(avg_exe_time)

    prof_file.close()
    return np.array(grans), np.array(times)

def power_law(x, a, b):
    return a*np.power(x, b)

if __name__ == '__main__':
    grans, times = read_prof_output("prof_cpu.txt")
    params, cov = curve_fit(f=power_law, xdata=grans,
                            ydata=times, p0=[0, 0], bounds=(-np.inf, np.inf))

    print("Computational index: %f ns/B" % (params[0] * 1e+9))
    print("Power factor: %f" % params[1])
