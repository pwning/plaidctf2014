#!/usr/bin/python
import sys
import beanstalkc
TIME_TO_RUN = 240
if len(sys.argv) == 2:
    beanstalk = beanstalkc.Connection(host='54.86.24.189', port=11300)
    beanstalk.put(str(int(sys.argv[1])), ttr=TIME_TO_RUN)

