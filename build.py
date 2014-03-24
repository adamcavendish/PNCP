#!/usr/bin/env python

import sys
import os
import shutil
import glob

omnetpp_root = '/opt/omnetpp-4.3.1/'
omnetpp_bin = omnetpp_root + '/bin/'
opp_makemake = omnetpp_bin + 'opp_makemake'
opp_msgc = omnetpp_bin + 'opp_msgc'

# change to ./build/ directory and generate messages
print("--------------------opp_msgc--------------------")
cwd = os.getcwd()
os.chdir(cwd + '/build/')
msglist = glob.glob('../msg/*.msg')
[os.system(opp_msgc + ' -h ' + i) for i in msglist]
os.chdir(cwd)

print("--------------------opp_makemake--------------------")
ret = os.system(opp_makemake + ' -f --deep -I./include/')
if ret != 0:
    exit(ret)

print("--------------------make -j3--------------------")
ret = os.system('make -j3')
if ret != 0:
    exit(ret)

print("--------------------make MODE=release -j3--------------------")
ret = os.system('make MODE=release -j3')
if ret != 0:
    exit(ret)

print("--------------------clean up--------------------")
headerlist = glob.glob('msg/*.h')
cclist = glob.glob('msg/*.cc')
[os.remove(i) for i in headerlist]
[os.remove(i) for i in cclist]

