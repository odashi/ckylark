#include <ckylark/PLFLatticeLoader.h>

#include <boost/algorithm/string.hpp>

#include <cctype>
#include <functional>
#include <string>

//#include <iostream>

using namespace std;

namespace Ckylark {

PLFLatticeLoader::PLFLatticeLoader() {}

PLFLatticeLoader::~PLFLatticeLoader() {}

shared_ptr<Lattice> PLFLatticeLoader::getNextData() {
    shared_ptr<istream> stream = getInput();

    string line;
    if (!getline(*stream, line)) {
        // end of stream
        return shared_ptr<Lattice>(nullptr);
    }

    shared_ptr<Lattice> lattice(new Lattice());

    boost::trim(line);
    if (line.empty()) {
        // empty line
        return lattice;
    }

    const size_t length = line.size();
    size_t pos = 0;
    size_t begin = 0;

    // exception for bad parsing
    runtime_error ERROR("PLFLatticeLoader: invalid format");    

    // procedure for moving to next character
    function<void(bool)> skip = [&](bool skip_space) {
        do ++pos; while (skip_space && pos < length && isspace(line[pos]));
        if (pos >= length) throw ERROR;
    };

    // procedure for matching character of current position
    function<void(char)> check = [&](char expected) {
        if (line[pos] != expected) throw ERROR;
    };

    // parse

    check('(');
    skip(true);
    
    while (line[pos] != ')') {
        check('(');
        skip(true);
        
        while (line[pos] != ')') {
            check('(');
            skip(true);
            
            check('\'');
            skip(false);
            string word = "";
            while (line[pos] != '\'') {
                if (line[pos] == '\\') {
                    word += '\\';
                    skip(false);
                }
                word += line[pos];
                skip(false);
            }
            skip(true);
            check(',');
            skip(true);
            
            string score_str = "";
            while (line[pos] != ',') {
                score_str += line[pos];
                skip(true);
            }
            skip(true);
            
            string span_str = "";
            while (line[pos] != ',' && line[pos] != ')') {
                span_str += line[pos];
                skip(true);
            }
            if (line[pos] == ',') {
                skip(true);
            }
            skip(true);
            
            double score = 0.0;
            int span = 0;
            try {
                score = stof(score_str);
                span = stoi(span_str);
            } catch (...) {
                throw ERROR;
            }

            //cout << word << ' ' << score << ' ' << begin << ' ' << (begin + span) << endl;

            lattice->addEdge(begin, begin + span, { word, score });

            if (line[pos] == ',') {
                skip(true);
            }
        }
        skip(true);
        if (line[pos] == ',') {
            skip(true);
        }

        ++begin;
    }

    return lattice;
}

} // namespace Ckylark

