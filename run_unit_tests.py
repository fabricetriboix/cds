#!/usr/bin/env python

import os
import sys
import subprocess
import fnmatch


modes = ["debug", "release"]
mode = ""
path = ""
for m in modes:
    path = "build/x64-linux/" + m + "/cds_unit_tests"
    if os.access(path, os.R_OK):
        mode = m
        break

if mode == "":
    raise RuntimeError("Please compile cds unit tests for x64-linux; modes tried: {}".format(modes))

if mode == "debug":
    os.environ['MALLOC_TRACE'] = os.path.join(os.getcwd(), "mtrace.log")

# Run the cds unit tests and capture the output
output = subprocess.check_output([path])
f = open("cds.rtt", 'w')
f.write(output)
f.close()

# Find all the unit test source files
unitTests = []
for dirName, dirs, files in os.walk("."):
    if dirName != "." and dirName != "./build":
        for f in files:
            if fnmatch.fnmatch(f, "unittest*.c"):
                unitTests.append(dirName + "/" + f)

# Print the results
args = ["rttest2text.py", "cds.rtt"]
args.extend(unitTests)
ret = 0
if subprocess.call(args) != 0:
    print("Failed to run rttest2text.py")
    ret = 1
os.unlink("cds.rtt")

# Check for memory leaks
if mode == "debug":
    if not os.path.exists("mtrace.log"):
        print("ERROR: mtrace file not produced!")
    else:
        args = ["mtrace", path, "mtrace.log"]
        subprocess.call(args)

sys.exit(ret)
