#ifndef CKYLARK_M1_GRAMMAR_H_
#define CKYLARK_M1_GRAMMAR_H_

#include <ckylark/TagSet.h>

#include <vector>

namespace Ckylark {

class M1Grammar {

    M1Grammar() = delete;
    M1Grammar(const M1Grammar &) = delete;
    M1Grammar & operator=(const M1Grammar &) = delete;

public:
    M1Grammar(const TagSet & tag_set)
        : score_(
            tag_set.numTags(), vector<vector<double> >(
                tag_set.numTags(), vector<double>(
                    tag_set.numTags(), 0.0))) {}
    ~M1Grammar() {}

    double getScore(int parent, int left, int right) const {
        return score_[parent][left][right];
    }
    
    void addScore(int parent, int left, int right, double delta) {
        score_[parent][left][right] += delta;
    }

private:
    std::vector<std::vector<std::vector<double> > > score_;

}; // class M1Grammar

} // namespace Ckylark

#endif // CKYLARK_M1_GRAMMAR_H_

