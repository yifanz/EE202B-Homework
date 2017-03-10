import sys
import serial
import struct
import random
import time
from numpy import mean
from numpy import median
from numpy import var
from scipy.stats import skew
from scipy.stats import kurtosis
from scipy.stats import pearsonr

serial_dev = sys.argv[1]
num_samples = int(sys.argv[2])

test_file_100 = "debug_100_KYUVIyu.txt"
test_file_500 = "public_500_3PmqMUv.txt"

def generate_answer(x_chan, y_chan, z_chan):
    window_size = 100
    win_x = []
    win_y = []
    win_z = []
    answer = []

    for x, y, z in zip(x_chan, y_chan, z_chan):
        win_x.append(x)
        win_y.append(y)
        win_z.append(z)

        if len(win_x) == window_size:
            answer.append(min(win_x))
            answer.append(max(win_x))
            answer.append(mean(win_x))
            answer.append(var(win_x))
            answer.append(skew(win_x))
            answer.append(kurtosis(win_x, fisher=False))
            answer.append(min(win_y))
            answer.append(max(win_y))
            answer.append(mean(win_y))
            answer.append(var(win_y))
            answer.append(skew(win_y))
            answer.append(kurtosis(win_y, fisher=False))
            answer.append(min(win_z))
            answer.append(max(win_z))
            answer.append(mean(win_z))
            answer.append(var(win_z))
            answer.append(skew(win_z))
            answer.append(kurtosis(win_z, fisher=False))
            answer.append(pearsonr(win_x,win_y)[0])
            answer.append(pearsonr(win_x,win_z)[0])
            answer.append(pearsonr(win_y,win_z)[0])
            win_x = []
            win_y = []
            win_z = []

    answer.append(median(x_chan))
    answer.append(median(y_chan))
    answer.append(median(z_chan))

    return answer


def generate_samples(n=0, fname=None):
    x_chan = []
    y_chan = []
    z_chan = []

    if fname != None:
        f = open(fname)
        for line in f:
            data = line.split(" ")
            x_chan.append(float(data[0]))
            y_chan.append(float(data[1]))
            z_chan.append(float(data[2]))
        f.close()
        answer = generate_answer(x_chan[:-1], y_chan[:-1], z_chan[:-1])
        xyz = list(sum(zip(x_chan, y_chan, z_chan), ()))
    else:
        x_chan = [m + 0.1 for m in range(n)]
        y_chan = [m for m in range(n)]
        z_chan = [random.random() for _ in range(n)]

        #x_chan = [random.random() for _ in range(n)]
        #y_chan = [random.random() for _ in range(n)]
        #z_chan = [random.random() for _ in range(n)]

        #x_chan = [m for m in range(n)]
        #y_chan = [m for m in range(n)]
        #z_chan = [m for m in range(n)]
        answer = generate_answer(x_chan, y_chan, z_chan)
        xyz = list(sum(zip(x_chan, y_chan, z_chan), ()))
        xyz.extend([0,0,0])

    return (xyz, answer)

def isclose(a, b, rel_tol=1e-09, abs_tol=0.0):
    return abs(a-b) <= max(rel_tol * max(abs(a), abs(b)), abs_tol)

#xyz, answer = generate_samples(fname=test_file_100)
#xyz, answer = generate_samples(fname=test_file_500)
#xyz, answer = generate_samples(n=500)
buf = struct.pack('%sf' % len(xyz), *xyz)

with serial.Serial(serial_dev, 115200, timeout=1) as ser:
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    ser.write(buf)
    rec = ser.read(len(answer) * 4)
    print "Received %s bytes" % len(rec)
    results = list(struct.unpack('%sf' % (len(rec) / 4), rec))

    num_err = 0

    for a, b in zip(answer, results):
        diff = a-b
        ok = isclose(a, b, abs_tol=1e-3)
        if not ok:
            num_err += 1
        print '{0:> 15e} {1:> 15e} {2:> 15e} {3:>8}'\
                .format(a, b, diff, str(ok))

    print ""
    print "Received %s/%s bytes, Errors: %s" \
            % (len(rec), len(answer) * 4, num_err)

    ser.reset_input_buffer()
    ser.reset_output_buffer()
    ser.close()
