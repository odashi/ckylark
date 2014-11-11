#include <ckylark/POSTagFormatter.h>

using namespace std;

namespace Ckylark {

POSTagFormatter::POSTagFormatter(const string & separator)
    : separator_(separator) {
}

POSTagFormatter::~POSTagFormatter() {}

string POSTagFormatter::generate(const Tree<string> & parse) const {
    if (parse.isLeaf()) {
        if (parse.isRoot()) return "";
        else return parse.value() + separator_ + parse.parent().value();
    } else {
        string ret = generate(parse.child(0));
        int nc = parse.numChildren();
        for (int i = 1; i < nc; ++i) {
            ret += " " + generate(parse.child(i));
        }
        return ret;
    }
}

} // namespace Ckylark

