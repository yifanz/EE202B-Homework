import serial
import struct
from numpy import mean
from numpy import median
from numpy import var
from scipy.stats import skew
from scipy.stats import kurtosis

#floatlist = [random.random() for _ in range(10**5)]
floatlist = [1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1]
#floatlist = [1,2,3,4,5,6,7,8,9,10]
#floatlist = [1,1,1,1,1,1,1,1,1,1]
buf = struct.pack('%sf' % len(floatlist), *floatlist)

print "Expected:"
print "Min      %s" % min(floatlist)
print "Max      %s" % max(floatlist)
print "Mean     %s" % mean(floatlist)
print "Variance %s" % var(floatlist)
print "Skewness %s" % skew(floatlist)
print "Kurtosis %s" % kurtosis(floatlist)
print "Median   %s" % median(floatlist)
print ""

with serial.Serial('/dev/ttyACM0', 115200, timeout=1) as ser:
    ser.write(buf)
    nresults = 7
    results = struct.unpack('%sf' % nresults, ser.read(nresults * 4))

    print "Results:"
    print "Min      %s" % results[0]
    print "Max      %s" % results[1]
    print "Mean     %s" % results[2]
    print "Variance %s" % results[3]
    print "Skewness %s" % results[4]
    print "Kurtosis %s" % results[5]
    print "Median   %s" % results[6]
    print ""

    ser.close()
