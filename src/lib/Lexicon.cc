#include <ckylark/Lexicon.h>

#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <iostream>
#include <string>

using namespace std;

namespace Ckylark {

double LexiconEntry::getMaxScore() const {
    double ret = 0.0;
    for (double x : score_) {
        if (x > ret) ret = x;
    }
    return ret;
}

Lexicon::Lexicon(const TagSet & tag_set, int level)
    : tag_set_(tag_set)
    , level_(level)
    , entry_(tag_set.numTags())
    , scaling_() {
}

Lexicon::~Lexicon() {
    for (auto & it1 : entry_) {
        for (auto & it2 : it1) {
            delete it2.second;
        }
    }
}

shared_ptr<Lexicon> Lexicon::loadFromStream(
    istream & stream,
    const Dictionary & word_table,
    const TagSet & tag_set) {

    Lexicon * lex = new Lexicon(tag_set, tag_set.getDepth()-1);
    shared_ptr<Lexicon> plex(lex);

    string line;
    while (getline(stream, line)) {
        boost::trim(line);
        int pos1 = line.find(" ");
        int pos2 = line.find(" ", pos1+1);

        // parse scores
        string tag = line.substr(0, pos1);
        string word = line.substr(pos1+1, pos2-pos1-1);
        int tid = tag_set.getTagId(tag);
        int wid = word_table.getId(word);
        if (wid == -1) {
            if (word.size() >= 3 && word.substr(0, 3) == "UNK") {
                // sum all UNK* entries (nothing to do here)
            } else {
                // skip OOV words
                cerr << "skipped: " << tag << " -> " << word << endl;
                continue;
            }
        }

        string ss = line.substr(pos2+1);
        boost::trim_if(ss, boost::is_any_of("[]"));
        vector<string> ls;
        boost::split(ls, ss, boost::is_any_of(", "), boost::token_compress_on);
        vector<double> score(ls.size());
        transform(ls.begin(), ls.end(), score.begin(), [](const string & x){ return stod(x); });
        
        LexiconEntry & ent = lex->getEntryOrCreate(tid, wid);
        if (score.size() != ent.numSubtags()) {
            throw runtime_error("Lexicon: invalid file format: number of subcategories mismatched");
        }
        for (size_t i = 0; i < score.size(); ++i) {
            ent.addScore(i, score[i]);
        }
    }

    return plex;
}

const LexiconEntry * Lexicon::getEntry(int tag_id, int word_id) const {
    auto it = entry_[tag_id].find(word_id);
    if (it != entry_[tag_id].end()) {
        return it->second;
    }
    return nullptr;
}

LexiconEntry & Lexicon::getEntryOrCreate(int tag_id, int word_id) {
    auto it = entry_[tag_id].find(word_id);
    if (it != entry_[tag_id].end()) {
        return *(it->second);
    }
    LexiconEntry * ent = new LexiconEntry(tag_id, word_id, tag_set_.numSubtags(tag_id, level_));
    entry_[tag_id].insert(make_pair(word_id, ent));
    return *ent;
}

double Lexicon::getScalingFactor(int word_id) const {
    if (scaling_.empty()) {
        // calculate scaling factors
        for (auto & entry_tag : entry_) {
            for (auto & keyval : entry_tag) {
                double max_score = keyval.second->getMaxScore();
                if (max_score > scaling_[keyval.first]) {
                    scaling_[keyval.first] = max_score;
                }
            }
        }
    }

    auto ent = scaling_.find(word_id);
    if (ent != scaling_.end()) {
        if (ent->second > 0.0) return 1.0 / ent->second;
        else return 0.0;
    }
    return 0.0;
}

} // namespace Ckylark

