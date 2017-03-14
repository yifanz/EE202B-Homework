import sys
import pdb
from math import sqrt
from numpy import median
from numpy import var
from scipy.stats import skew
from scipy.stats import kurtosis
from scipy.stats import pearsonr

def mean(A):
    s = 0
    n = len(A)
    for a in A:
        s = s + a
    m = s / n
    return m


def stddev(A):
    mean_A = mean(A);
    s = 0
    for a in A:
        d = abs(a - mean_A)
        s += d * d
    sdev = sqrt(s / len(A))
    return sdev


def covariance(A, B):
    mean_A = mean(A)
    mean_B = mean(B)
    s = 0
    for x, y in zip(A, B):
        s += (x - mean_A) * (y - mean_B)
    covar = s / len(A)
    return covar


def correlation(a, b):
    corr = covariance(a, b) / (stddev(a) * stddev(b))
    return corr


def calculate_stats(x_chan, y_chan, z_chan):
    sample_interval = 100
    win_x = []
    win_y = []
    win_z = []
    answer = []

    for x, y, z in zip(x_chan, y_chan, z_chan):
        win_x.append(x)
        win_y.append(y)
        win_z.append(z)

        # every 100 samples, calculate stats
        if len(win_x) > 0 and len(win_x) % sample_interval == 0:
            answer.append(min(win_x))
            answer.append(max(win_x))
            answer.append(mean(win_x))
            answer.append(var(win_x))
            answer.append(skew(win_x, bias=False))
            answer.append(kurtosis(win_x, fisher=False))
            answer.append(min(win_y))
            answer.append(max(win_y))
            answer.append(mean(win_y))
            answer.append(var(win_y))
            answer.append(skew(win_y, bias=False))
            answer.append(kurtosis(win_y, fisher=False))
            answer.append(min(win_z))
            answer.append(max(win_z))
            answer.append(mean(win_z))
            answer.append(var(win_z))
            answer.append(skew(win_z, bias=False))
            answer.append(kurtosis(win_z, fisher=False))
            answer.append(correlation(win_x, win_y))
            answer.append(correlation(win_x, win_z))
            answer.append(correlation(win_y, win_z))
            #answer.append(pearsonr(win_x,win_y)[0])
            #answer.append(pearsonr(win_x,win_z)[0])
            #answer.append(pearsonr(win_y,win_z)[0])

    # report median at the end only
    answer.append(median(x_chan))
    answer.append(median(y_chan))
    answer.append(median(z_chan))

    return answer


def check_samples(fname=None):
    x_chan = []
    y_chan = []
    z_chan = []

    with open(fname) as f:
        num_samples = int(f.readline().strip())
        for i in range(num_samples):
            x, y, z = tuple(map(float, f.readline().strip().split(' ')))
            x_chan.append(x)
            y_chan.append(y)
            z_chan.append(z)

        # ignore the last (0,0,0)
        answer = calculate_stats(x_chan[:-1], y_chan[:-1], z_chan[:-1])

        return answer

test_input_file = sys.argv[1]

answer = check_samples(test_input_file)

for i, val in enumerate(answer):
    print('{0:< 5} {1:> 15e}'.format(i+1, val))
