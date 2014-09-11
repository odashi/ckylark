#include "ArgumentParser.h"
#include "Formatter.h"
#include "LAPCFGParser.h"
#include "Mapping.h"
#include "Timer.h"

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <fstream>
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

    ap->addArgument(ArgumentParser::ARGTYPE_REAL, "smooth-unklex", "FLOAT", "1e-10", "smoothing strength using UNK lexicon", false);

    if (!ap->parseArgs(argc, argv)) {
        ap->printUsage();
        exit(0);
    }
    return ap;
}

int main(int argc, char * argv[]) {
    unique_ptr<ArgumentParser> ap = ::parseArgs(argc, argv);
    
    string ifname = ap->getString("input");
    ifstream ifs(ifname);
    if (!ifs.is_open()) {
        cerr << "ERROR: cannot open file to read: " << ifname << endl;
        return 1;
    }

    string ofname = ap->getString("output");
    ofstream ofs(ofname);
    if (!ofs.is_open()) {
        cerr << "ERROR: cannot open file to write: " << ofname << endl;
        return 1;
    }

    shared_ptr<LAPCFGParser> parser = LAPCFGParser::loadFromBerkeleyDump(ap->getString("model"));
    parser->setUNKLexiconSmoothing(ap->getReal("smooth-unklex"));

    Timer timer;

    cerr << "Ready" << endl;

    string line;
    int total_lines = 0;
    int total_words = 0;
    
    while (getline(ifs, line)) {
        boost::trim(line);
        vector<string> ls;
        if (!line.empty()) {
            boost::split(ls, line, boost::is_space(), boost::algorithm::token_compress_on);
        }
        
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
        fprintf(stderr, "  Time: %.3fs\n", lap);
        ofs << repr << endl;
    }

    cerr << endl;
    fprintf(stderr, "Parsed %d sentences, %d words.\n", total_lines, total_words);
    fprintf(stderr, "Total parsing time: %.3fs.\n", timer.elapsed());

    return 0;
}

