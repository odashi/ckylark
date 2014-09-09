#ifndef AHCP_RULE_H_
#define AHCP_RULE_H_

#include <vector>

namespace AHCParser {

class BinaryRule {

public:
    BinaryRule(int parent, int left, int right, size_t nsub_parent, size_t nsub_left, size_t nsub_right)
        : parent_(parent)
        , left_(left)
        , right_(right)
        , nsub_parent_(nsub_parent)
        , nsub_left_(nsub_left)
        , nsub_right_(nsub_right)
        , score_(nsub_parent, std::vector<std::vector<double> >(nsub_left)) {
    }

    ~BinaryRule() {}

    inline int parent() const { return parent_; }
    inline int left() const { return left_; }
    inline int right() const { return right_; }
    inline size_t numParentSubtags() const { return nsub_parent_; }
    inline size_t numLeftSubtags() const { return nsub_left_; }
    inline size_t numRightSubtags() const { return nsub_right_; }

    inline double getScore(int sub_parent, int sub_left, int sub_right) const {
        if (score_[sub_parent][sub_left].empty()) return 0.0;
        return score_[sub_parent][sub_left][sub_right];
    }

    // [sub_parent][sub_left or empty][sub_right or empty]
    inline const std::vector<std::vector<std::vector<double> > > & getScoreList() const { return score_; }

    inline void setScore(int sub_parent, int sub_left, int sub_right, double value) {
        if (score_[sub_parent][sub_left].empty()) {
            score_[sub_parent][sub_left].assign(nsub_right_, 0.0);
        }
        score_[sub_parent][sub_left][sub_right] = value;
    }

    inline void addScore(int sub_parent, int sub_left, int sub_right, double delta) {
        if (score_[sub_parent][sub_left].empty()) {
            score_[sub_parent][sub_left].assign(nsub_right_, 0.0);
        }
        score_[sub_parent][sub_left][sub_right] += delta;
    }

private:
    int parent_;
    int left_;
    int right_;
    size_t nsub_parent_;
    size_t nsub_left_;
    size_t nsub_right_;
    std::vector<std::vector<std::vector<double> > > score_;
    
}; // class BinaryRule

class UnaryRule {

public:
    UnaryRule(int parent, int child, size_t nsub_parent, size_t nsub_child)
        : parent_(parent)
        , child_(child)
        , nsub_parent_(nsub_parent)
        , nsub_child_(nsub_child)
        , score_(nsub_parent, std::vector<double>(nsub_child, 0.0)) {
    }

    ~UnaryRule() {}

    inline int parent() const { return parent_; }
    inline int child() const { return child_; }
    inline size_t numParentSubtags() const { return nsub_parent_; }
    inline size_t numChildSubtags() const { return nsub_child_; }

    inline double getScore(int sub_parent, int sub_child) const {
        return score_[sub_parent][sub_child];
    }

    // [sub_parent][sub_left]
    inline const std::vector<std::vector<double> > & getScoreList() const { return score_; }

    inline void setScore(int sub_parent, int sub_child, double value) {
        score_[sub_parent][sub_child] = value;
    }

    inline void addScore(int sub_parent, int sub_child, double delta) {
        score_[sub_parent][sub_child] += delta;
    }

private:
    int parent_;
    int child_;
    size_t nsub_parent_;
    size_t nsub_child_;
    std::vector<std::vector<double> > score_;

}; // class UnaryRule



} // namespace AHCParser

#endif // AHCP_RULE_H_

