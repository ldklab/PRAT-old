#!/usr/bin/env python3

from __future__ import unicode_literals, print_function

import plac
import random
import json
import spacy

from spacy.util import minibatch, compounding, decaying
from pathlib import Path

# Language model to load.
LANG = "en"

# Default training data.
TRAINING_DATA = [
    ("Who is Inigo Montoya?", {"entities": [(7, 20, "PERSON")]}),
    ("I like London and Berlin.", {"entities": [(7, 13, "LOC"), (18, 24, "LOC")]}),
]

def load_data(path):
	with open(path) as f:
		data = json.load(f)
	print("[+] Finished loading {}".format(path))
	return data

def get_batches(train_data, model_type):
    max_batch_sizes = {"tagger": 32, "parser": 16, "ner": 16, "textcat": 64}
    max_batch_size = max_batch_sizes[model_type]
    if len(train_data) < 1000:
        max_batch_size /= 2
    if len(train_data) < 500:
        max_batch_size /= 2
    batch_size = compounding(1, max_batch_size, 1.001)
    batches = minibatch(train_data, size=batch_size)
    return batches

@plac.annotations(
    model=("Model name. Defaults to blank 'en' model.", "option", "m", str),
    input_file=("spaCy JSON file to load from.", "option", "i", str),
    output_dir=("Optional output directory", "option", "o", Path),
    n_iter=("Number of training iterations", "option", "n", int),
)
def main(model=None, input_file=None, output_dir=None, n_iter=100):
	""" Load model, setup pipeline, train NER. """

	if model is not None:
		nlp = spacy.load(model) # Load existing model.
	else:
		nlp = spacy.blank(LANG) # Blank language class.
		print("[+] Created blank {} model".format(LANG))

	if input_file is not None:
		# Manually-annotated data.
		training_data = load_data(input_file)
	else:
		print("[-] No input specified, using example training set.")
		training_data = TRAINING_DATA

	# Change later, but this is useful for smaller networks.
	dropout = decaying(0.6, 0.2, 1e-4)

	# Create the pipeline components.
	# nlp.create_pipe works for built-ins that are registered with spaCy.
	if "ner" not in nlp.pipe_names:
		ner = nlp.create_pipe("ner")
		nlp.add_pipe(ner, last=True)
	# Else, get it so we can add labels.
	else:
		ner = nlp.get_pipe("ner")

	# Add the labels.
	for _, annotations in training_data:
		for ent in annotations.get("entities"):
			#print("[+] Adding entity: {}".format(ent[2]))
			ner.add_label(ent[2])

	# Disable other pipelines during training.
	other_pipes = [pipe for pipe in nlp.pipe_names if pipe != "ner"]
	# Only want to train NER.
	with nlp.disable_pipes(*other_pipes):
		# Reset and initialize weights randomly if training new model.
		if model is None:
			optimizer = nlp.begin_training()
			#print("Optimizer attrs: {}".format(optimizer.__dict__))
		else:
			# Fix later.
			optimizer = nlp.begin_training()

		for itn in range(n_iter):
			random.shuffle(training_data)
			losses = {}

			# Batch up examples using minibatch.
			#batches = minibatch(training_data, size=compounding(1.0, 32.0, 1.001))
			batches = get_batches(training_data, "ner")
			for batch in batches:
				texts, annotations = zip(*batch)
				nlp.update(
					texts, 					# Batch of texts.
					annotations, 			# Batch of annotations.
					drop=next(dropout), 	# Dropout - harder to memorize data.
					sgd=optimizer,
					losses=losses,
				)

			print("[+] Epoch {} of {}\t loss: {}".format(itn+1, n_iter, losses))

	# Test the trained model.
	for text, _ in training_data:
		doc = nlp(text)
		print("[+] Entities: {}".format([(ent.text, ent.label_) for ent in doc.ents]))
		#print("[+] Tokens: {}".format([(t.text, t.ent_type_, t.ent_iob) for t in doc]))

	# Save model.
	if output_dir is not None:
		output_dir = Path(output_dir)
		if not output_dir.exists():
			output_dir.mkdir()

		# Save using averages, not most recent value.
		with nlp.use_params(optimizer.averages):
			nlp.to_disk(output_dir)
			print("[+] Saved mode to: {}".format(output_dir))

		# Test saved model.
		#print("[+] Loading from: {}".format(output_dir))
		#nlp2 = spacy.load(output_dir)

		#for text, _ in training_data:
			#doc = nlp2(text)
			#print("[+] Entities: {}".format([(ent.text, ent.label_) for ent in doc.ents]))
			#print("[+] Tokens: {}".format([(t.text, t.ent_type_, t.ent_iob) for t in doc]))

if __name__ == "__main__":
	plac.call(main)