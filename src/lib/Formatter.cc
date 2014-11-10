#include <ckylark/Formatter.h>

#include <iostream>

using namespace std;

namespace Ckylark {

string Formatter::ToPennTreeBank(
    const Tree<string> & parse,
    bool add_root_tag) {

    string tag = escapeForPennTreeBank(parse.value());

    if (parse.isLeaf()) {
        return tag.empty() ? "()" : tag;
    } else if (tag == "ROOT") {
        string child = ToPennTreeBank(parse.child(0), true);
        return (add_root_tag ? "(ROOT " : "( ") + child + " )";
    } else {
        int nc = parse.numChildren();
        string child = "";
        for (int i = 0; i < nc; ++i) {
            child += " " + ToPennTreeBank(parse.child(i), true);
        }
        return "(" + tag + child + ")";
    }
}

string Formatter::escapeForPennTreeBank(const string & raw) {
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
