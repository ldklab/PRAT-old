#!/usr/bin/env python3

import spacy
from spacy import displacy

nlp = spacy.load('en') #en_core_web_lg

doc = nlp("""If the ACK control bit is set this field contains the value of the
    next sequence number the sender of the segment is expecting to
    receive. Once a connection is established this is always sent.""")

for ent in doc.ents:
    print(ent.text, ent.label_)

#for token in doc:
#    print("{0}/{1} <--{2}-- {3}/{4}".format(
#        token.text, token.tag_, token.dep_, token.head.text, token.head.tag_))

displacy.serve(doc, style='dep') # style='dep' for dependency tree.
