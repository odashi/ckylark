#ifndef CKYLARK_GRAMMAR_H_
#define CKYLARK_GRAMMAR_H_

#include "TagSet.h"
#include "Dictionary.h"
#include "Rule.h"

#include <iostream>
#include <memory>
#include <vector>

namespace Ckylark {

class Grammar {

    Grammar() = delete;
    Grammar(const Grammar &) = delete;
    Grammar & operator=(const Grammar &) = delete;

public:
    Grammar(const TagSet & tag_set, int level);
    ~Grammar();

    static std::shared_ptr<Grammar> loadFromStream(std::istream & stream, const TagSet & tag_set);

    inline int getLevel() const { return level_; }

    const TagSet & getTagSet() const { return tag_set_; }

    BinaryRule & getBinaryRule(int parent, int left, int right);
    UnaryRule & getUnaryRule(int parent, int child);

    inline const std::vector<BinaryRule *> & getBinaryRuleList(int parent) const { return binary_parent_[parent]; }

    //inline const std::vector<std::vector<std::vector<BinaryRule *> > > & getBinaryRuleListByPLR() const { return binary_parent_left_; }
    //inline const std::vector<std::vector<std::vector<BinaryRule *> > > & getBinaryRuleListByPRL() const { return binary_parent_right_; }
    //inline const std::vector<std::vector<std::vector<BinaryRule *> > > & getBinaryRuleListByLRP() const { return binary_left_right_; }
    inline const std::vector<std::vector<UnaryRule *> > & getUnaryRuleListByPC() const { return unary_parent_; }
    inline const std::vector<std::vector<UnaryRule *> > & getUnaryRuleListByCP() const { return unary_child_; }

private:
    const TagSet & tag_set_;
    int level_;
    std::vector<std::vector<BinaryRule *> > binary_parent_; // [parent]{(left, right)}
    //std::vector<std::vector<std::vector<BinaryRule *> > > binary_parent_left_; // [parent][left]{right}
    //std::vector<std::vector<std::vector<BinaryRule *> > > binary_parent_right_; // [parent][right]{left}
    //std::vector<std::vector<std::vector<BinaryRule *> > > binary_left_right_; // [left][right]{parent}
    std::vector<std::vector<UnaryRule *> > unary_parent_; // [parent]{child}
    std::vector<std::vector<UnaryRule *> > unary_child_; // [child]{parent}

}; // class Grammar

} // namespace Ckylark

#endif // CKYLARK_GRAMMAR_H_

