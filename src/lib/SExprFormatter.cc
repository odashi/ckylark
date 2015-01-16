#include <ckylark/SExprFormatter.h>

#include <iostream>

using namespace std;

namespace Ckylark {

SExprFormatter::SExprFormatter(bool add_root_tag)
    : add_root_tag_(add_root_tag) {
}

SExprFormatter::~SExprFormatter() {}

string SExprFormatter::generate(const Tree<string> & parse) const {

    string tag = escape(parse.value());

    if (parse.isLeaf()) {
        if (tag.empty()) {
            return parse.isRoot() ? "(())" : "()";
        } else {
            return tag;
        }
    } else {
        int nc = parse.numChildren();
        string child = "";
        for (int i = 0; i < nc; ++i) {
            child += " " + generate(parse.child(i));
        }
        if (parse.isRoot()) {
            tag = add_root_tag_ ? tag : "";
            return "(" + tag + child + " )";
        } else {
            return "(" + tag + child + ")";
        }
    }
}

string SExprFormatter::escape(const string & raw) const {
    string escaped = "";
    for (char c : raw) {
        switch (c) {
            case '(': escaped += "-LRB-"; break;
            case ')': escaped += "-RRB-"; break;
            default: escaped += c;
        }
    }
    return escaped;
}

} // namespace Ckylark
