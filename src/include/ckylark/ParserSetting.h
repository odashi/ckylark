#ifndef CKYLARK_PARSER_SETTING_H_
#define CKYLARK_PARSER_SETTING_H_

namespace Ckylark {

// configuration parameters for parser
struct ParserSetting {

    // do partial parsing
    bool partial;

    // generate only unary and binary rules.
    bool binarize;

}; // struct ParserSetting

} // namespace Ckylark

#endif // CKYLARK_PARSER_SETTING_H_

