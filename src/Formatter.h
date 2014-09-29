#ifndef CKYLARK_FORMATTER_H_
#define CKYLARK_FORMATTER_H_

#include "Tree.h"

#include <string>

namespace Ckylark {

class Formatter {

    Formatter() = delete;
    Formatter(const Formatter &) = delete;
    Formatter & operator=(const Formatter &) = delete;

public:
    static std::string ToPennTreeBank(
        const Tree<std::string> & parse,
        bool add_root_tag);

private:
    static std::string escapeForPennTreeBank(const std::string & raw);

}; // class Formatter

} // namespace Ckylark

#endif // CKYLARK_FORMATTER_H_

