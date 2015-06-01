#pragma once

#include <ckylark/ScalingFactor.h>
#include <ckylark/Dictionary.h>
#include <ckylark/TagSet.h>
#include <ckylark/Lexicon.h>
#include <ckylark/Grammar.h>

#include <vector>

namespace Ckylark {

class GeometricScalingFactor : public ScalingFactor {

    GeometricScalingFactor(const GeometricScalingFactor &) = delete;
    GeometricScalingFactor & operator=(const GeometricScalingFactor &) = delete;

public:
    GeometricScalingFactor(
        const Dictionary & word_table,
        const TagSet & tag_set,
        const Lexicon & lexicon,
        const Grammar & grammar,
        double lexicon_smoothing_factor);
    ~GeometricScalingFactor() {}

    double getLexiconScalingFactor(int word_id) const {
        return (word_id != -1) ? lexicon_factor_.at(word_id) : 1.0;
    }
    double getGrammarScalingFactor() const { return grammar_factor_; }

private:
    std::vector<double> lexicon_factor_;
    double grammar_factor_;

}; // class GeometricScalingFactor

} // namespace Ckylark

