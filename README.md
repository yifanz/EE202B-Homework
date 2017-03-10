# EE202B-Homework Energy Efficient Computing

To get started, you should install the `mbed-cli`, `numpy` and `scipy` python modules.
You also need to have an ARM toolchain. I'm using `GCC`, but other compilers should work too.
Configure your compiler toolchain in the `mbed_settings.py` file:

```
# mbed_settings.py
GCC_ARM_PATH = "~/gcc-arm-none-eabi-4_9-2015q2/bin"
```

Build the firmware:

```
logn@ubuntu:~/EE202B/homework$ ./build.sh 
Building project homework (NUCLEO_F746ZG, GCC_ARM)
Scan: .
Scan: FEATURE_BLE
Scan: FEATURE_UVISOR
Scan: FEATURE_LWIP
Scan: FEATURE_COMMON_PAL
Scan: FEATURE_STORAGE
Scan: FEATURE_LOWPAN_HOST
Scan: FEATURE_THREAD_BORDER_ROUTER
Scan: FEATURE_LOWPAN_ROUTER
Scan: FEATURE_THREAD_ROUTER
Scan: FEATURE_LOWPAN_BORDER_ROUTER
Scan: FEATURE_NANOSTACK
Scan: FEATURE_ETHERNET_HOST
Scan: FEATURE_NANOSTACK_FULL
Scan: FEATURE_THREAD_END_DEVICE
Scan: mbed
Scan: env
+-----------------------+-------+-------+-------+
| Module                | .text | .data |  .bss |
+-----------------------+-------+-------+-------+
| Fill                  |   171 |     7 |     9 |
| Misc                  | 60384 |  2257 | 15104 |
| drivers               |  2290 |     4 |   132 |
| features/FEATURE_LWIP |    44 |     0 | 12536 |
| hal                   |   360 |     0 |     8 |
| platform              |  1730 |     4 |   297 |
| rtos                  |   149 |     4 |     4 |
| rtos/rtx              |  6309 |    20 |  6870 |
| targets/TARGET_STM    |  9237 |     4 |  1152 |
| Subtotals             | 80674 |  2300 | 36112 |
+-----------------------+-------+-------+-------+
Allocated Heap: unknown
Allocated Stack: unknown
Total Static RAM memory (data + bss): 38412 bytes
Total RAM memory (data + bss + heap + stack): 38412 bytes
Total Flash memory (text + data + misc): 82974 bytes

Image: ./BUILD/NUCLEO_F746ZG/GCC_ARM/homework.bin
```

You can run tests on random data by specifying the number of samples: `./run_test.sh '/dev/ttyACM0' 100`.
Alternatively, you can run using an input file:

```
logn@ubuntu:~/EE202B/homework$ ./run_test.sh '/dev/ttyACM0' input_data/debug_100_KYUVIyu.txt 
Sent 1212/1212
Wait 0
   3.829800e-02    3.829800e-02    3.497601e-10         
   9.975231e+00    9.975231e+00   -1.706543e-07         
   4.940113e+00    4.940114e+00   -8.644641e-07         
   9.739988e+00    9.739989e+00   -1.426168e-06         
   3.689498e-02    3.689496e-02    1.825222e-08         
   1.628039e+00    1.628039e+00    5.043219e-07         
   2.276380e-01    2.276380e-01   -6.210327e-09         
   9.680403e+00    9.680403e+00    2.442627e-07         
   5.124520e+00    5.124519e+00    4.018555e-07         
   8.166231e+00    8.166232e+00   -1.503564e-06         
  -3.192221e-02   -3.192220e-02   -1.286369e-08         
   1.784675e+00    1.784675e+00    7.577149e-07         
   6.231900e-02    6.231900e-02    6.797314e-10         
   9.996001e+00    9.996001e+00   -2.435913e-07         
   5.480094e+00    5.480094e+00   -4.928308e-07         
   8.740883e+00    8.740884e+00   -7.862726e-07         
  -2.702209e-01   -2.702207e-01   -1.371474e-07         
   1.758540e+00    1.758539e+00    5.016217e-07         
   1.205637e-01    1.205637e-01    1.195888e-08         
   2.354575e-02    2.354575e-02    6.202456e-09         
   1.026963e-01    1.026963e-01    6.951818e-09         
   5.054524e+00    5.054523e+00    3.198242e-08         
   5.160806e+00    5.160807e+00   -1.558838e-07         
   6.257737e+00    6.257737e+00   -1.597290e-07         

Received 96/96 bytes, Errors: 0, Precision: -0.000000

Reminder: Don't forget to reset the board between test runs.
```

Full list of python dependencies:

```
adium-theme-ubuntu==0.3.4
beautifulsoup4==4.5.3
colorama==0.3.7
enum34==1.1.6
fasteners==0.14.1
funcsigs==1.0.2
future==0.16.0
fuzzywuzzy==0.15.0
intelhex==2.1
Jinja2==2.9.5
junit-xml==1.7
lockfile==0.12.2
MarkupSafe==0.23
mbed-cli==1.0.0
mbed-greentea==1.2.5
mbed-host-tests==1.1.7
mbed-ls==1.2.12
mercurial==3.7.3
mock==2.0.0
monotonic==1.2
numpy==1.12.0
pbr==2.0.0
prettytable==0.7.2
project-generator==0.9.10
project-generator-definitions==0.2.34
pyOCD==0.8.1a1
pyserial==3.2.1
Pyste==0.9.10
pyusb==1.0.0
PyYAML==3.12
requests==2.13.0
scipy==0.18.1
six==1.10.0
unity-lens-photos==1.0
websocket-client==0.40.0
xmltodict==0.10.2
```
