#!/usr/bin/env python3
import numpy as np
import matplotlib.pyplot as plt
import csv
import sys

# Usage: python performance_profile.py benchmark.csv grasp_extra_tabu.pdf

def read_benchmark_csv(filename):
    with open(filename, 'r') as f:
        reader = csv.reader(f)
        header = next(reader)
        ncols = int(header[0])
        method_names = header[1:]
        data = []
        for row in reader:
            if not row or len(row) < ncols + 1:
                continue
            costs = [float(x) for x in row[1:ncols+1]]
            data.append(costs)
    return np.array(data), method_names

def performance_profile(data, method_names, maxratio=1.5, output='performance_profile.pdf', logplot=False, title=None):
    n_instances, n_methods = data.shape
    minima = data.min(axis=1)
    ratios = data / minima[:, None]
    ratios = np.sort(ratios, axis=0)
    y = np.arange(n_instances) / n_instances
    dashes = ['-', '--', '-.', ':', '-', '--']
    markers = ['+', 'x', 's', '^', 'o', 'd']
    colors = ['r', 'b', 'y', 'g', 'm', 'c']
    plt.figure(figsize=(8, 6))
    for j in range(n_methods):
        options = dict(label=method_names[j],
                       linewidth=2, drawstyle='steps-post', linestyle=dashes[j % len(dashes)],
                       marker=markers[j % len(markers)], markeredgewidth=2, markersize=9, color=colors[j % len(colors)])
        if logplot:
            plt.semilogx(ratios[:, j], y, **options)
        else:
            plt.plot(ratios[:, j], y, **options)
    plt.axis([1, maxratio, 0, 1])
    plt.xlabel('Performance Ratio', fontsize=14)
    plt.ylabel('Proportion of Instances', fontsize=14)
    plt.legend(loc='lower right', fontsize=12)
    if title:
        plt.title(title, fontsize=15)
    else:
        plt.title('Performance Profile', fontsize=15)
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.savefig(output)
    print(f"Performance profile saved to {output}")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f"Usage: python {sys.argv[0]} benchmark.csv output.pdf [--log]")
        sys.exit(1)
    logplot = '--log' in sys.argv
    data, method_names = read_benchmark_csv(sys.argv[1])
    performance_profile(data, method_names, output=sys.argv[2], logplot=logplot)
