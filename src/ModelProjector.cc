#include "ModelProjector.h"

//#include <cstdio>

using namespace std;

namespace Ckylark {

ModelProjector::ModelProjector(
    const TagSet & tag_set,
    const Lexicon & lexicon,
    const Grammar & grammar,
    int fine_level,
    int coarse_level)
    : tag_set_(tag_set)
    , lexicon_(lexicon)
    , grammar_(grammar)
    , fine_level_(fine_level)
    , coarse_level_(coarse_level)
    , mapping_(tag_set, coarse_level, fine_level) {

    if (lexicon_.getLevel() != fine_level_) throw runtime_error("ModelProjector: lexicon level is mismatched");
    if (grammar_.getLevel() != fine_level_) throw runtime_error("ModelProjector: grammar level is mismatched");

    int num_tags = tag_set.numTags();
    int num_fine_pos = mapping_.getNumFinePos();
    int num_coarse_pos = mapping_.getNumCoarsePos();
    int root_pos = mapping_.getFinePos(tag_set_.getTagId("ROOT"), 0);

    //  calculate transition probabilities

    vector<vector<double> > transition_prob(num_fine_pos, vector<double>(num_fine_pos, 0.0));
    
    for (int ptag = 0; ptag < num_tags; ++ptag) {
        for (const BinaryRule * rule : grammar_.getBinaryRuleList(ptag)) {
            auto & score_list = rule->getScoreList();
            int psc = tag_set_.numSubtags(rule->parent(), fine_level_);
            int lsc = tag_set_.numSubtags(rule->left(), fine_level_);
            int rsc = tag_set_.numSubtags(rule->right(), fine_level_);
            for (int p = 0; p < psc; ++p) {
                auto & score_list_p = score_list[p];
                if (score_list_p.empty()) continue;
                int pm = mapping_.getFinePos(rule->parent(), p);
                for (int l = 0; l < lsc; ++l) {
                    auto & score_list_pl = score_list_p[l];
                    if (score_list_pl.empty()) continue;
                    int lm = mapping_.getFinePos(rule->left(), l);
                    for (int r = 0; r < rsc; ++r) {
                        int rm = mapping_.getFinePos(rule->right(), r);
                        double score = score_list_pl[r];
                        transition_prob[pm][lm] += score;
                        transition_prob[pm][rm] += score;
                    }
                }
            }
        }
    }

    for (auto & it1 : grammar_.getUnaryRuleListByPC()) {
        for (const UnaryRule * rule : it1) {
            auto & score_list = rule->getScoreList();
            int psc = tag_set_.numSubtags(rule->parent(), fine_level_);
            int csc = tag_set_.numSubtags(rule->child(), fine_level_);
            for (int p = 0; p < psc; ++p) {
                auto & score_list_p = score_list[p];
                if (score_list_p.empty()) continue;
                int pm = mapping_.getFinePos(rule->parent(), p);
                for (int c = 0; c < csc; ++c) {
                    int cm = mapping_.getFinePos(rule->child(), c);
                    transition_prob[pm][cm] += score_list_p[c];
                }
            }
        }
    }

    // calcluate expected counts

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

        double sum = 0.0;
        for (int i = 0; i < num_fine_pos; ++i) {
            sum += temp_count[i];
            exp_count[i] = temp_count[i];
            temp_count[i] = 0.0;
        }
        exp_count[root_pos] = 1.0;
        temp_count[root_pos] = 1.0;

        //printf("iteration %d: sum=%f\n", iteration+1, sum);
    }

    // calculate conditional probabilities

    cond_prob_.assign(num_fine_pos, 0.0);
    vector<double> total(num_coarse_pos, 0.0);
   
    for (int tag = 0; tag < num_tags; ++tag) {
        int fine_num_subtags = tag_set_.numSubtags(tag, fine_level_);
        for (int fine_subtag = 0; fine_subtag < fine_num_subtags; ++fine_subtag) {
            int coarse_subtag = mapping_.getFineToCoarseMap(tag, fine_subtag);
            int fine_pos = mapping_.getFinePos(tag, fine_subtag);
            int coarse_pos = mapping_.getCoarsePos(tag, coarse_subtag);
            total[coarse_pos] += exp_count[fine_pos];
        }
    }

    for (int tag = 0; tag < num_tags; ++tag) {
        int fine_num_subtags = tag_set_.numSubtags(tag, fine_level_);
        for (int fine_subtag = 0; fine_subtag < fine_num_subtags; ++fine_subtag) {
            int coarse_subtag = mapping_.getFineToCoarseMap(tag, fine_subtag);
            int fine_pos = mapping_.getFinePos(tag, fine_subtag);
            int coarse_pos = mapping_.getCoarsePos(tag, coarse_subtag);
            cond_prob_[fine_pos] = exp_count[fine_pos] / total[coarse_pos];
        }
    }

    //int a = tag_set_.getTagId("DT");
    //for (int i = 0; i < tag_set_.numSubtags(a, fine_level_); ++i) {
    //    printf("P(%d|group)=%f\n", i, cond_prob_[mapping_.getFinePos(a, i)]);
    //}
}

ModelProjector::~ModelProjector() {}

shared_ptr<Lexicon> ModelProjector::generateLexicon() const {
    Lexicon * lex = new Lexicon(tag_set_, coarse_level_);
    shared_ptr<Lexicon> plex(lex);
    
    for (auto & it1 : lexicon_.getEntryList()) {
        for (auto & it2 : it1) {
            const LexiconEntry & ent = *(it2.second);
            int pc = ent.tagId();
            int npsc = ent.numSubtags();
            LexiconEntry & new_ent = lex->getEntryOrCreate(pc, ent.wordId());
            for (int psc = 0; psc < npsc; ++psc) {
                double delta = cond_prob_[mapping_.getFinePos(pc, psc)] * ent.getScore(psc);
                new_ent.addScore(mapping_.getFineToCoarseMap(pc, psc), delta);
            }
        }
    }
    
    return plex;
}

shared_ptr<Grammar> ModelProjector::generateGrammar() const {
    Grammar * grm = new Grammar(tag_set_, coarse_level_);
    shared_ptr<Grammar> pgrm(grm);
    int num_tags = tag_set_.numTags();
    
    for (int ptag = 0; ptag < num_tags; ++ptag) {
        for (const BinaryRule * rule : grammar_.getBinaryRuleList(ptag)) {
            BinaryRule & new_rule = grm->getBinaryRule(rule->parent(), rule->left(), rule->right());
            auto & score_list = rule->getScoreList();
            int npsc = rule->numParentSubtags();
            for (int psc = 0; psc < npsc; ++psc) {
                auto & score_list_p = score_list[psc];
                if (score_list_p.empty()) continue;
                int nlsc = rule->numLeftSubtags();
                for (int lsc = 0; lsc < nlsc; ++lsc) {
                    auto & score_list_pl = score_list_p[lsc];
                    if (score_list_pl.empty()) continue;
                    int nrsc = rule->numRightSubtags();
                    for (int rsc = 0; rsc < nrsc; ++rsc) {
                        int new_psc = mapping_.getFineToCoarseMap(rule->parent(), psc);
                        int new_lsc = mapping_.getFineToCoarseMap(rule->left(), lsc);
                        int new_rsc = mapping_.getFineToCoarseMap(rule->right(), rsc);
                        double delta = cond_prob_[mapping_.getFinePos(rule->parent(), psc)] * score_list_pl[rsc];
                        new_rule.addScore(new_psc, new_lsc, new_rsc, delta);
                    }
                }
            }
        }
    }

    for (auto & it1 : grammar_.getUnaryRuleListByPC()) {
        for (const UnaryRule * rule : it1) {
            UnaryRule & new_rule = grm->getUnaryRule(rule->parent(), rule->child());
            auto & score_list = rule->getScoreList();
            int npsc = rule->numParentSubtags();
            for (int psc = 0; psc < npsc; ++psc) {
                auto & score_list_p = score_list[psc];
                if (score_list_p.empty()) continue;
                int ncsc = rule->numChildSubtags();
                for (int csc = 0; csc < ncsc; ++csc) {
                    int new_psc = mapping_.getFineToCoarseMap(rule->parent(), psc);
                    int new_csc = mapping_.getFineToCoarseMap(rule->child(), csc);
                    double delta = cond_prob_[mapping_.getFinePos(rule->parent(), psc)] * score_list_p[csc];
                    new_rule.addScore(new_psc, new_csc, delta);
                }
            }
        }
    }

    return pgrm;
}

} // namespace Ckylark

