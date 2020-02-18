#!/usr/bin/env python
from matplotlib import colors
import matplotlib.pyplot as plt
import numpy as np
import h5py, argparse, os, subprocess

def open_file(filename):
    f = h5py.File(filename, 'r')
    dataset = f['efu_hits']
    return dataset

def get_data(dataset, plane, args):
    x = []
    y = []
    col = []
    curr = 0
    count = 0
    for t, c, w, p in dataset:
        if curr > args.s:
            if p == plane:
                y.append(t)
                x.append(c)
                col.append(w)
                count = count + 1
        if args.n != 0 and count >= args.n:
            break
        curr = curr + 1
    return [x, y, col]


# Plots Hits for a given plane as (coord, t) scatter plot
def plot_plane(ax, plane, ymin, args):
    x = plane[0]
    y = plane[1]
    col = plane[2]
    vmin = np.amin(col)
    vmax = np.amax(col)
    cmap = plt.cm.jet
    norm = colors.Normalize(vmin=vmin, vmax=vmax)
    if args.z:
        s = (col - vmin) * 0.2 + 20
    else:
        s = 20

    ax.scatter(x, y - ymin, c = col, cmap = cmap, norm = norm, s = s)


# Setup side by side plots for Hits
def plot_data(planes, args, fname):
    filename = args.f
    plt.style.use('seaborn-whitegrid')
    fig, axs = plt.subplots(1,2, figsize=(10, 10))
    fig.suptitle("{} - {} hits offset {}".format(filename, args.n, args.s), fontsize=14)

    plane0 = planes[0]
    plane1 = planes[1]

    ymin = min(np.amin(plane0[1]), np.amin(plane1[1]))
    ymax = max(np.amax(plane0[1]), np.amax(plane1[1]))

    for plane in [0, 1]:
        axs[plane].set_title('Plane {}'.format(plane))
        axs[plane].set_ylabel('time (ns)')
        axs[plane].set_ylim(ymin=0, ymax=ymax - ymin)
        axs[plane].set_xlabel('coordinate')
        plot_plane(axs[plane], planes[plane], ymin, args)

    if args.i:
        plt.savefig(fname)
    else:
        plt.show()
    plt.close()

#
# #
#
def main():
    # Handle command line options
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", help="Filename", type=str, default="")
    parser.add_argument("-n", help="Number of Hits to read", type=int, default=0)
    parser.add_argument("-s", help="Skip samples", type=int, default=0)
    parser.add_argument('-z', help="Fixed symbol size", dest='z', default=True, action='store_false')
    parser.add_argument('-i', help="Save image to file", dest='i', default=False, action='store_true')
    parser.add_argument("-m", help="Number of images to make", type=int, default=1)
    args = parser.parse_args()

    # Read the data and plot
    files = []
    skip = args.s
    dataset = open_file(args.f)
    for i in range(args.m):
        args.s = i * skip
        fname = '_tmp%03d.png' % i
        print('Saving frame', fname)
        p0 = get_data(dataset, 0, args)
        p1 = get_data(dataset, 1, args)
        plot_data([p0, p1], args, fname)
        files.append(fname)

    if args.m > 1:
        print('Making movie animation.mpg - this may take a while')
        subprocess.call("mencoder 'mf://_tmp*.png' -mf type=png:fps=3 -ovc lavc "
                        "-lavcopts vcodec=wmv2 -oac copy -o animation.mpg", shell=True)
    if args.i:
        for fname in files:
            os.remove(fname)

if __name__ == '__main__':
    main()
