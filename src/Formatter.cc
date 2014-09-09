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

    string repr = "(" + parse.value();
    int nc = parse.numChildren();
    for (int i = 0; i < nc; ++i) {
        repr += " " + ToPennTreeBank(parse.child(i));
    }
    repr += ")";
    return repr;
}

} // namespace AHCParser
