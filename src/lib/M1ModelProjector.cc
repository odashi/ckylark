#include <ckylark/M1ModelProjector.h>

using namespace std;

namespace Ckylark {

M1ModelProjector::M1ModelProjector(
    const Dictionary & word_table,
    const TagSet & tag_set,
    const Lexicon & g0_lexicon,
    const Grammar & g0_grammar)
    : word_table_(word_table)
    , tag_set_(tag_set)
    , lexicon_(g0_lexicon)
    , grammar_(g0_grammar) {

    int num_tags = tag_set_.numTags();
    int root_pos = tag_set_.getTagId("ROOT");

    // calculate transition probabilities

    vector<vector<double> > transition_prob(num_tags, vector<double>(num_tags, 0.0));

    for (int ptag = 0; ptag < num_tags; ++ptag) {
        for (const BinaryRule * rule : grammar_.getBinaryRuleList(ptag)) {
            double score = rule->getScore(0, 0, 0);
            transition_prob[ptag][rule->left()] += score;
            transition_prob[ptag][rule->right()] += score;
        }
    }

    for (auto & it1 : grammar_.getUnaryRuleListByPC()) {
        for (const UnaryRule * rule : it1) {
            transition_prob[rule->parent()][rule->child()] += rule->getScore(0, 0);
        }
    }

    // calculate expected counts

    vector<double> exp_count(num_tags, 0.0);
    vector<double> temp_count(num_tags, 0.0);
    exp_count[root_pos] = 1.0;
    temp_count[root_pos] = 1.0;
    const int NUM_ITERATION = 50;

    for (int iteration = 0; iteration < NUM_ITERATION; ++iteration) {
        for (int i = 0; i < num_tags; ++i) {
            for (int j = 0; j < num_tags; ++j) {
                temp_count[i] += transition_prob[j][i] * exp_count[j];
            }
        }

        for (int i = 0; i < num_tags; ++i) {
            exp_count[i] = temp_count[i];
            temp_count[i] = 0.0;
        }
        exp_count[root_pos] = 1.0;
        temp_count[root_pos] = 1.0;
    }

    // calculate conditional probabilities

    cond_prob_.assign(num_tags, 0.0);
    vector<double> total(num_tags, 0.0);
    
    for (int tag = 0; tag < num_tags; ++tag) {
        int map_to = lexicon_.hasEntry(tag) ? tag : root_pos;
        total[map_to] += exp_count[tag];
    }

    for (int tag = 0; tag < num_tags; ++tag) {
        int map_to = lexicon_.hasEntry(tag) ? tag : root_pos;
        cond_prob_[tag] = exp_count[tag] / total[map_to];
    }
}

M1ModelProjector::~M1ModelProjector() {}

shared_ptr<M1Lexicon> M1ModelProjector::generateLexicon() const {
    M1Lexicon * lex = new M1Lexicon(word_table_, tag_set_);
    shared_ptr<M1Lexicon> plex(lex);

    for (auto & it1 : lexicon_.getEntryList()) {
        for (auto & it2 : it1) {
            // NOTE: cond_prob_[tag_id] == 1.0
            const LexiconEntry & ent = *(it2.second);
            lex->addScore(ent.tagId(), ent.wordId(), ent.getScore(0));
        }
    }

    return plex;
}

shared_ptr<M1Grammar> M1ModelProjector::generateGrammar() const {
    M1Grammar * grm = new M1Grammar(tag_set_);
    shared_ptr<M1Grammar> pgrm(grm);
    int num_tags = tag_set_.numTags();
    int root_pos = tag_set_.getTagId("ROOT");

    // NOTE: rule->parent() is NOT a lexicon tag,
    //       and is mapped always ROOT.

    for (int ptag = 0; ptag < num_tags; ++ptag) {
        for (const BinaryRule * rule : grammar_.getBinaryRuleList(ptag)) {
            int ltag_to = lexicon_.hasEntry(rule->left()) ? rule->left() : root_pos;
            int rtag_to = lexicon_.hasEntry(rule->right()) ? rule->right() : root_pos;
            double delta = cond_prob_[ptag] * rule->getScore(0, 0, 0);
            grm->addBinaryScore(ltag_to, rtag_to, delta);
        }
    }

    for (auto & it1 : grammar_.getUnaryRuleListByPC()) {
        for (const UnaryRule * rule : it1) {
            int ctag_to = lexicon_.hasEntry(rule->child()) ? rule->child() : root_pos;
            double delta = cond_prob_[rule->parent()] * rule->getScore(0, 0);
            grm->addUnaryScore(ctag_to, delta);
        }
    }

    return pgrm;
}

} // namespace Ckylark

