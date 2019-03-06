package com.ris3;

import edu.stanford.nlp.coref.data.CorefChain;
import edu.stanford.nlp.ie.util.RelationTriple;
import edu.stanford.nlp.ling.CoreLabel;
import edu.stanford.nlp.pipeline.*;
import edu.stanford.nlp.semgraph.SemanticGraph;
import edu.stanford.nlp.trees.Tree;

import java.util.List;
import java.util.Map;
import java.util.Properties;


public class BasicPipeline {

    private static final String text = "Ryan is from Los Angeles, CA and works in cybersecurity. " +
            "His parents are from London, UK. " +
            "He moved to Boston, MA in 2015 and has been working on his PhD at Northeastern University " +
            "since September 2018. He travels a lot to different countries every year.";

    public static void main(String[] args) {
        Properties props = new Properties();
        props.setProperty("annotators", "tokenize, ssplit, pos, lemma, ner, parse, depparse, coref");
        props.setProperty("ner.model", "edu/stanford/nlp/models/ner/english.all.3class.distsim.crf.ser.gz"); // Later I need to make my own model for RFC docs/etc.
        //props.setProperty("ner.applyNumericClassifiers", "false");
        props.setProperty("coref.algorithm", "neural");
        StanfordCoreNLP pipeline = new StanfordCoreNLP(props);

        // set up pipeline properties
        //Properties props = new Properties();
        // set the list of annotators to run
        //props.setProperty("annotators", "tokenize,ssplit,pos,lemma,ner,parse,depparse,coref,kbp,quote");
        // set a property for an annotator, in this case the coref annotator is being set to use the neural algorithm
        //props.setProperty("coref.algorithm", "neural");
        // build pipeline
        //StanfordCoreNLP pipeline = new StanfordCoreNLP(props);

        // Create a document object
        CoreDocument document = new CoreDocument(text);
        // Annotate the document
        pipeline.annotate(document);

        // 10th token of the document
        CoreLabel token = document.tokens().get(10);
        System.out.println("Example: token");
        System.out.println(token);
        System.out.println();

        // text of the first sentence
        String sentenceText = document.sentences().get(0).text();
        System.out.println("Example: sentence");
        System.out.println(sentenceText);
        System.out.println();

        // second sentence
        CoreSentence sentence = document.sentences().get(1);

        // list of the part-of-speech tags for the second sentence
        List<String> posTags = sentence.posTags();
        System.out.println("Example: pos tags");
        System.out.println(posTags);
        System.out.println();

        // list of the ner tags for the second sentence
        List<String> nerTags = sentence.nerTags();
        System.out.println("Example: ner tags");
        System.out.println(nerTags);
        System.out.println();

        // constituency parse for the second sentence
        Tree constituencyParse = sentence.constituencyParse();
        System.out.println("Example: constituency parse");
        System.out.println(constituencyParse);
        System.out.println();

        // dependency parse for the second sentence
        SemanticGraph dependencyParse = sentence.dependencyParse();
        System.out.println("Example: dependency parse");
        System.out.println(dependencyParse);
        System.out.println();

        // kbp relations found in fifth sentence
        /*
        List<RelationTriple> relations =
                document.sentences().get(4).relations();
        System.out.println("Example: relation");
        System.out.println(relations.get(0));
        System.out.println();*/

        // entity mentions in the second sentence
        List<CoreEntityMention> entityMentions = sentence.entityMentions();
        System.out.println("Example: entity mentions");
        System.out.println(entityMentions);
        System.out.println();

        // coreference between entity mentions
        CoreEntityMention originalEntityMention = document.sentences().get(3).entityMentions().get(1);
        System.out.println("Example: original entity mention");
        System.out.println(originalEntityMention);
        System.out.println("Example: canonical entity mention");
        System.out.println(originalEntityMention.canonicalEntityMention().get());
        System.out.println();

        // get document wide coref info
        Map<Integer, CorefChain> corefChains = document.corefChains();
        System.out.println("Example: coref chains for document");
        System.out.println(corefChains);
        System.out.println();

        // get quotes in document
        /*List<CoreQuote> quotes = document.quotes();
        CoreQuote quote = quotes.get(0);
        System.out.println("Example: quote");
        System.out.println(quote);
        System.out.println();

        // original speaker of quote
        // note that quote.speaker() returns an Optional
        System.out.println("Example: original speaker of quote");
        System.out.println(quote.speaker().get());
        System.out.println();

        // canonical speaker of quote
        System.out.println("Example: canonical speaker of quote");
        System.out.println(quote.canonicalSpeaker().get());
        System.out.println();*/

    }

}