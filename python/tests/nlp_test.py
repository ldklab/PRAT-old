#!/usr/bin/env python3

import nltk
from nltk.corpus import treebank

nltk.download('punkt')
nltk.download('treebank')
nltk.download('averaged_perceptron_tagger')

sentence = """At eight o'clock in the morning on Thursday 
    Ryan didn't feel too good."""
tokens = nltk.word_tokenize(sentence)

print("Tokens: {}".format(tokens))

tagged = nltk.pos_tag(tokens)

print("Tagged: {}".format(tagged))

t = treebank.parsed_sents('wsj_0001.mrg')[0]
t.draw()