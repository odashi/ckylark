#include <ckylark/MaxScalingFactor.h>

#include <ckylark/OOVLexiconSmoother.h>

#include <cmath>
#include <stdexcept>

using namespace std;

namespace Ckylark {

MaxScalingFactor::MaxScalingFactor(
    const Dictionary & word_table,
    const TagSet & tag_set,
    const Lexicon & lexicon,
    const Grammar & grammar,
    double lexicon_smoothing_factor)
    : lexicon_factor_(word_table.size(), 1.0) {

    // check lexicon/grammar levels
    int level = lexicon.getLevel();
    if (level != grammar.getLevel()) {
        throw runtime_error("MaxScalingFactor::MaxScalingFactor(): lexicon and grammar levels are mismatched.");
    }

    int num_words = word_table.size();
    int num_tags = tag_set.numTags();

    {
        // calculate lexicon scaling factors
        // factor(w) = 1.0 / (max_i P(X_i -> w))
        OOVLexiconSmoother smoother(lexicon, word_table, lexicon_smoothing_factor);

        for (int w = 0; w < num_words; ++w) {
            double max_score = 0.0;

            for (int tag = 0; tag < num_tags; ++tag) {
                if (!smoother.prepare(tag, w)) continue;
                int num_sub = tag_set.numSubtags(tag, level);

                for (int sub = 0; sub < num_sub; ++sub) {
                    double score = smoother.getScore(sub);
                    if (score > max_score) {
                        max_score = score;
                    }
                }
            }

            if (max_score > 0.0) {
                lexicon_factor_[w] = 1.0 / max_score;
            }
        }
    }

    {
        // calculate grammar scaling factor
        // factor = exp -E_{X,Y,Z}[ max_{i,j,k} log P(X_i -> Y_j Z_k) ]
        int n = 0;
        double log_prod = 0.0;

        for (int ptag = 0; ptag < num_tags; ++ptag) {
            auto & rules_p = grammar.getBinaryRuleList(ptag);

            for (auto * rule : rules_p) {
                double max_score = 0.0;

                for (auto & scores_p : rule->getScoreList()) {
                    for (auto & scores_pl : scores_p) {
                        for (double score : scores_pl) {
                            if (score > max_score) {
                                max_score = score;
                            }
                        }
                    }
                }

                if (max_score > 0.0) {
                    ++n;
                    log_prod += log(max_score);
                }
            }
        }

        grammar_factor_ = exp(-log_prod / static_cast<double>(n));
    }
}

} // namespace Ckylark

