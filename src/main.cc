#include "ArgumentParser.h"
#include "Formatter.h"
#include "LAPCFGParser.h"
#include "Mapping.h"
#include "Timer.h"

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace AHCParser;

unique_ptr<ArgumentParser> parseArgs(int argc, char * argv[]) {
    unique_ptr<ArgumentParser> ap(new ArgumentParser("ahcparse"));
    ap->addArgument(ArgumentParser::ARGTYPE_STRING, "input", "PATH", "/dev/stdin", "input file", false);
    ap->addArgument(ArgumentParser::ARGTYPE_STRING, "output", "PATH", "/dev/stdout", "output file", false);
    ap->addArgument(ArgumentParser::ARGTYPE_STRING, "model", "PATH", "", "model directory", true);
    if (!ap->parseArgs(argc, argv)) {
        ap->printUsage();
        exit(0);
    }
    return ap;
}

int main(int argc, char * argv[]) {
    unique_ptr<ArgumentParser> ap = ::parseArgs(argc, argv);
    shared_ptr<LAPCFGParser> parser = LAPCFGParser::loadFromBerkeleyDump(ap->getString("model"));

    cerr << "Ready" << endl;

    //const Dictionary & words = parser->getWordTable();
    //const TagSet & tags = parser->getTagSet();
    //int depth = tags.getDepth();

    /*
    int cid1 = model.category->getCategoryId("VP");
    int cid2 = model.category->getCategoryId("VB");
    int cid3 = model.category->getCategoryId("NP");
    cout << model.grammar[0]->getBinaryRule(cid1, cid2, cid3).getScore(38, 16, 11) << endl;
    cout << model.grammar[0]->getBinaryRule(cid1, cid2, cid3).getScore(38, 16, 12) << endl;
    */

    /*
    int c1 = tags.getTagId("NP");
    int c2 = tags.getTagId("DT");

    cout << c1 << ' ' << c2 << endl;

    for (int lv = 0; lv < depth; ++lv) {
        Grammar & grammar = parser->getGrammar(lv);
        UnaryRule & rule = grammar.getUnaryRule(c1, c2);
        int n1 = rule.numParentSubtags();
        int n2 = rule.numChildSubtags();
        printf("parent=%d, child=%d\n", n1, n2);
        for (int i1 = 0; i1 < n1; ++i1) {
            for (int i2 = 0; i2 < n2; ++i2) {
                double score = rule.getScore(i1, i2);
                if (score == 0.0) printf("___ ");
                else printf("%3.0f ", log(score));
            }
            printf("\n");
        }
    }
    */

    /*
    int cid = category.getCategoryId("DT");
    int wid = word_table.getId("the");

    cout << cid << ' ' << wid << endl;

    for (int lv = 0; lv < depth; ++lv) {
        LexiconEntry & ent = parser->getLexicon(lv).getEntry(cid, wid);
        int n = category.getNumSubcategories(cid, lv);
        cout << endl << "level = " << lv << endl;
        for (int i = 0; i < n; ++i) {
            cout << ent.getScore(i) << endl;
        }
    }
    */

    Timer timer;

    string line;
    int total_lines = 0;
    int total_words = 0;
    while (getline(cin, line)) {
        boost::trim(line);
        vector<string> ls;
        boost::split(ls, line, boost::is_any_of(" "));
        
        ++total_lines;
        total_words += ls.size();
        
        cerr << "Input " << total_lines << ":";
        for (const string & s : ls) cerr << " " << s;
        cerr << endl;

        timer.start();
        shared_ptr<Tree<string> > parse = parser->parse(ls);
        double lap = timer.stop();

        string repr = Formatter::ToPennTreeBank(*parse);
        cerr << "  Parse: " << repr << endl;
        fprintf(stderr, "  Time: %.4fs\n", lap);
        cout << repr << endl;
    }

    cerr << endl;
    fprintf(stderr, "Parsed %d sentences, %d words.\n", total_lines, total_words);
    fprintf(stderr, "Total parsing time: %.3fs.\n", timer.elapsed());

    return 0;
}

