#include "Dictionary.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace AHCParser {

Dictionary::Dictionary() {}

Dictionary::~Dictionary() {}

void Dictionary::addWord(const string & word) {
    map<string, int>::iterator it = ids_.find(word);
    if (it == ids_.end()) {
        // add new word
        int id = ids_.size();
        ids_.insert(make_pair(word, id));
        rev_.push_back(word);
    }
}

int Dictionary::getId(const string & word) const {
    map<string, int>::const_iterator it = ids_.find(word);
    return it == ids_.end() ? -1 : it->second;
}

string Dictionary::getWord(int id) const {
    if (id < 0 || id >= static_cast<int>(rev_.size())) {
        stringstream ss;
        ss << "Dictionary: invalid word ID: " << id;
        throw out_of_range(ss.str());
    }
    return rev_[id];
}

} // namespace AHCParser

