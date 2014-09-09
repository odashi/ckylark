#include "Formatter.h"

using namespace std;

namespace AHCParser {

string Formatter::ToPennTreeBank(const Tree<string> & parse) {
    if (parse.isLeaf()) {
        string tag = parse.value();
        if (!tag.empty()) {
            return parse.value();
        }
        return "()";
    }

    string ret = "(" + parse.value() + " " + ToPennTreeBank(parse.child(0));
    if (parse.numChildren() >= 2) {
        ret += " " + ToPennTreeBank(parse.child(1));
    }
    ret += ")";
    return ret;
}

} // namespace AHCParser
