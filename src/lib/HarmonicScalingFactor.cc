#include <ckylark/HarmonicScalingFactor.h>

#include <ckylark/OOVLexiconSmoother.h>
#include <ckylark/Mapping.h>

#include <cmath>
#include <stdexcept>
#include <vector>

#include <iostream>

using namespace std;

namespace Ckylark {

HarmonicScalingFactor::HarmonicScalingFactor(
    const Dictionary & word_table,
    const TagSet & tag_set,
    const Lexicon & lexicon,
    const Grammar & grammar,
    double lexicon_smoothing_factor)
    : lexicon_factor_(word_table.size(), 1.0) {

    // check lexicon/grammar levels
    int level = lexicon.getLevel();
    if (level != grammar.getLevel()) {
        throw runtime_error("HarmonicScalingFactor::HarmonicScalingFactor(): lexicon and grammar levels are mismatched.");
    }

    int num_words = word_table.size();
    int num_tags = tag_set.numTags();

    Mapping mapping(tag_set, 0, level);
    int num_fine_pos = mapping.getNumFinePos();
    int root_pos = mapping.getFinePos(tag_set.getTagId("ROOT"), 0);

    // calculate transition probabilities

    vector<vector<double> > transition_prob(num_fine_pos, vector<double>(num_fine_pos, 0.0));

    for (int ptag = 0; ptag < num_tags; ++ptag) {
        for (const BinaryRule * rule : grammar.getBinaryRuleList(ptag)) {
            auto & score_list = rule->getScoreList();
            int psc = tag_set.numSubtags(rule->parent(), level);
            int lsc = tag_set.numSubtags(rule->left(), level);
            int rsc = tag_set.numSubtags(rule->right(), level);
            for (int p = 0; p < psc; ++p) {
                auto & score_list_p = score_list[p];
                if (score_list_p.empty()) continue;
                int pm = mapping.getFinePos(rule->parent(), p);
                for (int l = 0; l < lsc; ++l) {
                    auto & score_list_pl = score_list_p[l];
                    if (score_list_pl.empty()) continue;
                    int lm = mapping.getFinePos(rule->left(), l);
                    for (int r = 0; r < rsc; ++r) {
                        int rm = mapping.getFinePos(rule->right(), r);
                        double score = score_list_pl[r];
                        transition_prob[pm][lm] += score;
                        transition_prob[pm][rm] += score;
                    }
                }
            }
        }
    }

    for (auto & it1 : grammar.getUnaryRuleListByPC()) {
        for (const UnaryRule * rule : it1) {
            auto & score_list = rule->getScoreList();
            int psc = tag_set.numSubtags(rule->parent(), level);
            int csc = tag_set.numSubtags(rule->child(), level);
            for (int p = 0; p < psc; ++p) {
                auto & score_list_p = score_list[p];
                if (score_list_p.empty()) continue;
                int pm = mapping.getFinePos(rule->parent(), p);
                for (int c = 0; c < csc; ++c) {
                    int cm = mapping.getFinePos(rule->child(), c);
                    transition_prob[pm][cm] += score_list_p[c];
                }
            }
        }
    }

    // calculate expected counts

    vector<double> exp_count(num_fine_pos, 0.0);
    vector<double> temp_count(num_fine_pos, 0.0);
    exp_count[root_pos] = 1.0;
    temp_count[root_pos] = 1.0;
    const int NUM_ITERATION = 50;

    for (int iteration = 0; iteration < NUM_ITERATION; ++iteration) {
        for (int i = 0; i < num_fine_pos; ++i) {
            for (int j = 0; j < num_fine_pos; ++j) {
                temp_count[i] += transition_prob[j][i] * exp_count[j];
            }
        }

        for (int i = 0; i < num_fine_pos; ++i) {
            exp_count[i] = temp_count[i];
            temp_count[i] = 0.0;
        }
        exp_count[root_pos] = 1.0;
        temp_count[root_pos] = 1.0;
    }

    // calculate conditional probabilities

    double semiterminal_total = 0.0;
    double inner_total = 0.0;

    for (int tag = 0; tag < num_tags; ++tag) {
        int num_sub = tag_set.numSubtags(tag, level);
        if (lexicon.hasEntry(tag)) {
            for (int sub = 0; sub < num_sub; ++sub) {
                semiterminal_total += exp_count[mapping.getFinePos(tag, sub)];
            }
        } else {
            for (int sub = 0; sub < num_sub; ++sub) {
                inner_total += exp_count[mapping.getFinePos(tag, sub)];
            }
        }
    }
    
    vector<double> inner_cond_prob(num_fine_pos, 0.0);
    vector<double> semiterminal_cond_prob(num_fine_pos, 0.0);

    for (int tag = 0; tag < num_tags; ++tag) {
        int num_sub = tag_set.numSubtags(tag, level);
        if (lexicon.hasEntry(tag)) {
            for (int sub = 0; sub < num_sub; ++sub) {
                int i = mapping.getFinePos(tag, sub);
                semiterminal_cond_prob[i] = exp_count[i] / semiterminal_total;
            }
        } else {
            for (int sub = 0; sub < num_sub; ++sub) {
                int i = mapping.getFinePos(tag, sub);
                inner_cond_prob[i] = exp_count[i] / inner_total;
            }
        }
    }
    
    {
        // calculate lexicon scaling factors
        // factor(w) = 1.0 / sum_X[ P(X) P(X -> w) ]
        OOVLexiconSmoother smoother(lexicon, word_table, lexicon_smoothing_factor);

        for (int w = 0; w < num_words; ++w) {
            double sum = 0.0;

            for (int tag = 0; tag < num_tags; ++tag) {
                if (!smoother.prepare(tag, w)) continue;
                int num_sub = tag_set.numSubtags(tag, level);

                for (int sub = 0; sub < num_sub; ++sub) {
                    double score = smoother.getScore(sub);
                    if (score == 0.0) continue;
                    sum += semiterminal_cond_prob[mapping.getFinePos(tag, sub)] * score;
                }
            }
            
            lexicon_factor_[w] = 1.0 / sum;
        }
    }

    {
        // calculate grammar scaling factor
        // factor = 1.0 / sum_X[ P(X) exp[ H(X) ] ]
        // H(X) = sum_Y,Z[ P_bin(X -> Y Z) log P(X -> Y Z) ]
        // P_bin(X -> Y Z) = P(X -> Y Z) / sum_Y',Z'[ P(X -> Y' Z') ]
        vector<double> entropy(num_fine_pos, 0.0);
        vector<double> sumprob(num_fine_pos, 0.0);

        for (int ptag = 0; ptag < num_tags; ++ptag) {
            for (const BinaryRule * rule : grammar.getBinaryRuleList(ptag)) {
                auto & score_list = rule->getScoreList();
                int psc = tag_set.numSubtags(rule->parent(), level);
                int lsc = tag_set.numSubtags(rule->left(), level);
                int rsc = tag_set.numSubtags(rule->right(), level);
                for (int p = 0; p < psc; ++p) {
                    auto & score_list_p = score_list[p];
                    if (score_list_p.empty()) continue;
                    int pm = mapping.getFinePos(rule->parent(), p);
                    for (int l = 0; l < lsc; ++l) {
                        auto & score_list_pl = score_list_p[l];
                        if (score_list_pl.empty()) continue;
                        for (int r = 0; r < rsc; ++r) {
                            double score = score_list_pl[r];
                            if (score > 0.0) {
                                entropy[pm] += score * log(score);
                                sumprob[pm] += score;
                            }
                        }
                    }
                }
            }
        }

        double e_prob = 0.0;

        for (int i = 0; i < num_fine_pos; ++i) {
            if (sumprob[i] > 0.0) {
                e_prob += inner_cond_prob[i] * exp(entropy[i] / sumprob[i]);
            }
        }

        grammar_factor_ = 1.0 / e_prob;
    }
}

} // namespace Ckylark

