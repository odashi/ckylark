#ifndef CKYLARK_PARSER_RESULT_H_
#define CKYLARK_PARSER_RESULT_H_

#include "Tree.h"

#include <memory>
#include <string>

namespace Ckylark {

struct ParserResult {
    std::shared_ptr<Tree<std::string> > best_parse; // best parse tree
    bool succeeded; // true if the parser finished successfully, false otherwise.
    int final_level; // grammar level which parser tried finally, -1 if parser did not try CKY parse.
}; // class ParserResult

} // namespace Ckylark

#endif // CKYLARK_PARSER_RESULT_H_

