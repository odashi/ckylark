#ifndef CKYLARK_S_EXPR_FORMATTER_H_
#define CKYLARK_S_EXPR_FORMATTER_H_

#include <ckylark/Formatter.h>

namespace Ckylark {

// S-expression (Penn Treebank) formatted output generator
class SExprFormatter : public Formatter {

    SExprFormatter(const SExprFormatter &) = delete;
    SExprFormatter & operator=(const SExprFormatter &) = delete;

public:
    SExprFormatter(bool add_root_tag);
    ~SExprFormatter();

    std::string generate(const Tree<std::string> & parse) const;

private:
    bool add_root_tag_;

    std::string escape(const std::string & raw) const;

}; // class Formatter

} // namespace Ckylark

#endif // CKYLARK_S_EXPR_FORMATTER_H_

