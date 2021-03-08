#!/usr/bin/env python3

import argparse
import subprocess
import sys, os
import shutil
import toml

def makeDiffs(path1, path2, feat):
    print("[+] Checking for matching files in {} and {}"
        .format(path1,  path2))
    
    unused_files = []
    
    aFiles = {os.path.splitext(x)[0] for x in os.listdir(path1)}
    bFiles = {os.path.splitext(x)[0] for x in os.listdir(path2)}

    outdir = "diff_" + feat
    p = subprocess.Popen(["mkdir", "-p", outdir])
    p.wait()
    p = subprocess.Popen(["mkdir", "-p", "reports"])
    p.wait()

    for f in aFiles:
        # We only diff files that exist in both compilations.
        if f in bFiles:
            #print("[+] {}".format(f))
            # Make diffs of the file and save in another folder.
            target = f + ".gcov"
            abs_target = outdir + "/" + target
            out = open(abs_target, 'w')
            p = subprocess.Popen(["diff", "-u", path1 + "/" + target, path2 + "/" + target], stdout=out)
            p.wait()
        else:
            # If an entire file is left out, we could posit that
            # it is only used when the tested feature ie enabled.
            print("[+] {} only exists with {} enabled".format(f, feat))
            # These files are useful in some cases. Save and mark for deletion later.
            unused_files.append(f)
    
    # Clean up empty files.
    for covFile in os.listdir(outdir):
        real_file = outdir + "/" + covFile
        if not os.path.getsize(real_file):
            #print("[-] {} is empty. Deleting".format(covFile))
            os.remove(real_file)
        else:
            # Now that we've cleaned up.
            # Generate HTML files here.
            if isTool("pygmentize"):
                print("[+] Generating HTML assets...")
                p = subprocess.Popen(["pygmentize", "-l", "diff", "-f", "html", "-O", "full", "-o", "reports/" + covFile + "-diff.html", real_file])
                p.wait()
            else:
                print("[-] `pygments` is not available. Install with: `pip install Pygments`")
                continue
    
    #return unused_files
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
    
    # TODO: make this output content from `genhtml` or something to
    # make the output a hierarchical webpage showing source files
    # and not just the current graphviz output.
    html = "<html><body><h3>Files with content to remove</h3>"
    for report in os.listdir("./reports"):
        html += "<a href=\"./reports/%s\">%s</a><br/>" % (report, report)
    outhtml = open("report.html", 'w')
    outhtml.writelines(html)
    
    print("[+] Attempting to open with Firefox...")
    p = subprocess.Popen(["firefox", "report.html"])
    p.wait()

# Generate coverage files for Mosquitto.
def makeMosquitto(path, feature, flag, tests=False):
    print("[+] Running in: {}".format(path))
    target = "WITH_" + feature + "=" + flag
    p = subprocess.Popen(["make", "clean"], cwd=path)
    p.wait()
    p = subprocess.Popen(["make", "binary", "-j", target], cwd=path)
    p.wait()

    # Add part here later for running tests.
    if tests is not False:
        p = subprocess.Popen(["make", "test", "-j"], cwd=path)
        p.wait()
    else:
        print("[+] Not running tests. Continuing.")

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

def makeFFmpeg(path, feature, flag, tests=False):
    print("[+] Running in: {}".format(path))

    # This is just a test for now, but could be useful later.
    # TODO: make a baseline/non-feature specific test.
    if feature == "baseline":
        if flag == "yes":
            # No explicit 'enable' flag for FFmpeg.
            p = subprocess.Popen(["bash", "configure", "--toolchain=gcov"], cwd=path)
            p.wait()
        else:
            target = "--disable-" + feature
            p = subprocess.Popen(["bash", "configure", "--toolchain=gcov", target], cwd=path)
            #p = subprocess.Popen("bash configure --toolchain=gcov --disable-", shell=True, cwd=path)
            p.wait()
    else:
        # Prep the build system using configure.
        if flag == "yes":
            # No explicit 'enable' flag for FFmpeg.
            p = subprocess.Popen(["bash", "configure", "--toolchain=gcov"], cwd=path)
            p.wait()
        else:
            target = "--disable-" + feature
            p = subprocess.Popen(["bash", "configure", "--toolchain=gcov", target], cwd=path)
            #p = subprocess.Popen("bash configure --toolchain=gcov --disable-", shell=True, cwd=path)
            p.wait()
    
    p = subprocess.Popen(["make", "clean"], cwd=path)
    p.wait()

    p = subprocess.Popen(["make", "-j3"], cwd=path)
    p.wait()

    # Add part later for running tests.
    if tests is not False:
        p = subprocess.Popen(["make", "fate", "-j3", "SAMPLES=fate-suite/"], cwd=path)
        p.wait()

    #p = subprocess.Popen(["./ffmpeg", "--help"], cwd=path)
    #p.wait()

    # FFmpeg requires this version, not llvm-cov.
    p = subprocess.Popen("gcov libavcodec/*", shell=True, cwd=path)
    p.wait()
    p = subprocess.Popen("gcov libavfilter/*", shell=True, cwd=path)
    p.wait()
    p = subprocess.Popen("gcov libavformat/*", shell=True, cwd=path)
    p.wait()

    # Make directories for storing the results.
    coverageFiles = "coverage_files_WITH_" + feature + "_" + flag
    p = subprocess.Popen("mkdir -p " + coverageFiles, shell=True, cwd=path)
    p.wait()
    p = subprocess.Popen("mv " + "*.gcov " + coverageFiles, shell=True, cwd=path)
    p.wait()
    p = subprocess.Popen("mv " + "*.gcov " + coverageFiles, shell=True, cwd=path)
    p.wait()
    p = subprocess.Popen("mv " + "*.gcov " + coverageFiles, shell=True, cwd=path)
    p.wait()

    # Move the files to working dir.
    home = os.getcwd()
    p = subprocess.Popen(["mv", coverageFiles, home], cwd=path)
    p.wait()

def makeRust(path, feature, flag, tests=False):
    print("[+] Prepping Rust environment for instrumentation...")
    os.environ["CARGO_INCREMENTAL"] = "0"
    os.environ["RUSTFLAGS"] = "-Zprofile -Ccodegen-units=1 -Copt-level=0 -Clink-dead-code -Coverflow-checks=off -Zpanic_abort_tests -Cpanic=abort"
    os.environ["RUSTDOCFLAGS"] = "-Cpanic=abort"

    print("[+] Loading `Cargo.toml`...")
    cargo = toml.load(path + "/Cargo.toml")
    features = cargo["features"]

    print("[+] Found {} features\n".format(len(features)))

    p = subprocess.Popen(["cargo", "clean"], cwd=path)
    p.wait()

    #for f in features:
    #    print("[+] {}".format(f))
    
    if flag == "yes":
        p = subprocess.Popen(["cargo", "build", "--features", feature], cwd=path)
        p.wait()
    else:
        p = subprocess.Popen(["cargo", "build", "--no-default-features"], cwd=path)
        p.wait()
    
    if tests is not False:
        # Now generate the coverage files using kcov.
        p = subprocess.Popen(["cargo", "test"], cwd=path)
        p.wait()

    src = path + "/target/debug/deps/"
    p = subprocess.Popen("gcov-7 *", shell=True, cwd=src)
    p.wait()

    # Make directories for storing the results.
    coverageFiles = "coverage_files_WITH_" + feature + "_" + flag
    p = subprocess.Popen("mkdir -p " + coverageFiles, shell=True, cwd=path)
    p.wait()
    p = subprocess.Popen("mv " + src + "*.gcov " + coverageFiles, shell=True, cwd=path)
    p.wait()

    # Move the files to working dir.
    home = os.getcwd()
    p = subprocess.Popen(["mv", coverageFiles, home], cwd=path)
    p.wait()

def makeAOM(path, feature, flag, tests=False):
    print("[+] Running in: {}".format(path))

    if flag == "yes":
        target = "-DCONFIG_" + feature + "=1"
    else:
        target = "-DCONFIG_" + feature + "=0"

    build = path + "/build"
    p = subprocess.Popen(["cmake", target, ".."], cwd=build)
    p.wait()

    #p = subprocess.Popen(["make", "clean"], cwd=path)
    #p.wait()
    p = subprocess.Popen(["make", "-j"], cwd=build)
    p.wait()

    # Add part here later for running tests.
    if tests is not False:
        p = subprocess.Popen("./test_libaom", shell=True, cwd=build)
        p.wait()
    else:
        print("[+] Not running tests. Continuing.")

    # Generate gcov files.
    src = path + "/build/CMakeFiles/aom.dir/aom/src/"
    p = subprocess.Popen("gcov *", shell=True, cwd=src)
    p.wait()

    # Make directories for storing the results.
    coverageFiles = "coverage_files_WITH_" + feature + "_" + flag
    p = subprocess.Popen("mkdir -p " + coverageFiles, shell=True, cwd=path)
    p.wait()
    p = subprocess.Popen("mv " + src + "/*.gcov " + coverageFiles, shell=True, cwd=path)
    p.wait()
    #p = subprocess.Popen("mv " + lib + "/*.gcov " + coverageFiles, shell=True, cwd=path)
    #p.wait()

    # Cleanup before leaving.
    p = subprocess.Popen("rm -rf *", shell=True, cwd=build)
    p.wait()
    p = subprocess.Popen("git checkout .", shell=True, cwd=build)
    p.wait()

    # Move the files to working dir.
    home = os.getcwd()
    p = subprocess.Popen(["mv", coverageFiles, home], cwd=path)
    p.wait()

def makeDDS(path, feature, flag, tests=False):
    print("[+] Running in: {}".format(path))

    # Prep the build system using configure.
    if flag == "yes":
        # No explicit 'enable' flag for FFmpeg.
        p = subprocess.Popen(["bash", "configure", "--no-tests"], cwd=path)
        p.wait()
    else:
        target = "--disable-" + feature
        p = subprocess.Popen(["bash", "configure", "--no-tests", target], cwd=path)
        p.wait()
    
    p = subprocess.Popen(["make", "clean"], cwd=path)
    p.wait()

    p = subprocess.Popen(["make", "-j3"], cwd=path)
    p.wait()

    # TODO: finish running tests and making gcov files.

def makeCM(path, feature, flag, tests=False):
    print("[-] TODO: makeCM.")

def isTool(prog):
    return shutil.which(prog) is not None

if __name__ == '__main__':
    # Setup the command line args for different projects.
    parser = argparse.ArgumentParser()
    parser.add_argument("project", help="Directory to project to target")
    parser.add_argument("feature", help="Feature to identify/remove from project")
    parser.add_argument("--list", help="List the features for the target codebase", action="store_true")
    parser.add_argument("--extract", help="Generate feature graph and show LoC to remove", action="store_true")
    parser.add_argument("--tests", help="Run tests at compile time (necessary for better coverage results)", action="store_true")
    parser.add_argument("--delete", help="Attempt to delete entire feature-specific files after analysis", action="store_true")
    args = parser.parse_args()

    home = os.getcwd()

    # Before checking the main loop below; look for list flag.
    if args.list:
        print("[+] Getting features available from: {}".format(args.project))
        sys.exit(0)

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
        
        #print(f"Running make clean in: {args.project}")
        p = subprocess.Popen(["make", "clean"], cwd=args.project)
        p.wait()
    elif "FFmpeg" in args.project:
        if args.tests:
            if not os.path.isfile(args.project + "/fate-suite"):
                # Download the test suite/etc for FFmpeg.
                p = subprocess.Popen(["make", "fate-rsync", "SAMPLES=fate-suite/"], cwd=args.project)
                p.wait()
                # Now we can also run the tests in `makeFFmpeg`.

        makeFFmpeg(args.project, args.feature, "yes", args.tests)
        makeFFmpeg(args.project, args.feature, "no", args.tests)

        # Make one file with the `diff` of coverage info.
        diffs = makeDiffs(home + "/coverage_files_WITH_" + args.feature + "_yes",
            home + "/coverage_files_WITH_" + args.feature + "_no", args.feature)

        # Attempt to delete feature-specific source files.
        # I made a change that broke this. Will fix later.
        if args.delete is not False:
            print("[+] Attempting to delete source files...")
            for td in diffs:
                if os.path.exists(args.project + "/libavfilter/" + td):
                    os.remove(args.project + "/libavfilter/" + td)
                elif os.path.exists(args.project + "/libavcodec/" + td):
                    os.remove(args.project + "/libavcodec/" + td)
                elif os.path.exists(args.project + "/libavformat/" + td):
                    os.remove(args.project + "/libavformat/" + td)
                else:
                    print("[-] File: {} could not be found in source tree; skipping.".format(td))
            print("[+] Finished deleting source files")
    elif "rav1e" in args.project:
        print("[+] Experimental feature: running on Rust-based project")

        makeRust(args.project, args.feature, "yes", args.tests)
        makeRust(args.project, args.feature, "no", args.tests)

        # Make one file with the `diff` of coverage info.
        diffs = makeDiffs(home + "/coverage_files_WITH_" + args.feature + "_yes",
            home + "/coverage_files_WITH_" + args.feature + "_no", args.feature)
    elif "aom" in args.project:
        # libaom uses all-caps names.
        feature = args.feature.upper()

        # Compile with feature enabled.
        makeAOM(args.project, feature, "yes", args.tests)
        # Compile with feature disabled.
        makeAOM(args.project, feature, "no", args.tests)

        # Make one file with the `diff` of coverage info.
        diffs = makeDiffs(home + "/coverage_files_WITH_" + feature + "_yes",
            home + "/coverage_files_WITH_" + feature + "_no", feature)
    elif "DDS" in args.project:
        # Compile with feature enabled.
        makeDDS(args.project, args.feature, "yes", args.tests)
        # Compile with feature disabled.
        makeDDS(args.project, args.feature, "no", args.tests)

        # Make one file with the `diff` of coverage info.
        diffs = makeDiffs(home + "/coverage_files_WITH_" + feature + "_yes",
            home + "/coverage_files_WITH_" + feature + "_no", feature)
    elif "azure" in args.project:
        # Compile with feature enabled.
        makeCM(args.project, args.feature, "yes", args.tests)
        # Compile with feature disabled.
        makeCM(args.project, args.feature, "no", args.tests)

        # TODO: make diffs.
    elif "quiche" in args.project:
        print("[+] Experimental feature: running on Rust-based project")

        makeRust(args.project, args.feature, "yes", args.tests)
        makeRust(args.project, args.feature, "no", args.tests)

        # Make one file with the `diff` of coverage info.
        diffs = makeDiffs(home + "/coverage_files_WITH_" + args.feature + "_yes",
            home + "/coverage_files_WITH_" + args.feature + "_no", args.feature)
    else:
        print("[-] Target currently unsupported!")
    
    if args.extract:
        extractFeatures(diffs)

    sys.exit(0)