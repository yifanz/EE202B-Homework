import serial
import struct
from numpy import mean
from numpy import var
from scipy.stats import skew
from scipy.stats import kurtosis

#floatlist = [random.random() for _ in range(10**5)]
floatlist = [1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1]
#floatlist = [1,2,3,4,5,6,7,8,9,10]
#floatlist = [1,1,1,1,1,1,1,1,1,1]
buf = struct.pack('%sf' % len(floatlist), *floatlist)

print "Expected:"
print "Mean     %s" % mean(floatlist);
print "Variance %s" % var(floatlist);
print "Skewness %s" % skew(floatlist);
print "Kurtosis %s" % kurtosis(floatlist);
print ""

with serial.Serial('/dev/ttyACM0', 115200, timeout=1) as ser:
    ser.write(buf)
    print struct.unpack('%sf' % 5, ser.read(20))
    ser.close()
