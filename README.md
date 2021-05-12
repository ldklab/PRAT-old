## PRAT

![](https://github.com/RiS3-Lab/PRAT/workflows/Demo-Container-Build/badge.svg)  
Protocol Representation and Analysis Toolkit

### Preparing Environment
`sudo ./setup.sh`

### Generating Coverage Files and Diffs from Source
From the `src` directory:
```
usage: PRAT.py [-h] [--list] [--extract] [--tests] [--delete] project feature

positional arguments:
  project     Directory to project to target
  feature     Feature to identify/remove from project

optional arguments:
  -h, --help  show this help message and exit
  --list      List the features for the target codebase
  --extract   Generate feature graph and show LoC to remove
  --tests     Run tests at compile time (necessary for better coverage results)
  --delete    Attempt to delete entire feature-specific files after analysis
```
The `--extract` option is currently still in progress, but it generates a set of HTML files corresponding to the files and respective lines of code to remove for a given feature. Passing this feature will (attempt) to automatically open a browser with the table of applicable source files/artifacts.
