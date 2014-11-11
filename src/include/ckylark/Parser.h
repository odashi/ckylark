#ifndef CKYLARK_PARSER_H_
#define CKYLARK_PARSER_H_

#include <ckylark/ParserResult.h>
#include <ckylark/ParserSetting.h>

#include <vector>
#include <string>

namespace Ckylark {

// interface of any parser classes
class Parser {

    Parser(const Parser &) = delete;
    Parser & operator=(const Parser &) = delete;

public:
    Parser() {}
    virtual ~Parser() {}

    // generate best 1-parse
    virtual ParserResult parse(
        const std::vector<std::string> & sentence,
        const ParserSetting & setting) const = 0;

}; // class Parser

} // namespace Ckylark

#endif // CKYLARK_PARSER_H_

