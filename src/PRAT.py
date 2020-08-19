#!/usr/bin/env python3

import argparse
import subprocess
import sys, os
import shutil

def makeDiffs(path1, path2, feat):
    print("[+] Checking for matching files in {} and {}"
        .format(path1,  path2))
    
    aFiles = {os.path.splitext(x)[0] for x in os.listdir(path1)}
    bFiles = {os.path.splitext(x)[0] for x in os.listdir(path2)}

    outdir = "diff_" + feat
    p = subprocess.Popen(["mkdir", "-p", outdir])
    p.wait()

    for f in aFiles:
        # We only diff files that exist in both compilations.
        if f in bFiles:
            #print("[+] {}".format(f))
            # Make diffs of the file and save in another folder.
            target = f + ".gcov"
            out = open(outdir + "/" + target, 'w')
            p = subprocess.Popen(["diff", path1 + "/" + target, path2 + "/" + target], stdout=out)
            p.wait()
        else:
            # If an entire file is left out, we could posit that
            # it is only used when the tested feature ie enabled.
            print("[+] {} only exists with {} enabled".format(f, feat))
    
    # Clean up empty files.
    for covFile in os.listdir(outdir):
        if not os.path.getsize(outdir + "/" + covFile):
            #print("[-] {} is empty. Deleting".format(covFile))
            os.remove(outdir + "/" + covFile)
    
    return outdir

def extractFeatures(path):
    print("[+] Extract features for removal from: {}".format(path))
    p = subprocess.Popen(["perl", "extract_features.pl", path + "/"])
    p.wait()

    if isTool("xdot"):
        p = subprocess.Popen(["xdot", "FDG.dot"])
        p.wait()
    else:
        print("[-] `xdot` is not available. Saving to FDG.dot")

# Generate coverage files for Mosquitto.
def makeMosquitto(path, feature, flag, tests=None):
    print("[+] Running in: {}".format(path))
    target = "WITH_" + feature + "=" + flag
    p = subprocess.Popen(["make", "clean"], cwd=path)
    p.wait()
    p = subprocess.Popen(["make", "binary", "-j", target], cwd=path)
    p.wait()

    # Add part here later for running tests.
    if tests is not None:
        p = subprocess.Popen(["make", "test", "-j"], cwd=path)
        p.wait()

    # Generate gcov files.
    src = path + "/src"
    lib = path + "/lib"
    print("[+] Running in: {} and {}".format(src, lib))
    p = subprocess.Popen("llvm-cov-10 gcov *", shell=True, cwd=src)
    p.wait()
    #p = subprocess.Popen("llvm-cov-10", "gcov", "*", shell=True, cwd=lib)
    #p.wait()

    # Make directories for storing the results.
    coverageFiles = "coverage_files_WITH_" + feature + "_" + flag
    p = subprocess.Popen("mkdir -p " + coverageFiles, shell=True, cwd=path)
    p.wait()
    p = subprocess.Popen("mv " + src + "/*.gcov " + coverageFiles, shell=True, cwd=path)
    p.wait()
    #p = subprocess.Popen("mv " + lib + "/*.gcov " + coverageFiles, shell=True, cwd=path)
    #p.wait()

    # Move the files to working dir.
    home = os.getcwd()
    p = subprocess.Popen(["mv", coverageFiles, home], cwd=path)
    p.wait()

def makeFFmpeg(path, feature, flag, tests=None):
    print("[+] Running in: {}".format(path))

    # Prep the build system using configure.
    if flag == "yes":
        # No explicit 'enable' flag for FFmpeg.
        p = subprocess.Popen(["bash", "configure", "--toolchain=gcov"], cwd=path)
        p.wait()
    else:
        target = "--disable-" + feature
        p = subprocess.Popen(["bash", "configure", "--toolchain=gcov", target], cwd=path)
        p.wait()
    
    p = subprocess.Popen(["make", "clean"], cwd=path)
    p.wait()

    p = subprocess.Popen(["make", "-j3"], cwd=path)
    p.wait()

    # Add part later for running tests.
    if tests is not None:
        p = subprocess.Popen(["make", "fate", "-j", "SAMPLES=fate-suite/"], cwd=args.path)
        p.wait()

    p = subprocess.Popen(["./ffmpeg", "--help"], cwd=path)
    p.wait()

    # Then, generate coverage.
    codec = path + "/libavcodec"
    libfilter = path + "/libavfilter"
    libfmt = path + "/libavformat"
    # FFmpeg requires this version, not llvm-cov.
    p = subprocess.Popen("gcov *", shell=True, cwd=codec)
    p.wait()
    p = subprocess.Popen("gcov *", shell=True, cwd=libfilter)
    p.wait()
    p = subprocess.Popen("gcov *", shell=True, cwd=libfmt)
    p.wait()

    # Make directories for storing the results.
    coverageFiles = "coverage_files_WITH_" + feature + "_" + flag
    p = subprocess.Popen("mkdir -p " + coverageFiles, shell=True, cwd=path)
    p.wait()
    p = subprocess.Popen("mv " + codec + "/*.gcov " + coverageFiles, shell=True, cwd=path)
    p.wait()
    p = subprocess.Popen("mv " + libfilter + "/*.gcov " + coverageFiles, shell=True, cwd=path)
    p.wait()
    p = subprocess.Popen("mv " + libfmt + "/*.gcov " + coverageFiles, shell=True, cwd=path)
    p.wait()

    # Move the files to working dir.
    home = os.getcwd()
    p = subprocess.Popen(["mv", coverageFiles, home], cwd=path)
    p.wait()

def makeDDS(path, feature):
    print("[-] TODO: makeDDS.")

def makeCM(path, feature):
    print("[-] TODO: makeCM.")

def isTool(prog):
    return shutil.which(prog) is not None

if __name__ == '__main__':
    # Setup the command line args for different projects.
    parser = argparse.ArgumentParser()
    parser.add_argument("project", help="Directory to project to target")
    parser.add_argument("feature", help="Feature to identify/remove from project")
    parser.add_argument("--extract", help="Generate feature graph and show LoC to remove", action="store_true")
    parser.add_argument("--tests", help="Run tests at compile time (necessary for better coverage results)", action="store_true")
    args = parser.parse_args()

    home = os.getcwd()

    if "mosquitto" in args.project:
        # Mosquitto uses all-caps names.
        feature = args.feature.upper()

        # Compile with feature enabled.
        makeMosquitto(args.project, feature, "yes", args.tests)
        # Compile with feature disabled.
        makeMosquitto(args.project, feature, "no", args.tests)

        # Make one file with the `diff` of coverage info.
        diffs = makeDiffs(home + "/coverage_files_WITH_" + feature + "_yes",
            home + "/coverage_files_WITH_" + feature + "_no", feature)
    elif "FFmpeg" in args.project:
        makeFFmpeg(args.project, args.feature, "yes", args.tests)
        makeFFmpeg(args.project, args.feature, "no", args.tests)

        if args.tests:
            # Download the test suite/etc for FFmpeg.
            p = subprocess.Popen(["make", "fate-rsync", "SAMPLES=fate-suite/"], cwd=args.path)
            p.wait()
            # Now we can also run the tests in `makeFFmpeg`.

        # Make one file with the `diff` of coverage info.
        diffs = makeDiffs(home + "/coverage_files_WITH_" + args.feature + "_yes",
            home + "/coverage_files_WITH_" + args.feature + "_no", args.feature)
    else:
        print("[-] Target currently unsupported!")
    
    if args.extract:
        extractFeatures(diffs)

    sys.exit(0)