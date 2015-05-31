#ifndef CKYLARK_MAX_SCALING_FACTOR_H_
#define CKYLARK_MAX_SCALING_FACTOR_H_

#include <ckylark/ScalingFactor.h>
#include <ckylark/Dictionary.h>
#include <ckylark/TagSet.h>
#include <ckylark/Lexicon.h>
#include <ckylark/Grammar.h>

#include <vector>

namespace Ckylark {

class MaxScalingFactor : public ScalingFactor {

    MaxScalingFactor(const MaxScalingFactor &) = delete;
    MaxScalingFactor & operator=(const MaxScalingFactor &) = delete;

public:
    MaxScalingFactor(
        const Dictionary & word_table,
        const TagSet & tag_set,
        const Lexicon & lexicon,
        const Grammar & grammar,
        double lexicon_smoothing_factor);
    ~MaxScalingFactor() {}

    double getLexiconScalingFactor(int word_id) const { return lexicon_factor_.at(word_id); }
    double getGrammarScalingFactor() const { return grammar_factor_; }

private:
    std::vector<double> lexicon_factor_;
    double grammar_factor_;

}; // class MaxScalingFactor

} // namespace Ckylark

#endif // CKYLARK_MAX_SCALING_FACTOR_H_

