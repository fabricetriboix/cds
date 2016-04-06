#!/usr/bin/env python

import os
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
ret = 0
try:
    args = ["rttest2text.py", "cds.rtt"]
    args.extend(unitTests)
    subprocess.check_call(args)
except:
    print("Failed to run rttest2text.py")
    ret = 1
os.unlink("cds.rtt")
exit(ret)