#!/usr/bin/env python3

import re
import pandas as pd
from time import time
from collections import defaultdict

import gzip
import gensim

# Gensim logger setup.
import logging
logging.basicConfig(format="%(asctime)s : %(levelname)s : %(message)s", datefmt='%H:%M:%S', level=logging.INFO)

# Using params from Word2Vec_FastText_Comparison.
params = {
    'alpha': 0.05,
    'size': 100,
    'window': 5,
    'iter': 5,
    'min_count': 5,
    'sample': 1e-4,
    'sg': 1,
    'hs': 0,
    'negative': 5
}

def read_input(input_file):
	""" Method to read input file in gzip format. """
	logging.info("Reading file {}... this may take a while".format(input_file))

	with gzip.open(input_file, 'rb') as f:
		for i, line in enumerate(f):
			if (i % 10000 == 0):
				logging.info("Read {} lines".format(i))

			# Return a list of pre-processed words.
			yield gensim.utils.simple_preprocess(line)