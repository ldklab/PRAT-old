#!/usr/bin/env python3

import gensim.downloader as api
from gensim.models import TfidfModel
from gensim.corpora import Dictionary

dataset = api.load("text8")
dct = Dictionary(dataset) # Fit dictionary.
corpus = [dct.doc2bow(line) for line in dataset] # Convert corpus to BoW format.

model = TfidfModel(corpus) # Fit model.
vector = model[corpus[0]] # Apply model to the first corpus document.
