#!/usr/bin/env python3

import argparse
import sys

def makeMosquitto(path, feature):
    print("[-] TODO: makeMosquitto.")

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
        makeMosquitto(args.project, args.feature)
    elif "FFmpeg" in args.project:
        makeFFmpeg(args.project, args.feature)
    else:
        print("[-] Target currently unsupported!")

    sys.exit(0)