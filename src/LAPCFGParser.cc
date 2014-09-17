#include "LAPCFGParser.h"

#include "Mapping.h"
#include "ModelProjector.h"

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <cstdio>
#include <cmath>
#include <functional>
#include <iostream>
#include <fstream>
#include <limits>
#include <stdexcept>

using namespace std;

namespace AHCParser {

LAPCFGParser::LAPCFGParser()
    : prune_threshold_(1e-5)
    , smooth_unklex_(0) {
}

LAPCFGParser::~LAPCFGParser() {}

shared_ptr<LAPCFGParser> LAPCFGParser::loadFromBerkeleyDump(const string & path) {
    shared_ptr<LAPCFGParser> parser(new LAPCFGParser());

    parser->loadWordTable(path + "/words");
    parser->loadTagSet(path + "/splits");
    parser->loadLexicon(path + "/lexicon");
    parser->loadGrammar(path + "/grammar");
    parser->generateCoarseModels();

    return parser;
}

void LAPCFGParser::loadWordTable(const string & path) {
    cerr << "Loading words: " << path << " ..." << endl;
    
    ifstream ifs(path);
    if (!ifs.is_open()) {
        throw runtime_error("LAPCFGParser: file not found: " + path);
    }

    word_table_.reset(new Dictionary());
    string line;
    while (getline(ifs, line)) {
        boost::trim(line);
        if (line.size() >= 3 && line.substr(0, 3) == "UNK") {
            // skip UNK* entries
            continue;
        }
        word_table_->addWord(line);
    }
}

void LAPCFGParser::loadTagSet(const string & path) {
    cerr << "Loading tags: " << path << " ..." << endl;
    
    ifstream ifs(path);
    if (!ifs.is_open()) {
        throw runtime_error("LAPCFGParser: file not found: " + path);
    }

    try {
        tag_set_ = TagSet::loadFromStream(ifs);
    } catch (...) {
        throw runtime_error("LAPCFGParser: invalid file format: " + path);
    }
}

void LAPCFGParser::loadLexicon(const string & path) {
    cerr << "Loading lexicon: " << path << " ..." << endl;
    
    ifstream ifs(path);
    if (!ifs.is_open()) {
        throw runtime_error("LAPCFGParser: file not found: " + path);
    }

    try {
        shared_ptr<Lexicon> lexicon(Lexicon::loadFromStream(ifs, *word_table_, *tag_set_));
        lexicon_.push_back(lexicon);
    } catch (...) {
        throw runtime_error("LAPCFGParser: invalid file format: " + path);
    }
}

void LAPCFGParser::loadGrammar(const string & path) {
    cerr << "Loading grammar: " << path << " ..." << endl;
    
    ifstream ifs(path);
    if (!ifs.is_open()) {
        throw runtime_error("LAPCFGParser: file not found: " + path);
    }

    try {
        shared_ptr<Grammar> grammar(Grammar::loadFromStream(ifs, *tag_set_));
        grammar_.push_back(grammar);
    } catch (...) {
        throw runtime_error("LAPCFGParser: invalid file format: " + path);
    }
}

void LAPCFGParser::generateCoarseModels() {
    const int depth = tag_set_->getDepth();

    for (int level = depth-2; level >= 0; --level) {
        cerr << "Generating coarse model (level=" << level << ") ..." << endl;
        int fine = grammar_.size() - 1;
        ModelProjector projector(*tag_set_, *(lexicon_[fine]), *(grammar_[fine]), depth-1, level);
        lexicon_.insert(lexicon_.begin(), projector.generateLexicon());
        grammar_.insert(grammar_.begin(), projector.generateGrammar());
    }
}

shared_ptr<Tree<string> > LAPCFGParser::parse(const vector<string> & sentence) const {
    const int num_words = sentence.size();
    const int num_tags = tag_set_->numTags();
    const int depth = tag_set_->getDepth();
    const int root_tag = tag_set_->getTagId("ROOT");

    // check empty sentence
    if (num_words == 0) {
        return getDefaultParse();
    }

    vector<int> wid_list = makeWordIDList(sentence);

    CKYTable<vector<bool> > allowed(num_words, num_tags);
    CKYTable<vector<double> > inside(num_words, num_tags);
    CKYTable<vector<double> > outside(num_words, num_tags);
    vector<vector<Extent> > extent(num_words + 1, vector<Extent>(num_tags, {
        num_words + 1, // narrow_right
        -1, // narrow_left
        -1, // wide_right
        num_words + 1 })); // wide_left
    
    // pre-parsing

    for (int level = 0; level < depth; ++level) {
        initializeCharts(allowed, inside, outside, extent, level);
        setInsideScoresByLexicon(allowed, inside, wid_list, level);
        calculateInsideScores(allowed, inside, extent, level);

        // check if all possible parses are pruned
        double sentence_score = inside.at(0, num_words, root_tag)[0];
        if (sentence_score == 0.0) {
            cerr << "**** No any possible parses! ****" << endl;
            return getDefaultParse();
        }

        calculateOutsideScores(allowed, inside, outside, extent, level);
        pruneCharts(allowed, inside, outside, level);

        //fprintf(stderr, "pre-parse %d ... ROOT: %e\n", level, inside[0][num_words][root_tag][0]);
    } // level

    // retrieve max-rule parse over allowed nodes
    
    CKYTable<double> maxc_log_score(num_words, num_tags);
    CKYTable<int> maxc_left(num_words, num_tags);
    CKYTable<int> maxc_right(num_words, num_tags);
    CKYTable<int> maxc_mid(num_words, num_tags);
    CKYTable<int> maxc_child(num_words, num_tags);
    const double log_normalizer = log(inside.at(0, num_words, root_tag)[0]);
    const double NEG_INFTY = -1e20;
    const int fine_level = depth - 1;
    const Lexicon & fine_lexicon = getLexicon(fine_level);
    const Grammar & fine_grammar = getGrammar(fine_level);

    for (int len = 1; len <= num_words; ++len) {
        for (int begin = 0; begin < num_words - len + 1; ++begin) {
            int end = begin + len;

            // inirialize arrays
            
            for (int ptag = 0; ptag < num_tags; ++ptag) {
                maxc_log_score.at(begin, end, ptag) = NEG_INFTY;
                maxc_left.at(begin, end, ptag) = -1;
                maxc_right.at(begin, end, ptag) = -1;
                maxc_mid.at(begin, end, ptag) = -1;
                maxc_child.at(begin, end, ptag) = -1;
            }
            
            if (len > 1) {
                // process binary rules

                for (int ptag = 0; ptag < num_tags; ++ptag) {
                    if (fine_lexicon.hasEntry(ptag)) continue; // semi-terminal
                    auto & binary_rules_p = fine_grammar.getBinaryRuleListByPLR()[ptag];
                    int num_psub = tag_set_->numSubtags(ptag, fine_level);

                    for (int ltag = 0; ltag < num_tags; ++ltag) {
                        auto & binary_rules_pl = binary_rules_p[ltag];
                        int num_lsub = tag_set_->numSubtags(ltag, fine_level);

                        for (const BinaryRule * rule : binary_rules_pl) {
                            auto & score_list = rule->getScoreList();
                            int rtag = rule->right();

                            int min1 = extent[begin][ltag].narrow_right;
                            if (min1 >= end) continue;
                            int max1 = extent[end][rtag].narrow_left;
                            if (max1 < min1) continue;
                            int min2 = extent[end][rtag].wide_left;
                            int min = min1 > min2 ? min1 : min2;
                            if (min > max1) continue;
                            int max2 = extent[begin][ltag].wide_right;
                            int max = max1 < max2 ? max1 : max2;
                            if (min > max) continue;

                            int num_rsub = tag_set_->numSubtags(rtag, fine_level);
                            double old_log_score = maxc_log_score.at(begin, end, ptag);

                            for (int mid = min; mid <= max; ++mid) {
                                if (mid - begin > 1 && fine_lexicon.hasEntry(ltag)) continue; // semi-terminal
                                if (end - mid > 1 && fine_lexicon.hasEntry(rtag)) continue; // semi-terminal
                                double cur_log_score =
                                    maxc_log_score.at(begin, mid, ltag) +
                                    maxc_log_score.at(mid, end, rtag);
                                if (cur_log_score < old_log_score) continue;

                                double rule_score = 0.0;

                                for (int psub = 0; psub < num_psub; ++psub) {
                                    if (!allowed.at(begin, end, ptag)[psub]) continue;
                                    auto & score_list_p = score_list[psub];
                                    if (score_list_p.empty()) continue;
                                    double po = outside.at(begin, end, ptag)[psub];

                                    for (int lsub = 0; lsub < num_lsub; ++lsub) {
                                        if (!allowed.at(begin, mid, ltag)[lsub]) continue;
                                        auto & score_list_pl = score_list_p[lsub];
                                        if (score_list_pl.empty()) continue;
                                        double li = inside.at(begin, mid, ltag)[lsub];

                                        for (int rsub = 0; rsub < num_rsub; ++rsub) {
                                            if (!allowed.at(mid, end, rtag)[rsub]) continue;
                                            double ri = inside.at(mid, end, rtag)[rsub];
                                            double beta = score_list_pl[rsub];
                                            rule_score += po * li * ri * beta;
                                        }
                                    }
                                }

                                if (rule_score == 0) continue;

                                cur_log_score += log(rule_score) - log_normalizer;

                                if (cur_log_score > old_log_score) {
                                    old_log_score = cur_log_score;
                                    maxc_log_score.at(begin, end, ptag) = cur_log_score;
                                    maxc_left.at(begin, end, ptag) = ltag;
                                    maxc_right.at(begin, end, ptag) = rtag;
                                    maxc_mid.at(begin, end, ptag) = mid;
                                }
                            } // mid
                        } // rule
                    } // ltag
                } // ptag
            } else {
                // process lexicon

                for (int tag = 0; tag < num_tags; ++tag) {
                    int wid = wid_list[begin];
                    const LexiconEntry * ent_word = fine_lexicon.getEntry(tag, wid);
                    const LexiconEntry * ent_unk = fine_lexicon.getEntry(tag, -1);
                    if (!ent_word && !ent_unk) continue;
                    int num_sub = tag_set_->numSubtags(tag, fine_level);
                    double rule_score = 0.0;

                    for (int sub = 0; sub < num_sub; ++sub) {
                        if (!allowed.at(begin, end, tag)[sub]) continue;
                        double po = outside.at(begin, end, tag)[sub];
                        double beta =
                            (1.0 - smooth_unklex_) * (ent_word ? ent_word->getScore(sub) : 0.0) +
                            smooth_unklex_ * (ent_unk ? ent_unk->getScore(sub) : 0.0);
                        rule_score += po * beta;
                    }

                    if (rule_score == 0.0) continue;

                    maxc_log_score.at(begin, end, tag) = log(rule_score) - log_normalizer;
                }
            }

            // process unary rules
            
            vector<double> after_unary(num_tags);
            for (int tag = 0; tag < num_tags; ++tag) {
                after_unary[tag] = maxc_log_score.at(begin, end, tag);
            }

            for (int ptag = 0; ptag < num_tags; ++ptag) {
                if (fine_lexicon.hasEntry(ptag)) continue; // semi-terminal
                auto & unary_rules_p = fine_grammar.getUnaryRuleListByPC()[ptag];
                int num_psub = tag_set_->numSubtags(ptag, fine_level);
                
                for (const UnaryRule * rule : unary_rules_p) {
                    int ctag = rule->child();
                    if (len > 1 && fine_lexicon.hasEntry(ctag)) continue; // semi-terminal
                    if (ctag == ptag) continue;
                    auto & score_list = rule->getScoreList();
                    int num_csub = tag_set_->numSubtags(ctag, fine_level);

                    double cur_log_score = maxc_log_score.at(begin, end, ctag);
                    if (cur_log_score < after_unary[ptag]) continue;

                    double rule_score = 0.0;

                    for (int psub = 0; psub < num_psub; ++psub) {
                        if (!allowed.at(begin, end, ptag)[psub]) continue;
                        auto & score_list_p = score_list[psub];
                        if (score_list_p.empty()) continue;
                        double po = outside.at(begin, end, ptag)[psub];
                        
                        for (int csub = 0; csub < num_csub; ++csub) {
                            if (!allowed.at(begin, end, ctag)[csub]) continue;
                            double ci = inside.at(begin, end, ctag)[csub];
                            double beta = score_list_p[csub];
                            rule_score += po * ci * beta;
                        }
                    }

                    if (rule_score == 0.0) continue;

                    cur_log_score += log(rule_score) - log_normalizer;

                    if (cur_log_score > after_unary[ptag]) {
                        after_unary[ptag] = cur_log_score;
                        maxc_child.at(begin, end, ptag) = ctag;
                    }
                } // rule
            } // ptag

            for (int tag = 0; tag < num_tags; ++tag) {
                maxc_log_score.at(begin, end, tag) = after_unary[tag];
            }

            /*
            for (int tag  = 0; tag < num_tags; ++tag) {
                if (maxc_log_score.at(begin, end, tag) == NEG_INFTY) continue;
                if (maxc_child.at(begin, end, tag) >= 0) {
                    fprintf(stderr, "max-rule[%d:%d] %s -> %s (%e)\n",
                        begin, end,
                        tag_set_->getTagName(tag).c_str(),
                        tag_set_->getTagName(maxc_child.at(begin, end, tag)).c_str(),
                        maxc_log_score.at(begin, end, tag));
                } else if (maxc_left.at(begin, end, tag) >= 0) {
                    fprintf(stderr, "max-rule[%d:%d] %s -> %s/%d/%s (%e)\n",
                        begin, end,
                        tag_set_->getTagName(tag).c_str(),
                        tag_set_->getTagName(maxc_left.at(begin, end, tag)).c_str(),
                        maxc_mid.at(begin, end, tag),
                        tag_set_->getTagName(maxc_right.at(begin, end, tag)).c_str(),
                        maxc_log_score.at(begin, end, tag));
                }
            }
            */
        } // begin
    } // len

    // build max-rule parse tree

    function<void(Tree<string> &, Tree<string> *)> addChildOrCoalesce
        = [](Tree<string> & parent_tree, Tree<string> * child_tree) {
    
        string ptag = parent_tree.value();
        string ctag = child_tree->value();
        string pbar = ptag;
        if (pbar.empty() || pbar[0] != '@') {
            pbar = "@" + pbar;
        }

        if (ctag == pbar) {
            // (X (...) (@X foo bar)) -> (X (...) foo bar)
            //cerr << "[Coalesce] " << ptag << "(" << pbar << ") " << ctag << endl;
            int nc = child_tree->numChildren();
            for (int i = 0; i < nc; ++i) {
                parent_tree.addChild(new Tree<string>(child_tree->child(i)));
            }
            delete child_tree;
        } else {
            // add as child
            //cerr << "[Add] " << ptag << "(" << pbar << ") " << ctag << endl;
            parent_tree.addChild(child_tree);
        }
    };

    function<Tree<string> *(int, int, int, bool)> buildTree
        = [&](int begin, int end, int ptag, bool first_time) -> Tree<string> * {
        
        if (ptag < 0) return nullptr; // for ptag = -1

        int ctag = maxc_child.at(begin, end, ptag);

        Tree<string> * parent_tree = nullptr;

        if (ctag != -1 && first_time) {
            // make unary derivation

            Tree<string> * child_tree = buildTree(begin, end, ctag, false);
            if (child_tree) {
                parent_tree = new Tree<string>(tag_set_->getTagName(ptag));
                addChildOrCoalesce(*parent_tree, child_tree);
            }
        } else if (end - begin > 1) {
            // make binary derivation
            
            int ltag = maxc_left.at(begin, end, ptag);
            int rtag = maxc_right.at(begin, end, ptag);
            int mid = maxc_mid.at(begin, end, ptag);
            Tree<string> * left_tree = buildTree(begin, mid, ltag, true);
            Tree<string> * right_tree = buildTree(mid, end, rtag, true);
            if (left_tree && right_tree) {
                parent_tree = new Tree<string>(tag_set_->getTagName(ptag));
                addChildOrCoalesce(*parent_tree, left_tree);
                addChildOrCoalesce(*parent_tree, right_tree);
            } else {
                delete left_tree;
                delete right_tree;
            }

        } else {
            // make lexical/word nodes
            
            parent_tree = new Tree<string>(tag_set_->getTagName(ptag));
            Tree<string> * word_node = new Tree<string>(sentence[begin]);
            parent_tree->addChild(word_node);
        }

        return parent_tree; // complete tree or nullptr
    };

    Tree<string> * parse = buildTree(0, num_words, root_tag, true);
    if (parse) return shared_ptr<Tree<string> >(parse);
    else return getDefaultParse();
}

void LAPCFGParser::setPruningThreshold(double value) {
    if (value < 0.0 || value > 1.0)
        throw runtime_error("LAPCFGParser::setPruningThreshold(): invalid value");
    prune_threshold_ = value;
}

void LAPCFGParser::setUNKLexiconSmoothing(double value) {
    if (value < 0.0 || value > 1.0)
        throw runtime_error("LAPCFGParser::setUNKLexiconSmoothing(): invalid value");
    smooth_unklex_ = value;
}

shared_ptr<Tree<string> > LAPCFGParser::getDefaultParse() const {
    return shared_ptr<Tree<string> >(new Tree<string>(""));
}

vector<int> LAPCFGParser::makeWordIDList(const vector<string> & sentence) const {
    const int num_words = sentence.size();
    vector<int> wid_list(num_words);
    //cerr << "  WID:";
    for (int i = 0; i < num_words; ++i) {
        wid_list[i] = word_table_->getId(sentence[i]);
        //cerr << " " << wid_list[i];
    }
    //cerr << endl;
    return wid_list;
}

void LAPCFGParser::initializeCharts(
    CKYTable<vector<bool> > & allowed,
    CKYTable<vector<double> > & inside,
    CKYTable<vector<double> > & outside,
    vector<vector<Extent> > & extent,
    int cur_level) const {

    const int num_words = allowed.numWords();
    const int num_tags = allowed.numTags();
   
    unique_ptr<Mapping> mapping(nullptr);
    if (cur_level > 0) {
        mapping.reset(new Mapping(*tag_set_, cur_level - 1, cur_level));
    }

    for (int begin = 0; begin < num_words; ++begin) {
        for (int end = begin + 1; end <= num_words; ++end) {
            for (int tag = 0; tag < num_tags; ++tag) {
                int num_subtags_fine = tag_set_->numSubtags(tag, cur_level);
                inside.at(begin, end, tag).assign(num_subtags_fine, 0.0);
                outside.at(begin, end, tag).assign(num_subtags_fine, 0.0);
                if (cur_level > 0) {
                    // initialize subtag constraints
                    int num_subtags_coarse = tag_set_->numSubtags(tag, cur_level - 1);
                    vector<bool> & allowed_fine = allowed.at(begin, end, tag);
                    vector<bool> allowed_coarse = allowed_fine;
                    allowed_fine.assign(num_subtags_fine, false);
                    for (int subtag_coarse = 0; subtag_coarse < num_subtags_coarse; ++subtag_coarse) {
                        if (!allowed_coarse[subtag_coarse]) continue;
                        for (int subtag_fine : mapping->getCoarseToFineMaps(tag, subtag_coarse)) {
                            allowed_fine[subtag_fine] = true;
                        }
                    }
                } else {
                    // only 1 subtag is possible for the first time
                    allowed.at(begin, end, tag).assign(num_subtags_fine, true);
                }
            }
        }

        int end2 = begin + 1;
        for (int tag = 0; tag < num_tags; ++tag) {
            extent[begin][tag].narrow_right = end2;
            extent[begin][tag].wide_right = end2;
            extent[end2][tag].narrow_left = begin;
            extent[end2][tag].wide_left = begin;
        }
    }
}

void LAPCFGParser::setInsideScoresByLexicon(
    const CKYTable<vector<bool> > & allowed,
    CKYTable<vector<double> > & inside,
    const vector<int> & wid_list,
    int cur_level) const {

    const int num_words = allowed.numWords();
    const int num_tags = allowed.numTags();
    const Lexicon & cur_lexicon = getLexicon(cur_level);

    for (int begin = 0; begin < num_words; ++begin) {
        int end = begin + 1;
        int wid = wid_list[begin];
        
        for (int tag = 0; tag < num_tags; ++tag) {
            const LexiconEntry * ent_word = cur_lexicon.getEntry(tag, wid);
            const LexiconEntry * ent_unk = cur_lexicon.getEntry(tag, -1);
            if (!ent_word && !ent_unk) continue;
            int num_sub = tag_set_->numSubtags(tag, cur_level);
            
            for (int sub = 0; sub < num_sub; ++sub) {
                if (!allowed.at(begin, end, tag)[sub]) continue;
                inside.at(begin, end, tag)[sub] =
                    (1.0 - smooth_unklex_) * (ent_word ? ent_word->getScore(sub) : 0.0) +
                    smooth_unklex_ * (ent_unk ? ent_unk->getScore(sub) : 0.0);
                //cerr << begin << "(" << sentence[begin] << ")->"
                //    << tag_set_->getTagName(tag) << "[" << sub << "] = "
                //    << inside.at(begin, end, tag)[sub] << endl;
            }
        }
    }
}

void LAPCFGParser::calculateInsideScores(
    const CKYTable<vector<bool> > & allowed,
    CKYTable<vector<double> > & inside,
    vector<vector<Extent> > & extent,
    int cur_level) const {

    const int num_words = allowed.numWords();
    const int num_tags = allowed.numTags();
    const Lexicon & cur_lexicon = getLexicon(cur_level);
    const Grammar & cur_grammar = getGrammar(cur_level);

    for (int len = 1; len <= num_words; ++len) {
        for (int begin = 0; begin < num_words - len + 1; ++begin) {
            int end = begin + len;

            // process binary rules

            if (len > 1) {
                for (int ptag = 0; ptag < num_tags; ++ptag) {
                    if (cur_lexicon.hasEntry(ptag)) continue; // semi-terminal
                    auto & binary_rules_p = cur_grammar.getBinaryRuleListByPLR()[ptag];
                    int num_psub = tag_set_->numSubtags(ptag, cur_level);
                
                    for (int psub = 0; psub < num_psub; ++psub) {
                        if (!allowed.at(begin, end, ptag)[psub]) continue;
                        double sum = 0.0;

                        for (int ltag = 0; ltag < num_tags; ++ltag) {
                            auto & binary_rules_pl = binary_rules_p[ltag];
                            int num_lsub = tag_set_->numSubtags(ltag, cur_level);
                    
                            for (const BinaryRule * rule : binary_rules_pl) {
                                int rtag = rule->right();

                                int min1 = extent[begin][ltag].narrow_right;
                                if (min1 >= end) continue;
                                int max1 = extent[end][rtag].narrow_left;
                                if (max1 < min1) continue;
                                int min2 = extent[end][rtag].wide_left;
                                int min = min1 > min2 ? min1 : min2;
                                if (min > max1) continue;
                                int max2 = extent[begin][ltag].wide_right;
                                int max = max1 < max2 ? max1 : max2;
                                if (min > max) continue;

                                int num_rsub = tag_set_->numSubtags(rtag, cur_level);
                                auto & score_list_p = rule->getScoreList()[psub];
                                if (score_list_p.empty()) continue;
                                
                                for (int mid = min; mid <= max; ++mid) {
                                    if (mid - begin > 1 && cur_lexicon.hasEntry(ltag)) continue; // semi-terminal
                                    if (end - mid > 1 && cur_lexicon.hasEntry(rtag)) continue; // semi-terminal
                            
                                    for (int lsub = 0; lsub < num_lsub; ++lsub) {
                                        if (!allowed.at(begin, mid, ltag)[lsub]) continue;
                                        auto & score_list_pl = score_list_p[lsub];
                                        if (score_list_pl.empty()) continue;
                        
                                        for (int rsub = 0; rsub < num_rsub; ++rsub) {
                                            if (!allowed.at(mid, end, rtag)[rsub]) continue;
                                
                                            sum +=
                                                score_list_pl[rsub] *
                                                inside.at(begin, mid, ltag)[lsub] *
                                                inside.at(mid, end, rtag)[rsub];
                                        }
                                    }
                                }
                            }
                        }

                        inside.at(begin, end, ptag)[psub] = sum;
                    } // psub

                    if (begin > extent[end][ptag].narrow_left) {
                        extent[end][ptag].narrow_left = begin;
                        extent[end][ptag].wide_left = begin;
                    } else if (begin < extent[end][ptag].wide_left) {
                        extent[end][ptag].wide_left = begin;
                    }
                    if (end < extent[begin][ptag].narrow_right) {
                        extent[begin][ptag].narrow_right = end;
                        extent[begin][ptag].wide_right = end;
                    } else if (end > extent[begin][ptag].wide_right) {
                        extent[begin][ptag].wide_right = end;
                    }
                } // ptag
            } // len > 1

            // process unary rules

            vector<vector<double> > delta_unary(num_tags);

            for (int ptag = 0; ptag < num_tags; ++ptag) {
                if (cur_lexicon.hasEntry(ptag)) continue; // semi-terminal
                auto & unary_rules_p = cur_grammar.getUnaryRuleListByPC()[ptag];
                int num_psub = tag_set_->numSubtags(ptag, cur_level);
                delta_unary[ptag].assign(num_psub, 0.0);
                
                for (int psub = 0; psub < num_psub; ++psub) {
                    if (!allowed.at(begin, end, ptag)[psub]) continue;

                    for (const UnaryRule * rule : unary_rules_p) {
                        int ctag = rule->child();
                        if (len > 1 && cur_lexicon.hasEntry(ctag)) continue; // semi-terminal
                        if (ctag == ptag) continue;
                        int num_csub = tag_set_->numSubtags(ctag, cur_level);
                        auto & score_list_p = rule->getScoreList()[psub];
                        if (score_list_p.empty()) continue;
                        
                        for (int csub = 0; csub < num_csub; ++csub) {
                            if (!allowed.at(begin, end, ctag)[csub]) continue;
                            delta_unary[ptag][psub] +=
                                score_list_p[csub] *
                                inside.at(begin, end, ctag)[csub];
                        }
                    }
                }
            }

            for (int ptag = 0; ptag < num_tags; ++ptag) {
                if (cur_lexicon.hasEntry(ptag)) continue; // semi-terminal
                int num_psub = tag_set_->numSubtags(ptag, cur_level);
                for (int psub = 0; psub < num_psub; ++psub) {
                    if (!allowed.at(begin, end, ptag)[psub]) continue;
                    inside.at(begin, end, ptag)[psub] += delta_unary[ptag][psub];
                }
            }

            /*
            double best_score = -1;
            int best_ptag = -1;
            int best_psub = -1;
            for (int ptag = 0; ptag < num_tags; ++ptag) {
                if (cur_lexicon.hasEntry(ptag)) continue; // semi-terminal
                int num_psub = tag_set_->numSubtags(ptag, level);
                for (int psub = 0; psub < num_psub; ++psub) {
                    if (inside.at(begin, end, ptag)[psub] > best_score) {
                        best_score = inside.at(begin, end, ptag)[psub];
                        best_ptag = ptag;
                        best_psub = psub;
                    }
                }
            }

            fprintf(stderr, "best[%d:%d] ... %s[%d] = %e\n",
                begin, end, tag_set_->getTagName(best_ptag).c_str(), best_psub, best_score);
            */
        } // begin
    } // len
}

void LAPCFGParser::calculateOutsideScores(
    const CKYTable<vector<bool> > & allowed,
    const CKYTable<vector<double> > & inside,
    CKYTable<vector<double> > & outside,
    vector<vector<Extent> > & extent,
    int cur_level) const {

    const int num_words = allowed.numWords();
    const int num_tags = allowed.numTags();
    const int root_tag = tag_set_->getTagId("ROOT");
    const Lexicon & cur_lexicon = getLexicon(cur_level);
    const Grammar & cur_grammar = getGrammar(cur_level);

    outside.at(0, num_words, root_tag)[0] = 1.0;

    for (int len = num_words; len >= 1; --len) {
        for (int begin = 0; begin < num_words - len + 1; ++begin) {
            int end = begin + len;

            // process unary rules

            vector<vector<double> > delta_unary(num_tags);

            for (int ctag = 0; ctag < num_tags; ++ctag) {
                if (len > 1 && cur_lexicon.hasEntry(ctag)) continue; // semi-terminal
                auto & unary_rules_c = cur_grammar.getUnaryRuleListByCP()[ctag];
                int num_csub = tag_set_->numSubtags(ctag, cur_level);
                delta_unary[ctag].assign(num_csub, 0.0);

                for (int csub = 0; csub < num_csub; ++csub) {
                    if (!allowed.at(begin, end, ctag)[csub]) continue;
                    
                    for (const UnaryRule * rule : unary_rules_c) {
                        int ptag = rule->parent();
                        if (ptag == ctag) continue;
                        int num_psub = tag_set_->numSubtags(ptag, cur_level);
                        auto & score_list = rule->getScoreList();

                        for (int psub = 0; psub < num_psub; ++psub) {
                            if (!allowed.at(begin, end, ptag)[psub]) continue;
                            auto & score_list_p = score_list[psub];
                            if (score_list_p.empty()) continue;
                            delta_unary[ctag][csub] +=
                                score_list_p[csub] *
                                outside.at(begin, end, ptag)[psub];
                        }
                    }
                }
            }

            for (int ctag = 0; ctag < num_tags; ++ctag) {
                if (len > 1 && cur_lexicon.hasEntry(ctag)) continue; // semi-terminal
                int num_csub = tag_set_->numSubtags(ctag, cur_level);
                for (int csub = 0; csub < num_csub; ++csub) {
                    outside.at(begin, end, ctag)[csub] += delta_unary[ctag][csub];
                }
            }

            // process binary rules

            if (len > 1) {
                for (int ptag = 0; ptag < num_tags; ++ptag) {
                    if (cur_lexicon.hasEntry(ptag)) continue; // semi-terminal
                    auto binary_rules_p = cur_grammar.getBinaryRuleListByPLR()[ptag];
                    int num_psub = tag_set_->numSubtags(ptag, cur_level);

                    for (int psub = 0; psub < num_psub; ++psub) {
                        if (!allowed.at(begin, end, ptag)[psub]) continue;

                        for (int ltag = 0; ltag < num_tags; ++ltag) {
                            auto & binary_rules_pl = binary_rules_p[ltag];
                            int num_lsub = tag_set_->numSubtags(ltag, cur_level);

                            for (const BinaryRule * rule : binary_rules_pl) {
                                int rtag = rule->right();

                                int min1 = extent[begin][ltag].narrow_right;
                                if (min1 >= end) continue;
                                int max1 = extent[end][rtag].narrow_left;
                                if (max1 < min1) continue;
                                int min2 = extent[end][rtag].wide_left;
                                int min = min1 > min2 ? min1 : min2;
                                if (min > max1) continue;
                                int max2 = extent[begin][ltag].wide_right;
                                int max = max1 < max2 ? max1 : max2;
                                if (min > max) continue;

                                int num_rsub = tag_set_->numSubtags(rtag, cur_level);
                                auto & score_list_p = rule->getScoreList()[psub];
                                if (score_list_p.empty()) continue;

                                for (int mid = min; mid <= max; ++mid) {
                                    if (mid - begin > 1 && cur_lexicon.hasEntry(ltag)) continue; // semi-terminal
                                    if (end - mid > 1 && cur_lexicon.hasEntry(rtag)) continue; // semi-terminal
                        
                                    for (int lsub = 0; lsub < num_lsub; ++lsub) {
                                        if (!allowed.at(begin, mid, ltag)[lsub]) continue;
                                        auto & score_list_pl = score_list_p[lsub];
                                        if (score_list_pl.empty()) continue;

                                        for (int rsub = 0; rsub < num_rsub; ++rsub) {
                                            if (!allowed.at(mid, end, rtag)[rsub]) continue;

                                            double rule_score = score_list_pl[rsub];
                                            double parent_score = outside.at(begin, end, ptag)[psub];
                                            outside.at(begin, mid, ltag)[lsub] +=
                                                rule_score * parent_score *
                                                inside.at(mid, end, rtag)[rsub];
                                            outside.at(mid, end, rtag)[rsub] +=
                                                rule_score * parent_score *
                                                inside.at(begin, mid, ltag)[lsub];
                                        }
                                    }
                                }
                            }
                        }
                    } // psub
                } // ptag
            } // len > 1

            /*
            for (int ctag = 0; ctag < num_tags; ++ctag) {
                int num_csub = tag_set_->numSubtags(ctag, level);
                for (int csub = 0; csub < num_csub; ++csub) {
                    cerr << begin << "-" << end << ": " <<
                        "" << tag_set_->getTagName(ctag) << "[" << csub << "] =" <<
                        outside.at(begin, end, ctag)[csub] << endl;
                }
            }
            */
        } // begin
    } // len
}

void LAPCFGParser::pruneCharts(
    CKYTable<vector<bool> > & allowed,
    const CKYTable<vector<double> > & inside,
    const CKYTable<vector<double> > & outside,
    int cur_level) const {
    
    const int num_words = allowed.numWords();
    const int num_tags = allowed.numTags();
    const int root_tag = tag_set_->getTagId("ROOT");
    //int num_pruned = 0;

    const double sentence_score = inside.at(0, num_words, root_tag)[0];

    for (int len = 1; len <= num_words; ++len) {
        for (int begin = 0; begin < num_words - len + 1; ++begin) {
            int end = begin + len;

            //double best_score = -1;
            //int best_tag = -1;
            //int best_sub = -1;

            for (int tag = 0; tag < num_tags; ++tag) {
                int num_sub = tag_set_->numSubtags(tag, cur_level);

                for (int sub = 0; sub < num_sub; ++sub) {
                    double posterior =
                        inside.at(begin, end, tag)[sub] *
                        outside.at(begin, end, tag)[sub] /
                        sentence_score;
                    if (posterior < prune_threshold_) {
                        allowed.at(begin, end, tag)[sub] = false;
                        //++num_pruned;
                    }

                    //if (score > best_score) {
                    //    best_score = score;
                    //    best_tag = tag;
                    //    best_sub = sub;
                    //}
                }
            }

            //fprintf(stderr, "best[%d:%d] ... %s[%d] = %e\n",
            //    begin, end, tag_set_->getTagName(best_tag).c_str(), best_sub, best_score);
            
        } // begin
    } // len

    //cerr << "pruned: " << num_pruned << endl;
}

} // namespace AHCParser

