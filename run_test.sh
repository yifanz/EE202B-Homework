#!/bin/bash

# Example usage

# Run with random generated samples
#sudo python test.py '/dev/ttyACM0' 1000

# Run with test data
#sudo python test.py '/dev/ttyACM0' debug_100_KYUVIyu.txt

SERIAL_DEV='/dev/ttyACM0'

if [ $# -eq 1 ]; then
    sudo python test.py "$SERIAL_DEV" "$1"
elif [ $# -eq 2 ]; then
    sudo python test.py "$1" "$2"
else
    echo 'Example usage: ./run_test.sh "/dev/ttyACM0" debug_100_KYUVIyu.txt'
fi
