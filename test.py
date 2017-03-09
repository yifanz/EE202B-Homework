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

#floatlist = [random.random() for _ in range(10**5)]
#floatlist = [1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1]
#floatlist = [1,2,3,4,5,6,7,8,9,10]
#floatlist = [1,1,1,1,1,1,1,1,1,1]
x = [1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1]
y = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
z = [random.random() for _ in range(10)]
xyz = list(sum(zip(x, y, z), ()))
buf = struct.pack('%sf' % len(xyz), *xyz)

def print_stats_channel(channel_name, floatlist):
	print "%s Min      %s" % (channel_name, min(floatlist))
	print "%s Max      %s" % (channel_name, max(floatlist))
	print "%s Mean     %s" % (channel_name, mean(floatlist))
	print "%s Variance %s" % (channel_name, var(floatlist))
	print "%s Skewness %s" % (channel_name, skew(floatlist))
	print "%s Kurtosis %s" % (channel_name, kurtosis(floatlist))
	print ""

print "Expected:"
print_stats_channel("X", x)
print_stats_channel("Y", y)
print_stats_channel("Z", z)
print "Correlation XY %s" % pearsonr(x,y)[0]
print "Correlation XZ %s" % pearsonr(x,z)[0]
print "Correlation YZ %s" % pearsonr(y,z)[0]
print ""
print "X Median %s" % median(x)
print "Y Median %s" % median(y)
print "Z Median %s" % median(z)
print ""

with serial.Serial(serial_dev, 115200, timeout=1) as ser:
	ser.write(buf)
   	nresults = 21
        res = ser.read(nresults * 4)
	results = struct.unpack('%sf' % nresults, res)

	print "Results:"
	print "X Min      %s" % results[0]
    	print "X Max      %s" % results[1]
    	print "X Mean     %s" % results[2]
    	print "X Variance %s" % results[3]
    	print "X Skewness %s" % results[4]
    	print "X Kurtosis %s" % results[5]
    	print ""
	print "Y Min      %s" % results[6]
    	print "Y Max      %s" % results[7]
    	print "Y Mean     %s" % results[8]
    	print "Y Variance %s" % results[9]
    	print "Y Skewness %s" % results[10]
    	print "Y Kurtosis %s" % results[11]
    	print ""
	print "Z Min      %s" % results[12]
    	print "Z Max      %s" % results[13]
    	print "Z Mean     %s" % results[14]
    	print "Z Variance %s" % results[15]
    	print "Z Skewness %s" % results[16]
    	print "Z Kurtosis %s" % results[17]
    	print ""
    	print "Correlation XY %s" % results[18]
    	print "Correlation XZ %s" % results[19]
    	print "Correlation YZ %s" % results[20]
        print ""
        #print "X Median %s" % results[21]
        #print "Y Median %s" % results[22]
        #print "Z Median %s" % results[23]

    	ser.close()
