#pragma once

#include <ckylark/ScalingFactor.h>
#include <ckylark/Dictionary.h>
#include <ckylark/TagSet.h>
#include <ckylark/Lexicon.h>
#include <ckylark/Grammar.h>

#include <vector>

namespace Ckylark {

class HarmonicScalingFactor : public ScalingFactor {

    HarmonicScalingFactor(const HarmonicScalingFactor &) = delete;
    HarmonicScalingFactor & operator=(const HarmonicScalingFactor &) = delete;

public:
    HarmonicScalingFactor(
        const Dictionary & word_table,
        const TagSet & tag_set,
        const Lexicon & lexicon,
        const Grammar & grammar,
        double lexicon_smoothing_factor);
    ~HarmonicScalingFactor() {}
    
    double getLexiconScalingFactor(int word_id) const {
        return (word_id != -1) ? lexicon_factor_.at(word_id) : 1.0;
    }
    double getGrammarScalingFactor() const { return grammar_factor_; }

private:
    std::vector<double> lexicon_factor_;
    double grammar_factor_;

}; // class HarmonicScalingFactor

} // namespace Ckylark

