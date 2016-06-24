#!/usr/bin/env python

import sys
import os
import argparse
import glob
import shutil
import autodetectplf


# Preliminaries

autoplf = autodetectplf.autodetectplf()

# Ensure we are in top-level directory
os.chdir(os.path.dirname(os.path.realpath(__file__)))


# Arguments

argsParser = argparse.ArgumentParser(description="Install script - "
        "All paths except `prefix` can be relative, in which case they will "
        "be relative to `prefix`")

argsParser.add_argument("--target", default=autoplf,
        help="Compilation target (auto-detected default: " + autoplf + ")")

argsParser.add_argument("--show", action='store_true', default=False,
        help="Show defaults directories for selected target; "
            "no action will be taken")

argsParser.add_argument("--prefix", default="",
        help="Base installation directory")

argsParser.add_argument("--bindir", default="",
        help="Where to install executables; "
            "can be absolute of relative (from `prefix`)")

argsParser.add_argument("--libdir", default="",
        help="Where to install libraries; "
            "can be absolute of relative (from `prefix`)")

argsParser.add_argument("--incdir", default="",
        help="Where to install header files; "
            "can be absolute of relative (from `prefix`)")

argsParser.add_argument("--docdir", default="",
        help="Where to install documentation; "
            "can be absolute of relative (from `prefix`)")

argsParser.add_argument("--etcdir", default="",
        help="Where to install configuration files; "
            "can be absolute of relative (from `prefix`)")

argsParser.add_argument("--datadir", default="",
        help="Where to install static data files; "
            "can be absolute of relative (from `prefix`)")

argsParser.add_argument("--vardir", default="",
        help="Where dynamic data files live; "
            "can be absolute of relative (from `prefix`)")

argsParser.add_argument("-i", "--ignore", default=False,
        action='store_true', help="Ignore errors when installing files and "
            "directories")

args = argsParser.parse_args()


# Import settings & establish installation paths

tgtplf = args.target
if not tgtplf:
    tgtplf = autoplf
    print("--target not specified, using auto-detected: " + tgtplf)

path = os.path.join("src", "plf", tgtplf)
if not os.path.isdir(path) or not os.access(path, os.R_OK):
    print("ERROR: Can't find platform: " + path)
    sys.exit(1)

sys.path.append(path)
import plfsettings

allsettings = plfsettings.GetPlfSettings(['release'])
settings = allsettings['release']

builddir = os.path.join("build", tgtplf, "release")

def EstablishInstallPath(settings, d, arg):
    prefix = settings['prefix']
    if arg:
        if os.path.isabs(arg):
            return arg
        return os.path.join(prefix, arg)
    if os.path.isabs(settings[d]):
        return settings[d]
    return os.path.join(prefix, settings[d])

if args.prefix:
    settings['prefix'] = args.prefix
if not os.path.isabs(settings['prefix']):
    print("ERROR: `prefix` must be an absolute path: " + settings['prefix'])
    sys.exit(1)
bindir = EstablishInstallPath(settings, 'bindir', args.bindir)
libdir = EstablishInstallPath(settings, 'libdir', args.libdir)
incdir = EstablishInstallPath(settings, 'incdir', args.incdir)
docdir = EstablishInstallPath(settings, 'docdir', args.docdir)
etcdir = EstablishInstallPath(settings, 'etcdir', args.etcdir)
shrdir = EstablishInstallPath(settings, 'shrdir', args.datadir)
vardir = EstablishInstallPath(settings, 'vardir', args.vardir)


### START CONFIG ###

# Executables to install
installExecutables = []

# Libraries to install
installLibs = [os.path.join(builddir, "libcds.a")]

# Include files to install
installIncs = glob.glob(os.path.join(builddir, "include", "*.h"))

# Documentation to install
installDoc = [os.path.join(builddir, "doc", "html")]

# Configuration files to install
installConfig = []

# Static data files to install
installData = []

# Dynamic data files to install
installVar = []

### END CONFIG ###


# Install stuff

def Install(installdir, path):
    """Install `path` to `installdir`. `path` could be a file or a directory.
    """
    if not os.path.exists(path):
        print("ERROR: File/directory to install not found: " + path)
        sys.exit(1)

    if os.path.exists(installdir):
        if not os.path.isdir(installdir):
            print("ERROR: Installation directory is not a directory: "
                    + installdir)
            sys.exit(1)
        if not os.access(installdir, os.W_OK):
            print("ERROR: Access denied to installation directory: "
                    + installdir)
            sys.exit(1)
    else:
        os.makedirs(installdir)

    basename = os.path.basename(path)
    if os.path.isfile(path):
        dst = os.path.join(installdir, basename)
        if os.path.exists(dst):
            if args.ignore:
                print("WARNING: File to install already exists: " + dst)
            else:
                print("ERROR: File to install already exists: " + dst)
                sys.exit(1)
        print("Installing file '{}' to {}".format(path, installdir))
        shutil.copy(path, installdir)

    elif os.path.isdir(path):
        print("Installing directory '{}' to {}".format(path, installdir))
        try:
            shutil.copytree(path, os.path.join(installdir, basename))
        except Exception, e:
            dst = os.path.join(installdir, basename)
            if args.ignore:
                print("WARNING: Failed to install directory {}: {}".format(
                        dst, e))
            else:
                print("ERROR: Failed to install directory {}: {}".format(
                        dst, e))
                sys.exit(1)

    else:
        print("ERROR: Thingy to install is neither a file nor a directory: "
                + path)
        sys.exit(1)

# Only show install directories if requested
if args.show:
    print("Target: " + tgtplf)
    print("prefix: " + settings['prefix'])
    print("bindir: " + bindir)
    print("libdir: " + libdir)
    print("incdir: " + incdir)
    print("docdir: " + docdir)
    print("etcdir: " + etcdir + " (configuration)")
    print("shrdir: " + shrdir + " (static data files)")
    print("vardir: " + vardir + " (dynamic data files)")
    sys.exit(0)

for i in installExecutables:
    Install(bindir, i)

for i in installLibs:
    Install(libdir, i)

for i in installIncs:
    Install(incdir, i)

for i in installDoc:
    Install(docdir, i)

for i in installConfig:
    Install(etcdir, i)

for i in installData:
    Install(shrdir, i)

for i in installVar:
    Install(vardir, i)
