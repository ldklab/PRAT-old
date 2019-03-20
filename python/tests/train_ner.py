#!/usr/bin/env python3

from __future__ import unicode_literals, print_function

import plac
import random
import spacy

from spacy.util import minibatch, compounding
from pathlib import Path

# Define some training data (text from RFC 793).
TRAINING_DATA = [
	("Who is Ryan Williams?", {"entities": [(7, 20, "PERSON")]}),
	("I like London and Berlin.", {"entities": [(7, 13, "LOC"), (18, 24, "LOC")]})
]

# Language model to load.
LANG = "en"

@plac.annotations(
    model=("Model name. Defaults to blank 'en' model.", "option", "m", str),
    output_dir=("Optional output directory", "option", "o", Path),
    n_iter=("Number of training iterations", "option", "n", int),
)
def main(model=None, output_dir=None, n_iter=100):
	""" Load model, setup pipeline, train NER. """

	if model is not None:
		nlp = spacy.load(model) # Load existing model.
	else:
		nlp = spacy.blank(LANG) # Blank language class.
		print("[+] Created blank {} model".format(LANG))

	# Create the pipeline components.
	# nlp.create_pipe works for built-ins that are registered with spaCy.
	if "ner" not in nlp.pipe_names:
		ner = nlp.create_pipe("ner")
		nlp.add_pipe(ner, last=True)
	# Else, get it so we can add labels.
	else:
		ner = nlp.get_pipe("ner")

	# Add the labels.
	for _, annotations in TRAINING_DATA:
		for ent in annotations.get("entities"):
			#print("[+] Adding entity: {}".format(ent[2]))
			ner.add_label(ent[2])

	# Disable other pipelines during training.
	other_pipes = [pipe for pipe in nlp.pipe_names if pipe != "ner"]
	# Only want to train NER.
	with nlp.disable_pipes(*other_pipes):
		# Reset and initialize weights randomly if training new model.
		if model is None:
			nlp.begin_training()

		for itn in range(n_iter):
			random.shuffle(TRAINING_DATA)
			losses = {}

			# Batch up examples using minibatch.
			batches = minibatch(TRAINING_DATA, size=compounding(4.0, 32.0, 1.001))
			for batch in batches:
				texts, annotations = zip(*batch)
				nlp.update(
					texts, 			# Batch of texts.
					annotations, 	# Batch of annotations.
					drop=0.5, 		# Dropout - harder to memorize data.
					losses=losses,
				)

		print("[+] Losses {}".format(losses))

	# Test the trained model.
	for text, _ in TRAINING_DATA:
		doc = nlp(text)
		print("[+] Entities: {}".format([(ent.text, ent.label_) for ent in doc.ents]))
		print("[+] Tokens: {}".format([(t.text, t.ent_type_, t.ent_iob) for t in doc]))

	# Save model.
	if output_dir is not None:
		output_dir = Path(output_dir)
		if not output_dir.exists():
			output_dir.mkdir()
		nlp.to_disk(output_dir)
		print("[+] Saved mode to: {}".format(output_dir))

		# Test saved model.
		print("[+] Loading from: {}".format(output_dir))
		nlp2 = spacy.load(output_dir)

		for text, _ in TRAINING_DATA:
			doc = nlp2(text)
			#print("[+] Entities: {}".format([(ent.text, ent.label_) for ent in doc.ents]))
			#print("[+] Tokens: {}".format([(t.text, t.ent_type_, t.ent_iob) for t in doc]))

if __name__ == "__main__":
	plac.call(main)