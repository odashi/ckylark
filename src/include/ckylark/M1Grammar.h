#ifndef CKYLARK_M1_GRAMMAR_H_
#define CKYLARK_M1_GRAMMAR_H_

#include <ckylark/TagSet.h>

#include <vector>

namespace Ckylark {

// G-1 Grammar
// This grammar has only below rules:
//   X -> X X
//   X -> X r-lex
//   X -> l-lex X
//   X -> l-lex r-lex
// X tag is indexed as ROOT tag of other grammar.
class M1Grammar {

    M1Grammar() = delete;
    M1Grammar(const M1Grammar &) = delete;
    M1Grammar & operator=(const M1Grammar &) = delete;

public:
    M1Grammar(const TagSet & tag_set)
        : binary_(tag_set.numTags(), std::vector<double>(tag_set.numTags(), 0.0))
        , unary_(tag_set.numTags(), 0.0) {}
    ~M1Grammar() {}

    double getBinaryScore(int left, int right) const {
        return binary_[left][right];
    }
    
    void addBinaryScore(int left, int right, double delta) {
        binary_[left][right] += delta;
    }

    double getUnaryScore(int child) const {
        return unary_[child];
    }

    void addUnaryScore(int child, double delta) {
        unary_[child] += delta;
    }

private:
    std::vector<std::vector<double> > binary_;
    std::vector<double> unary_;

}; // class M1Grammar

} // namespace Ckylark

#endif // CKYLARK_M1_GRAMMAR_H_

