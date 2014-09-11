#include "Formatter.h"

using namespace std;

namespace AHCParser {

string Formatter::ToPennTreeBank(const Tree<string> & parse) {
    if (parse.isLeaf()) {
        string tag = escapeForPennTreeBank(parse.value());
        if (!tag.empty()) {
            return tag;
        }
        return "()";
    }

    string repr = "(" + parse.value();
    int nc = parse.numChildren();
    for (int i = 0; i < nc; ++i) {
        repr += " " + ToPennTreeBank(parse.child(i));
    }
    repr += ")";
    return repr;
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

} // namespace AHCParser
