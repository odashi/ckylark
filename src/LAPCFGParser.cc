#include "LAPCFGParser.h"

#include "Mapping.h"
#include "ModelProjector.h"

#include <boost/algorithm/string.hpp>
#include <boost/multi_array.hpp>

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

LAPCFGParser::LAPCFGParser() {}

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
    int depth = tag_set_->getDepth();

    for (int level = depth-2; level >= 0; --level) {
        cerr << "Generating coarse model (level=" << level << ") ..." << endl;
        int fine = grammar_.size() - 1;
        ModelProjector projector(*tag_set_, *(lexicon_[fine]), *(grammar_[fine]), depth-1, level);
        lexicon_.insert(lexicon_.begin(), projector.generateLexicon());
        grammar_.insert(grammar_.begin(), projector.generateGrammar());
    }
}

shared_ptr<Tree<string> > LAPCFGParser::parse(const vector<string> & text) const {
    int num_words = text.size();
    int num_tags = tag_set_->numTags();
    int depth = tag_set_->getDepth();
    int root_tag = tag_set_->getTagId("ROOT");
    double prune_threshold = 1e-5;
    boost::array<int, 3> shape_cky = {{ num_words, num_words + 1, num_tags }};

    // check empty text
    if (num_words == 0) {
        return getDefaultParse();
    }

    // convert words into ID
    vector<int> wid_list(num_words);
    cerr << "  WID:";
    for (int i = 0; i < num_words; ++i) {
        wid_list[i] = word_table_->getId(text[i]);
        cerr << " " << wid_list[i];
    }
    cerr << endl;

    // pre-parsing

    boost::multi_array<vector<bool>, 3> allowed(shape_cky);
    boost::multi_array<vector<double>, 3> inside(shape_cky);
    boost::multi_array<vector<double>, 3> outside(shape_cky);
    
    for (int level = 0; level < depth; ++level) {
        const Lexicon & cur_lexicon = *(lexicon_[level]);
        const Grammar & cur_grammar = *(grammar_[level]);

        unique_ptr<Mapping> mapping(nullptr);
        if (level > 0) {
            mapping.reset(new Mapping(*tag_set_, level - 1, level));
        }

        // initialize arrays

        for (int begin = 0; begin < num_words; ++begin) {
            for (int end = begin + 1; end <= num_words; ++end) {
                for (int tag = 0; tag < num_tags; ++tag) {
                    int num_subtags_fine = tag_set_->numSubtags(tag, level);
                    inside[begin][end][tag].assign(num_subtags_fine, 0.0);
                    outside[begin][end][tag].assign(num_subtags_fine, 0.0);
                    if (level > 0) {
                        // initialize subtag constraints
                        int num_subtags_coarse = tag_set_->numSubtags(tag, level - 1);
                        vector<bool> & allowed_fine = allowed[begin][end][tag];
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
                        allowed[begin][end][tag].assign(num_subtags_fine, true);
                    }
                }
            }
        }

        // initialize inside scores of leaves

        for (int begin = 0; begin < num_words; ++begin) {
            int end = begin + 1;
            int wid = wid_list[begin];
            
            for (int tag = 0; tag < num_tags; ++tag) {
                const LexiconEntry * ent = cur_lexicon.getEntry(tag, wid);
                if (!ent) continue;
                int num_sub = ent->numSubtags();
                
                for (int sub = 0; sub < num_sub; ++sub) {
                    if (!allowed[begin][end][tag][sub]) continue;
                    inside[begin][end][tag][sub] = ent->getScore(sub);
                    //cerr << begin << "(" << text[begin] << ")->"
                    //    << tag_set_->getTagName(tag) << "[" << sub << "] = "
                    //    << inside[begin][end][tag][sub] << endl;
                }
            }
        }

        // calculate inside scores

        for (int len = 1; len <= num_words; ++len) {
            for (int begin = 0; begin < num_words - len + 1; ++begin) {
                int end = begin + len;

                // process binary rules

                if (len > 1) {
                    for (int ptag = 0; ptag < num_tags; ++ptag) {
                        auto & binary_rules_p = cur_grammar.getBinaryRuleListByPLR()[ptag];
                        int num_psub = tag_set_->numSubtags(ptag, level);
                    
                        for (int psub = 0; psub < num_psub; ++psub) {
                            if (!allowed[begin][end][ptag][psub]) continue;
                            double sum = 0.0;

                            for (int mid = begin + 1; mid < end; ++mid) {

                                for (int ltag = 0; ltag < num_tags; ++ltag) {
                                    auto & binary_rules_pl = binary_rules_p[ltag];
                                    int num_lsub = tag_set_->numSubtags(ltag, level);
                            
                                    for (int lsub = 0; lsub < num_lsub; ++lsub) {
                                        if (!allowed[begin][mid][ltag][lsub]) continue;
                                    
                                        for (const BinaryRule * rule : binary_rules_pl) {
                                            int rtag = rule->right();
                                            int num_rsub = tag_set_->numSubtags(rtag, level);
                                            auto & score_list = rule->getScoreList();
                                            if (score_list[psub][lsub].empty()) continue;
                                
                                            for (int rsub = 0; rsub < num_rsub; ++rsub) {
                                                if (!allowed[mid][end][rtag][rsub]) continue;
                                        
                                                sum +=
                                                    score_list[psub][lsub][rsub] *
                                                    inside[begin][mid][ltag][lsub] *
                                                    inside[mid][end][rtag][rsub];
                                            }
                                        }
                                    }
                                }
                            } // mid

                            inside[begin][end][ptag][psub] = sum;

                        } // psub
                    } // ptag
                } // len > 1

                // process unary rules

                vector<vector<double> > delta_unary(num_tags);

                for (int ptag = 0; ptag < num_tags; ++ptag) {
                    auto & unary_rules_p = cur_grammar.getUnaryRuleListByPC()[ptag];
                    int num_psub = tag_set_->numSubtags(ptag, level);
                    delta_unary[ptag].assign(num_psub, 0.0);
                    
                    for (int psub = 0; psub < num_psub; ++psub) {
                        if (!allowed[begin][end][ptag][psub]) continue;

                        for (const UnaryRule * rule : unary_rules_p) {
                            int ctag = rule->child();
                            if (ctag == ptag) continue;
                            int num_csub = tag_set_->numSubtags(ctag, level);
                            auto & score_list = rule->getScoreList();
                            
                            for (int csub = 0; csub < num_csub; ++csub) {
                                if (!allowed[begin][end][ctag][csub]) continue;
                                delta_unary[ptag][psub] +=
                                    score_list[psub][csub] *
                                    inside[begin][end][ctag][csub];
                            }
                        }
                    }
                }

                for (int ptag = 0; ptag < num_tags; ++ptag) {
                    int num_psub = tag_set_->numSubtags(ptag, level);
                    for (int psub = 0; psub < num_psub; ++psub) {
                        if (!allowed[begin][end][ptag][psub]) continue;
                        inside[begin][end][ptag][psub] += delta_unary[ptag][psub];
                    }
                }

                /*
                double best_score = -1;
                int best_ptag = -1;
                int best_psub = -1;
                for (int ptag = 0; ptag < num_tags; ++ptag) {
                    int num_psub = tag_set_->numSubtags(ptag, level);
                    for (int psub = 0; psub < num_psub; ++psub) {
                        if (inside[begin][end][ptag][psub] > best_score) {
                            best_score = inside[begin][end][ptag][psub];
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

        // check if all possible parses are pruned

        double sentence_score = inside[0][num_words][root_tag][0];
        if (sentence_score == 0.0) {
            cerr << "**** No any possible parses! ****" << endl;
            return getDefaultParse();
        }
        
        // initialize outside scores of root

        outside[0][num_words][root_tag][0] = 1.0;

        // calculate outside scores

        for (int len = num_words; len >= 1; --len) {
            for (int begin = 0; begin < num_words - len + 1; ++begin) {
                int end = begin + len;

                // process unary rules

                vector<vector<double> > delta_unary(num_tags);

                for (int ctag = 0; ctag < num_tags; ++ctag) {
                    auto & unary_rules_c = cur_grammar.getUnaryRuleListByCP()[ctag];
                    int num_csub = tag_set_->numSubtags(ctag, level);
                    delta_unary[ctag].assign(num_csub, 0.0);

                    for (int csub = 0; csub < num_csub; ++csub) {
                        if (!allowed[begin][end][ctag][csub]) continue;
                        
                        for (const UnaryRule * rule : unary_rules_c) {
                            int ptag = rule->parent();
                            if (ptag == ctag) continue;
                            int num_psub = tag_set_->numSubtags(ptag, level);
                            auto & score_list = rule->getScoreList();

                            for (int psub = 0; psub < num_psub; ++psub) {
                                if (!allowed[begin][end][ptag][psub]) continue;
                                delta_unary[ctag][csub] +=
                                    score_list[psub][csub] *
                                    outside[begin][end][ptag][psub];
                            }
                        }
                    }
                }

                for (int ctag = 0; ctag < num_tags; ++ctag) {
                    int num_csub = tag_set_->numSubtags(ctag, level);
                    for (int csub = 0; csub < num_csub; ++csub) {
                        outside[begin][end][ctag][csub] += delta_unary[ctag][csub];
                    }
                }

                // process binary rules

                if (len > 1) {
                    for (int ptag = 0; ptag < num_tags; ++ptag) {
                        auto binary_rules_p = cur_grammar.getBinaryRuleListByPLR()[ptag];
                        int num_psub = tag_set_->numSubtags(ptag, level);

                        for (int psub = 0; psub < num_psub; ++psub) {
                            if (!allowed[begin][end][ptag][psub]) continue;

                            for (int mid = begin + 1; mid < end; ++mid) {
                                
                                for (int ltag = 0; ltag < num_tags; ++ltag) {
                                    auto & binary_rules_pl = binary_rules_p[ltag];
                                    int num_lsub = tag_set_->numSubtags(ltag, level);

                                    for (int lsub = 0; lsub < num_lsub; ++lsub) {
                                        if (!allowed[begin][mid][ltag][lsub]) continue;

                                        for (const BinaryRule * rule : binary_rules_pl) {
                                            int rtag = rule->right();
                                            int num_rsub = tag_set_->numSubtags(rtag, level);
                                            auto & score_list = rule->getScoreList();
                                            if (score_list[psub][lsub].empty()) continue;

                                            for (int rsub = 0; rsub < num_rsub; ++rsub) {
                                                if (!allowed[mid][end][rtag][rsub]) continue;

                                                double rule_score = score_list[psub][lsub][rsub];
                                                double parent_score = outside[begin][end][ptag][psub];
                                                outside[begin][mid][ltag][lsub] +=
                                                    rule_score * parent_score *
                                                    inside[mid][end][rtag][rsub];
                                                outside[mid][end][rtag][rsub] +=
                                                    rule_score * parent_score *
                                                    inside[begin][mid][ltag][lsub];
                                            }
                                        }
                                    } // lsub
                                } // ltag
                            } // mid
                        } // psub
                    } // ptag
                } // len > 1

                /*
                for (int ctag = 0; ctag < num_tags; ++ctag) {
                    int num_csub = tag_set_->numSubtags(ctag, level);
                    for (int csub = 0; csub < num_csub; ++csub) {
                        cerr << begin << "-" << end << ": " <<
                            "" << tag_set_->getTagName(ctag) << "[" << csub << "] =" <<
                            outside[begin][end][ctag][csub] << endl;
                    }
                }
                */
            } // begin
        } // len

        // pruning

        //int num_pruned = 0;

        for (int len = 1; len <= num_words; ++len) {
            for (int begin = 0; begin < num_words - len + 1; ++begin) {
                int end = begin + len;

                //double best_score = -1;
                //int best_ptag = -1;
                //int best_psub = -1;

                for (int ptag = 0; ptag < num_tags; ++ptag) {
                    int num_psub = tag_set_->numSubtags(ptag, level);

                    for (int psub = 0; psub < num_psub; ++psub) {
                        double posterior =
                            inside[begin][end][ptag][psub] *
                            outside[begin][end][ptag][psub] /
                            sentence_score;
                        if (posterior < prune_threshold) {
                            allowed[begin][end][ptag][psub] = false;
                            //++num_pruned;
                        }

                        //if (score > best_score) {
                        //    best_score = score;
                        //    best_ptag = ptag;
                        //    best_psub = psub;
                        //}
                    }
                }

                //fprintf(stderr, "best[%d:%d] ... %s[%d] = %e\n",
                //    begin, end, tag_set_->getTagName(best_ptag).c_str(), best_psub, best_score);
                
            } // begin
        } // len

        //cerr << "pruned: " << num_pruned << endl;
 
        //fprintf(stderr, "pre-parse %d ... ROOT: %e\n", level, inside[0][num_words][root_tag][0]);
    } // level

    // retrieve max-rule parse over allowed nodes
    
    boost::multi_array<double, 3> maxc_log_score(shape_cky); // log-likelihood
    boost::multi_array<int, 3> maxc_left(shape_cky); // for binary derivation
    boost::multi_array<int, 3> maxc_right(shape_cky); // for binary derivation
    boost::multi_array<int, 3> maxc_mid(shape_cky); // for binary derivation
    boost::multi_array<int, 3> maxc_child(shape_cky); // for unary derivation
    double log_normalizer = log(inside[0][num_words][root_tag][0]);
    double NEG_INFTY = -1e20;
    int fine_level = depth - 1;
    Lexicon & fine_lexicon = *(lexicon_[fine_level]);
    Grammar & fine_grammar = *(grammar_[fine_level]);

    for (int len = 1; len <= num_words; ++len) {
        for (int begin = 0; begin < num_words - len + 1; ++begin) {
            int end = begin + len;

            // inirialize arrays
            
            for (int ptag = 0; ptag < num_tags; ++ptag) {
                maxc_log_score[begin][end][ptag] = NEG_INFTY;
                maxc_left[begin][end][ptag] = -1;
                maxc_right[begin][end][ptag] = -1;
                maxc_mid[begin][end][ptag] = -1;
                maxc_child[begin][end][ptag] = -1;
            }
            
            if (len > 1) {
                // process binary rules

                for (int ptag = 0; ptag < num_tags; ++ptag) {
                    auto & binary_rules_p = fine_grammar.getBinaryRuleListByPLR()[ptag];
                    int num_psub = tag_set_->numSubtags(ptag, fine_level);

                    for (int ltag = 0; ltag < num_tags; ++ltag) {
                        auto & binary_rules_pl = binary_rules_p[ltag];
                        int num_lsub = tag_set_->numSubtags(ltag, fine_level);

                        for (const BinaryRule * rule : binary_rules_pl) {
                            auto & score_list = rule->getScoreList();
                            int rtag = rule->right();
                            int num_rsub = tag_set_->numSubtags(rtag, fine_level);
                            double old_log_score = maxc_log_score[begin][end][ptag];

                            for (int mid = begin + 1; mid < end; ++mid) {
                                double cur_log_score =
                                    maxc_log_score[begin][mid][ltag] +
                                    maxc_log_score[mid][end][rtag];
                                if (cur_log_score < old_log_score) continue;

                                double rule_score = 0.0;

                                for (int psub = 0; psub < num_psub; ++psub) {
                                    if (!allowed[begin][end][ptag][psub]) continue;
                                    double po = outside[begin][end][ptag][psub];

                                    for (int lsub = 0; lsub < num_lsub; ++lsub) {
                                        if (!allowed[begin][mid][ltag][lsub]) continue;
                                        if (score_list[psub][lsub].empty()) continue;
                                        double li = inside[begin][mid][ltag][lsub];

                                        for (int rsub = 0; rsub < num_rsub; ++rsub) {
                                            if (!allowed[mid][end][rtag][rsub]) continue;
                                            double ri = inside[mid][end][rtag][rsub];
                                            double beta = score_list[psub][lsub][rsub];
                                            rule_score += po * li * ri * beta;
                                        }
                                    }
                                }

                                if (rule_score == 0) continue;

                                cur_log_score += log(rule_score) - log_normalizer;

                                if (cur_log_score > old_log_score) {
                                    old_log_score = cur_log_score;
                                    maxc_log_score[begin][end][ptag] = cur_log_score;
                                    maxc_left[begin][end][ptag] = ltag;
                                    maxc_right[begin][end][ptag] = rtag;
                                    maxc_mid[begin][end][ptag] = mid;
                                }
                            } // mid
                        } // rule
                    } // ltag
                } // ptag
            } else {
                // process lexicon

                for (int tag = 0; tag < num_tags; ++tag) {
                    int wid = wid_list[begin];
                    const LexiconEntry * ent = fine_lexicon.getEntry(tag, wid);
                    if (!ent) continue;
                    int num_sub = ent->numSubtags();

                    double rule_score = 0.0;

                    for (int sub = 0; sub < num_sub; ++sub) {
                        if (!allowed[begin][end][tag][sub]) continue;
                        double po = outside[begin][end][tag][sub];
                        double beta = ent->getScore(sub);
                        rule_score += po * beta;
                    }

                    if (rule_score == 0.0) continue;

                    maxc_log_score[begin][end][tag] = log(rule_score) - log_normalizer;
                }
            }

            // process unary rules
            
            vector<double> after_unary(num_tags);
            for (int tag = 0; tag < num_tags; ++tag) {
                after_unary[tag] = maxc_log_score[begin][end][tag];
            }

            for (int ptag = 0; ptag < num_tags; ++ptag) {
                auto & unary_rules_p = fine_grammar.getUnaryRuleListByPC()[ptag];
                int num_psub = tag_set_->numSubtags(ptag, fine_level);
                
                for (const UnaryRule * rule : unary_rules_p) {
                    int ctag = rule->child();
                    if (ctag == ptag) continue;
                    auto & score_list = rule->getScoreList();
                    int num_csub = tag_set_->numSubtags(ctag, fine_level);

                    double cur_log_score = maxc_log_score[begin][end][ctag];
                    if (cur_log_score < after_unary[ptag]) continue;

                    double rule_score = 0.0;

                    for (int psub = 0; psub < num_psub; ++psub) {
                        if (!allowed[begin][end][ptag][psub]) continue;
                        double po = outside[begin][end][ptag][psub];
                        
                        for (int csub = 0; csub < num_csub; ++csub) {
                            if (!allowed[begin][end][ctag][csub]) continue;
                            double ci = inside[begin][end][ctag][csub];
                            double beta = score_list[psub][csub];
                            rule_score += po * ci * beta;
                        }
                    }

                    if (rule_score == 0.0) continue;

                    cur_log_score += log(rule_score) - log_normalizer;

                    if (cur_log_score > after_unary[ptag]) {
                        after_unary[ptag] = cur_log_score;
                        maxc_child[begin][end][ptag] = ctag;
                    }
                } // rule
            } // ptag

            for (int tag = 0; tag < num_tags; ++tag) {
                maxc_log_score[begin][end][tag] = after_unary[tag];
            }

            /*
            for (int tag  = 0; tag < num_tags; ++tag) {
                if (maxc_log_score[begin][end][tag] == NEG_INFTY) continue;
                if (maxc_child[begin][end][tag] >= 0) {
                    fprintf(stderr, "max-rule[%d:%d] %s -> %s (%e)\n",
                        begin, end,
                        tag_set_->getTagName(tag).c_str(),
                        tag_set_->getTagName(maxc_child[begin][end][tag]).c_str(),
                        maxc_log_score[begin][end][tag]);
                } else if (maxc_left[begin][end][tag] >= 0) {
                    fprintf(stderr, "max-rule[%d:%d] %s -> %s/%d/%s (%e)\n",
                        begin, end,
                        tag_set_->getTagName(tag).c_str(),
                        tag_set_->getTagName(maxc_left[begin][end][tag]).c_str(),
                        maxc_mid[begin][end][tag],
                        tag_set_->getTagName(maxc_right[begin][end][tag]).c_str(),
                        maxc_log_score[begin][end][tag]);
                }
            }
            */
        } // begin
    } // len

    // build max-rule parse tree

    function<Tree<string> * (int, int, int, bool)> buildTree
        = [&](int begin, int end, int ptag, bool first_time) -> Tree<string> * {
        
        if (ptag < 0) return nullptr; // for ptag = -1

        int ctag = maxc_child[begin][end][ptag];

        if (ctag != -1 && first_time) {
            // make unary derivation
            Tree<string> * child_tree = buildTree(begin, end, ctag, false);
            if (!child_tree) return nullptr;
            Tree<string> * parent_tree = new Tree<string>(tag_set_->getTagName(ptag));
            parent_tree->addChild(child_tree);
            return parent_tree;
        } else if (end - begin > 1) {
            // make binary derivation
            int ltag = maxc_left[begin][end][ptag];
            int rtag = maxc_right[begin][end][ptag];
            int mid = maxc_mid[begin][end][ptag];
            Tree<string> * left_tree = buildTree(begin, mid, ltag, true);
            Tree<string> * right_tree = buildTree(mid, end, rtag, true);
            if (!left_tree || !right_tree) {
                delete left_tree;
                delete right_tree;
                return nullptr;
            }
            Tree<string> * parent_tree = new Tree<string>(tag_set_->getTagName(ptag));
            parent_tree->addChild(left_tree);
            parent_tree->addChild(right_tree);
            return parent_tree;
        } else {
            // make lexical/word nodes
            Tree<string> * parent_node = new Tree<string>(tag_set_->getTagName(ptag));
            Tree<string> * word_node = new Tree<string>(text[begin]);
            parent_node->addChild(word_node);
            return parent_node;
        }

        // never come here
    };

    Tree<string> * parse = buildTree(0, num_words, root_tag, true);
    if (parse) return shared_ptr<Tree<string> >(parse);
    else return getDefaultParse();
}

shared_ptr<Tree<string> > LAPCFGParser::getDefaultParse() const {
    return shared_ptr<Tree<string> >(new Tree<string>(""));
}

} // namespace AHCParser

