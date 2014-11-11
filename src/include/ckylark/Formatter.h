#ifndef CKYLARK_FORMATTER_H_
#define CKYLARK_FORMATTER_H_

#include <ckylark/Tree.h>

#include <string>

namespace Ckylark {

// abstract class for output string generator
class Formatter {

    Formatter(const Formatter &) = delete;
    Formatter & operator=(const Formatter &) = delete;

public:
    Formatter() {}
    virtual ~Formatter() {}

    // generate output string from a parse tree
    virtual std::string generate(const Tree<std::string> & parse) const = 0;

}; // class Formatter

} // namespace Ckylark

#endif // CKYLARK_FORMATTER_H_

