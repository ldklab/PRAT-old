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

# Generate coverage files for Mosquitto.
def makeMosquitto(path, feature, flag):
    print("[+] Running in: {}".format(path))
    target = "WITH_" + feature + "=" + flag
    p = subprocess.Popen(["make", "clean"], cwd=path)
    p = subprocess.Popen(["make", "binary", "-j", target], cwd=path)
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

    home = os.getcwd()

    if "mosquitto" in args.project:
        # Compile with feature enabled.
        makeMosquitto(args.project, args.feature, "yes")
        # Compile with feature disabled.
        makeMosquitto(args.project, args.feature, "no")

        # Make one file with the `diff` of coverage info.
        makeDiffs(home + "/coverage_files_WITH_" + args.feature + "_yes",
            home + "/coverage_files_WITH_" + args.feature + "_no", args.feature)
    elif "FFmpeg" in args.project:
        makeFFmpeg(args.project, args.feature)
    else:
        print("[-] Target currently unsupported!")

    sys.exit(0)