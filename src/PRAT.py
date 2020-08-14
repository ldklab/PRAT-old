#!/usr/bin/env python3

import argparse
import subprocess
import sys

# Generate coverage files for Mosquitto.

def makeMosquitto(path, feature, flag):
    print("[+] Running in: {}".format(path))
    target = "WITH_" + feature + "=" + flag
    p = subprocess.Popen(["make", "binary", "-j", target], cwd=path)
    p.wait()

    # Generate gcov files.
    src = path + "/src"
    lib = path + "/lib"
    p = subprocess.Popen(["llvm-cov", "gcov", "*"], cwd=src)
    p.wait()
    p = subprocess.Popen(["llvm-cov", "gcov", "*"], cwd=lib)
    p.wait()

    # Make directories for storing the results.
    coverageFiles = "coverage_files_" + feature + "_" + flag
    p = subprocess.Popen(["mkdir", "-p", coverageFiles], cwd=path)
    p.wait()
    p = subprocess.Popen(["mv", src + "/*.gcov", coverageFiles], cwd=path)
    p.wait()
    p = subprocess.Popen(["mv", lib + "/*.gcov", coverageFiles], cwd=path)
    p.wait()

def makeFFmpeg(path, feature):
    print("[-] TODO: makeFFmpeg.")

def makeDDS(path, feature):
    print("[-] TODO: makeDDS.")

def makeCM(path, feature):
    print("[-] TODO: makeCM.")

if __name__ == '__main__':
    # Setup the command line args for different projects.
    parser = argparse.ArgumentParser()
    parser.add_argument("project", help="Directory to project to target")
    parser.add_argument("feature", help="Feature to identify/remove from project")
    args = parser.parse_args()

    #print("[+] Targeting project in: {} and feature: {}".format(args.project, args.feature))

    if "mosquitto" in args.project:
        # Compile with feature enabled.
        makeMosquitto(args.project, args.feature, "yes")
        # Compile with feature disabled.
        makeMosquitto(args.project, args.feature, "no")
    elif "FFmpeg" in args.project:
        makeFFmpeg(args.project, args.feature)
    else:
        print("[-] Target currently unsupported!")

    sys.exit(0)