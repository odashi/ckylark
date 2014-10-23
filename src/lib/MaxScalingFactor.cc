#include "MaxScalingFactor.h"

#include <cmath>

using namespace std;

namespace Ckylark {

MaxScalingFactor::MaxScalingFactor() {}

MaxScalingFactor::~MaxScalingFactor() {}

double MaxScalingFactor::calculate(const Lexicon & lexicon, const Grammar & grammar) {

    // factor = exp -E_{X,Y,Z}[ max_{i,j,k} log P(X_i -> Y_j Z_k) ]

    int num_tags = grammar.getTagSet().numTags();
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

    return exp(-log_prod / static_cast<double>(n));
}

} // namespace Ckylark

