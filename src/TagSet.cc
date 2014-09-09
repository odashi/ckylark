#include "TagSet.h"

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>

using namespace std;

namespace AHCParser {

TagSet::TagSet()
    : depth_(0)
    , tree_list_()
    , num_subtags_() {
}

TagSet::~TagSet() {
    // delete all trees
    for (Tree<int> * tree : tree_list_) {
        delete tree;
    }
}

shared_ptr<TagSet> TagSet::loadFromStream(istream & stream) {
    typedef boost::char_separator<char> separator;
    typedef boost::tokenizer<separator> tokenizer;

    TagSet * tags = new TagSet();
    shared_ptr<TagSet> ptags(tags);

    string line;
    while (getline(stream, line)) {
        boost::trim(line);
        vector<string> ls;
        boost::split(ls, line, boost::is_any_of("\t"));
        tags->tag_table_.addWord(ls[0]);

        // tokenize S formula
        separator sep(" ", "()");
        tokenizer tokens(ls[1], sep);
        vector<string> tok;
        copy(tokens.begin(), tokens.end(), back_inserter(tok));

        // make tree
        int pos = 0;
        tags->tree_list_.push_back(makeSubtagTree(tok, pos));
    }

    tags->checkDepth();

    // add ROOT tag
    tags->tag_table_.addWord("ROOT");
    Tree<int> * node = new Tree<int>(0);
    for (size_t i = 1; i < tags->depth_; ++i) {
        Tree<int> * parent = new Tree<int>(0);
        parent->addChild(node);
        node = parent;
    }
    tags->tree_list_.push_back(node);

    // calculate #subtags
    tags->num_subtags_.assign(tags->numTags(), vector<size_t>(tags->getDepth(), -1));

    function<size_t(Tree<int> &, int)> numSubtagsByNode
        = [&](Tree<int> & node, int rest_depth) -> size_t {
        
        if (rest_depth == 0) return 1;
        size_t n = numSubtagsByNode(node.child(0), rest_depth - 1);
        if (node.numChildren() >= 2) n += numSubtagsByNode(node.child(1), rest_depth - 1);
        return n;
    };

    for (size_t tag = 0; tag < tags->numTags(); ++tag) {
        for (size_t level = 0; level <  tags->getDepth(); ++level) {
            tags->num_subtags_[tag][level] = numSubtagsByNode(*(tags->tree_list_[tag]), level);
        }
    }

    return ptags;
}

Tree<int> * TagSet::makeSubtagTree(const vector<string> & tok, int & pos) {
    Tree<int> * node = nullptr;
    try {
        if (tok[pos] == "(") {
            // branch
            ++pos;
            node = new Tree<int>(stoi(tok[pos]));
            ++pos;
            node->addChild(makeSubtagTree(tok, pos));
            ++pos;
            if (tok[pos] != ")") {
                node->addChild(makeSubtagTree(tok, pos));
                ++pos;
            }
        } else {
            // leaf
            node = new Tree<int>(stoi(tok[pos]));
        }
    } catch (...) {
        delete node;
        throw;
    }
    return node;
}

void TagSet::checkDepth() {
    size_t d = getDepthByNode(*(tree_list_[0]));
    for (auto root : tree_list_) {
        size_t dd  = getDepthByNode(*root);
        if (dd != d) {
            throw runtime_error("TagSet: invalid file format");
        }
    }
    depth_ = d;
}

size_t TagSet::getDepthByNode(Tree<int> & node) {
    if (node.isLeaf()) {
        return 1;
    }
    size_t d = getDepthByNode(node.child(0));
    if (node.numChildren() >= 2) {
        size_t dd = getDepthByNode(node.child(1));
        if (dd != d) {
            throw runtime_error("TagSet: invalid file format");
        }
    }
    return d + 1;
}

} // namespace AHCParser

