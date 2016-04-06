#!/usr/bin/env python

import os
import argparse
import subprocess
# TODO: from here

argsParser = argparse.ArgumentParser(description="Script to run doxygen")
argsParser.add_argument("--hasdot", action='store_true',
        default=False, help="Indicates that 'dot' is available")
argsParser.add_argument("OUTPUT", help="Output directory")
argsParser.add_argument("SRCFILE", nargs='+',
        help="Source/include files from which to build doxygen documentation")
args = argsParser.parse_args()

# Derive a doxyfile with the right options
indoxy = "Doxyfile"
outdoxy = "Doxyfile.tmp"
infile = open(indoxy, 'r')
outfile = open(outdoxy, 'w')
for line in infile:
    tokens = line.strip().split()
    if len(tokens) > 0:
        if tokens[0] == "OUTPUT_DIRECTORY":
            outfile.write("{} = {}\n".format(tokens[0], args.OUTPUT))
        elif tokens[0] == "INPUT":
            tmp = " ".join(args.SRCFILE)
            outfile.write("INPUT = {}\n".format(tmp))
        elif tokens[0] == "HAVE_DOT" and args.hasdot:
            outfile.write("HAVE_DOT = YES\n")
        else:
            outfile.write(line)
    else:
        outfile.write(line)
infile.close()
outfile.close()

ret = 0
try:
    subprocess.check_call(["doxygen", outdoxy])
except:
    ret = 1

os.unlink(outdoxy)
exit(ret)
