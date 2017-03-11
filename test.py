import sys
import serial
import struct
import random
import time
import threading
from Queue import Queue
from numpy import mean
from numpy import median
from numpy import var
from scipy.stats import skew
from scipy.stats import kurtosis
from scipy.stats import pearsonr


USE_THREADING = True
#SEND_LIMIT = int(200)
SAMPLE_WIN = int(100)


def generate_answer(x_chan, y_chan, z_chan):
    window_size = SAMPLE_WIN
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
        with open(fname) as f:
            num_samples = int(f.readline().strip())
            input_bytes = b''
            for i in range(num_samples):
                x, y, z = tuple(map(float, f.readline().strip().split(' ')))
                x_chan.append(x)
                y_chan.append(y)
                z_chan.append(z)
                input_bytes += struct.pack('=fff', x, y, z)

            num_sending_events = int(f.readline().strip())
            sending_events = []
            for i in range(num_sending_events):
                terms = f.readline().strip().split(' ')
                num_bytes, waiting_time = int(terms[0]), float(terms[1])
                sending_events.append((num_bytes, waiting_time))
            #expected_received_bytes = (((num_samples // 100) * 21) + 3) * 4

            answer = generate_answer(x_chan[:-1], y_chan[:-1], z_chan[:-1])

            return (input_bytes, sending_events, answer)
    else:
        #x_chan = [m + 0.1 for m in range(n)]
        #y_chan = [m for m in range(n)]
        #z_chan = [random.random() for _ in range(n)]

        x_chan = [random.random() for _ in range(n)]
        y_chan = [random.random() for _ in range(n)]
        z_chan = [random.random() for _ in range(n)]

        #x_chan = [m for m in range(n)]
        #y_chan = [m for m in range(n)]
        #z_chan = [m for m in range(n)]

        answer = generate_answer(x_chan, y_chan, z_chan)
        xyz = list(sum(zip(x_chan, y_chan, z_chan), ()))
        xyz.extend([0,0,0])
        input_bytes = struct.pack('%sf' % len(xyz), *xyz)

        sending_events = None

        # TODO was tring to divide up random samples and send them
        # with delays in between.
        #n_samples = n + 1
        #if n_samples > SEND_LIMIT:
        #    num_sending_events = int(n_samples) // SEND_LIMIT
        #    remaining = int(n_samples) % SEND_LIMIT
        #    if remaining > 0:
        #        num_sending_events += 1
        #    sending_events = []
        #    for i in range(num_sending_events):
        #        num_bytes = SEND_LIMIT * 4 * 3
        #        waiting_time = 0
        #        sending_events.append((num_bytes, waiting_time))
        #    if remaining > 0:
        #        sending_events[-1] = (remaining * 4 * 3, 0)

        return (input_bytes, sending_events, answer)


def isint(s):
    try:
        int(s)
        return True
    except ValueError:
        return False


def isclose(a, b, rel_tol=1e-09, abs_tol=0.0):
    return abs(a-b) <= max(rel_tol * max(abs(a), abs(b)), abs_tol)


serial_dev = sys.argv[1]

if isint(sys.argv[2]):
    num_samples = int(sys.argv[2])
    input_bytes, sending_events, answer = generate_samples(n=num_samples)
else:
    test_input_file = sys.argv[2]
    input_bytes, sending_events, answer = generate_samples( \
            fname=test_input_file)

with serial.Serial(port=serial_dev, baudrate=115200, \
        timeout=1, write_timeout=None, \
        parity=serial.PARITY_NONE, bytesize=serial.EIGHTBITS, \
        stopbits=serial.STOPBITS_ONE) as ser:

    ser.reset_input_buffer()
    ser.reset_output_buffer()

    if USE_THREADING:
        connected = False
        def read_from_port(ser, num_bytes, q):
            output = bytes()
            while connected:
                output += ser.read(num_bytes)
            q.put(output)

        q = Queue()
        read_thread = threading.Thread(target=read_from_port, \
                args=(ser, len(answer) * 4, q))
        connected = True
        read_thread.start()

    ts_start = time.time()

    if sending_events != None:
        bidx = 0
        output_bytes = bytes()
        for chunk_size, wait_time in sending_events:
            for i in range(chunk_size):
                ser.write(input_bytes[bidx:bidx+1])
                bidx += 1
            print('Sent %d/%d' % (bidx, len(input_bytes)))
            print('Wait %d' % wait_time)
            time.sleep(wait_time)
    else:
        ser.write(input_bytes)

    if USE_THREADING:
        connected = False
        read_thread.join()
        output_bytes = q.get()
    else:
        # Warning, if you don't use threading, then you risk losing
        # some result data when the input size of large (>5000).
        # Probably due to the limited serial buffer size in Linux
        # or pyserial.
        output_bytes = ser.read(len(answer) * 4)

    ts_end = time.time()


    results = list( \
            struct.unpack('%sf' % (len(output_bytes) / 4), output_bytes))

    num_err = 0

    for a, b in zip(answer, results):
        diff = a-b
        ok = isclose(a, b, abs_tol=1e-3)
        if not ok:
            num_err += 1
        print '{0:> 15e} {1:> 15e} {2:> 15e} {3:>8}'\
                .format(a, b, diff, ' ' if ok else 'FAIL')

    print ""
    print "Received %d/%d bytes, Errors: %d, Elapsed: %f sec" \
            % (len(output_bytes), len(answer) * 4, num_err, \
            (ts_end - ts_start))

    print ""
    if len(output_bytes) == (len(answer) * 4) and num_err == 0:
        print "Test SUCCESSFUL"
    else:
        print "Test FAILED"
    print ""

    ser.reset_input_buffer()
    ser.reset_output_buffer()
