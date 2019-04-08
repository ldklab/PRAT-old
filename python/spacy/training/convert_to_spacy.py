#!/usr/bin/env python3

import json
import sys
import logging

def convert_dataturks_to_spacy(dataturks_JSON_FilePath):
    try:
        training_data = []
        lines=[]
        with open(dataturks_JSON_FilePath, 'r') as f:
            lines = f.readlines()

        for line in lines:
            data = json.loads(line)
            text = data['content']
            entities = []
            for annotation in data['annotation']:
                # only a single point in text annotation.
                point = annotation['points'][0]
                labels = annotation['label']
                # handle both list of labels or a single label.
                if not isinstance(labels, list):
                    labels = [labels]

                for label in labels:
                    # dataturks indices are both inclusive [start, end] but spacy is not [start, end)
                    entities.append((point['start'], point['end'] + 1 ,label))


            training_data.append((text, {"entities" : entities}))

        return training_data
    except Exception as e:
        logging.exception("Unable to process " + dataturks_JSON_FilePath + "\n" + "error = " + str(e))
        return None

if __name__ == "__main__":

    if len(sys.argv) < 3:
        print("Usage: python {} inputJSON outputSpacy".format(sys.argv[0]))
        sys.exit()

    out_f = open(sys.argv[2] + '.json', 'w')

    spacy_data = convert_dataturks_to_spacy(sys.argv[1])
    json.dump(spacy_data, out_f)
    out_f.close()
