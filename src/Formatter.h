#ifndef AHCP_FORMATTER_H_
#define AHCP_FORMATTER_H_

#include "Tree.h"

#include <string>

namespace AHCParser {

class Formatter {

    Formatter() = delete;
    Formatter(const Formatter &) = delete;

public:
    static std::string ToPennTreeBank(const Tree<std::string> & parse);

private:
    static std::string escapeForPennTreeBank(const std::string & raw);

}; // class Formatter

} // namespace AHCParser

#endif // AHCP_FORMATTER_H_

