#ifndef CKYLARK_M1_LEXICON_H_
#define CKYLARK_M1_LEXICON_H_

#include <ckylark/Dictionary.h>
#include <ckylark/TagSet.h>

#include <vector>

namespace Ckylark {

class M1Lexicon {

    M1Lexicon() = delete;
    M1Lexicon(const M1Lexicon &) = delete;
    M1Lexicon & operator=(const M1Lexicon &) = delete;

public:
    M1Lexicon(const Dictionary & word_table, const TagSet & tag_set)
        : score_(
            tag_set.numTags(), std::vector<double>(
                word_table.size(), 0.0)) {}
    ~M1Lexicon() {}

    double getScore(int tag_id, int word_id) const {
        return score_[tag_id][word_id];
    }
    
    void addScore(int tag_id, int word_id, double delta) {
        score_[tag_id][word_id] += delta;
    }

private:
    std::vector<std::vector<double> > score_;

}; // class M1Lexicon

} // namespace Ckylark

#endif // CKYLARK_M1_LEXICON_H_

