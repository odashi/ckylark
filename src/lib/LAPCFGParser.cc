#include <ckylark/LAPCFGParser.h>

#include <ckylark/Mapping.h>
#include <ckylark/ModelProjector.h>
#include <ckylark/Tracer.h>
#include <ckylark/BerkeleySignatureEstimator.h>
#include <ckylark/MaxScalingFactor.h>
#include <ckylark/OOVLexiconSmoother.h>
#include <ckylark/StreamFactory.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <cstdio>
#include <cmath>
#include <functional>
#include <fstream>
#include <limits>
#include <stdexcept>

#include <iostream> // for debug

using namespace std;

namespace Ckylark {

LAPCFGParser::LAPCFGParser()
    : fine_level_(-1)
    , prune_threshold_(1e-5)
    , smooth_unklex_(0)
    , scaling_factor_(1.0) {
}

LAPCFGParser::~LAPCFGParser() {}

shared_ptr<LAPCFGParser> LAPCFGParser::loadFromBerkeleyDump(const string & path) {
    shared_ptr<LAPCFGParser> parser(new LAPCFGParser());

    parser->loadWordTable(path + ".words");
    parser->loadTagSet(path + ".splits");
    parser->loadLexicon(path + ".lexicon");
    parser->loadGrammar(path + ".grammar");
    parser->generateCoarseModels();
    parser->setFineLevel(-1);

    parser->sig_est_.reset(new BerkeleySignatureEstimator(
        BerkeleySignatureEstimator::English,
        *parser->word_table_));

    int finest_level = parser->tag_set_->getDepth() - 1;
    auto & finest_lexicon = parser->getLexicon(finest_level);
    auto & finest_grammar = parser->getGrammar(finest_level);
    parser->scaling_factor_ = MaxScalingFactor().calculate(finest_lexicon, finest_grammar);

    Tracer::println(1, (boost::format("Scaling Factor: %.6e") % parser->scaling_factor_).str());

    return parser;
}

void LAPCFGParser::loadWordTable(const string & path) {
    Tracer::println(1, "Loading words: " + path + " ...");
    shared_ptr<InputStream> ifs = StreamFactory::createInputStream(path);

    word_table_.reset(new Dictionary());
    string line;
    while (ifs->readLine(line)) {
        boost::trim(line);
        //if (line.size() >= 3 && line.substr(0, 3) == "UNK") {
        //    // skip UNK* entries
        //    continue;
        //}
        word_table_->addWord(line);
    }
}

void LAPCFGParser::loadTagSet(const string & path) {
    Tracer::println(1, "Loading tags: " + path + " ...");
    shared_ptr<InputStream> ifs = StreamFactory::createInputStream(path);
    tag_set_ = TagSet::loadFromStream(*ifs);
}

void LAPCFGParser::loadLexicon(const string & path) {
    Tracer::println(1, "Loading lexicon: " + path + " ...");
    shared_ptr<InputStream> ifs = StreamFactory::createInputStream(path);
    shared_ptr<Lexicon> lexicon(Lexicon::loadFromStream(*ifs, *word_table_, *tag_set_));
    lexicon_.push_back(lexicon);
}

void LAPCFGParser::loadGrammar(const string & path) {
    Tracer::println(1, "Loading grammar: " + path + " ...");
    shared_ptr<InputStream> ifs = StreamFactory::createInputStream(path);
    shared_ptr<Grammar> grammar(Grammar::loadFromStream(*ifs, *tag_set_));
    grammar_.push_back(grammar);
}

void LAPCFGParser::generateCoarseModels() {
    const int depth = tag_set_->getDepth();

    for (int level = depth-2; level >= 0; --level) {
        Tracer::println(1, (boost::format("Generating coarse model (level=%d) ...") % level).str());
        
        //int fine = grammar_.size() - 1;
        ModelProjector projector(*tag_set_, *(lexicon_[0]), *(grammar_[0]), level+1, level);
        lexicon_.insert(lexicon_.begin(), projector.generateLexicon());
        grammar_.insert(grammar_.begin(), projector.generateGrammar());
    }
}

ParserResult LAPCFGParser::parse(const vector<string> & sentence) const {
    ParserResult result = generateMaxRuleOneBestParse(sentence, fine_level_);

    // if full-level parsing is failed, rollback coarse grammar and retry parsing
    if (!result.succeeded && result.final_level > 0) {
        result = generateMaxRuleOneBestParse(sentence, result.final_level - 1);
    }

    return result;
}

ParserResult LAPCFGParser::generateMaxRuleOneBestParse(
    const vector<string> & sentence,
    int final_level_to_try) const {
    
    const int num_words = sentence.size();
    const int num_tags = tag_set_->numTags();
    const int root_tag = tag_set_->getTagId("ROOT");

    // check empty sentence
    if (num_words == 0) {
        return ParserResult { getDefaultParse(), true, -1 };
    }

    vector<int> wid_list = makeWordIdList(sentence);
    vector<int> tid_list = makeTagIdList(sentence);

    CKYTable<bool> allowed_tag(num_words, num_tags);
    CKYTable<vector<bool> > allowed_sub(num_words, num_tags);
    CKYTable<vector<double> > inside(num_words, num_tags);
    CKYTable<vector<double> > outside(num_words, num_tags);
    vector<vector<Extent> > extent(num_words + 1, vector<Extent>(num_tags, {
        num_words + 1, // narrow_right
        -1, // narrow_left
        -1, // wide_right
        num_words + 1 })); // wide_left
    
    // pre-parsing

    for (int level = 0; level <= final_level_to_try; ++level) {
        initializeCharts(allowed_tag, allowed_sub, inside, outside, extent, level);
        //cout << "  init" << endl;
        setTerminalScores(allowed_tag, allowed_sub, inside, wid_list, level);
        //cout << "  lexicon" << endl;
        calculateInsideScores(allowed_tag, allowed_sub, inside, extent, level);
        //cout << "  inside" << endl;

        // check if all possible parses are pruned
        double sentence_score = inside.at(0, num_words, root_tag)[0];
        if (sentence_score == 0.0) {
            Tracer::println(1, (boost::format("  No any possible parses (level=%d).") % level).str());
            return ParserResult { getDefaultParse(), false, level };
        }
        //cout << "  check" << endl;

        calculateOutsideScores(allowed_tag, allowed_sub, inside, outside, extent, level);
        //cout << "  outside" << endl;
        pruneCharts(allowed_tag, allowed_sub, inside, outside, level);
        //cout << "  prune" << endl;

        //fprintf(stderr, "pre-parse %d ... ROOT: %e\n", level, inside.at(0, num_words, root_tag)[0]);
    } // level

    // retrieve max-rule parse over allowed nodes
    
    CKYTable<double> maxc_log_score(num_words, num_tags);
    CKYTable<int> maxc_left(num_words, num_tags);
    CKYTable<int> maxc_right(num_words, num_tags);
    CKYTable<int> maxc_mid(num_words, num_tags);
    CKYTable<int> maxc_child(num_words, num_tags);
    const double log_normalizer = log(inside.at(0, num_words, root_tag)[0]);
    const double NEG_INFTY = -1e20;
    const Lexicon & fine_lexicon = getLexicon(final_level_to_try);
    const Grammar & fine_grammar = getGrammar(final_level_to_try);
    OOVLexiconSmoother smoother(fine_lexicon, *word_table_, smooth_unklex_);

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
                    if (!allowed_tag.at(begin, end, ptag)) continue;
                    if (fine_lexicon.hasEntry(ptag)) continue; // semi-terminal
                    auto & binary_rules_p = fine_grammar.getBinaryRuleList(ptag);
                    int num_psub = tag_set_->numSubtags(ptag, final_level_to_try);

                    for (const BinaryRule * rule : binary_rules_p) {
                        auto & score_list = rule->getScoreList();
                        int ltag = rule->left();
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

                        int num_lsub = tag_set_->numSubtags(ltag, final_level_to_try);
                        int num_rsub = tag_set_->numSubtags(rtag, final_level_to_try);
                        double old_log_score = maxc_log_score.at(begin, end, ptag);

                        for (int mid = min; mid <= max; ++mid) {
                            if (!allowed_tag.at(begin, mid, ltag)) continue;
                            if (!allowed_tag.at(mid, end, rtag)) continue;
                            if (mid - begin > 1 && fine_lexicon.hasEntry(ltag)) continue; // semi-terminal
                            if (end - mid > 1 && fine_lexicon.hasEntry(rtag)) continue; // semi-terminal
                            double cur_log_score =
                                maxc_log_score.at(begin, mid, ltag) +
                                maxc_log_score.at(mid, end, rtag);
                            if (cur_log_score < old_log_score) continue;

                            double rule_score = 0.0;

                            for (int psub = 0; psub < num_psub; ++psub) {
                                if (!allowed_sub.at(begin, end, ptag)[psub]) continue;
                                auto & score_list_p = score_list[psub];
                                if (score_list_p.empty()) continue;

                                double po = outside.at(begin, end, ptag)[psub];

                                for (int lsub = 0; lsub < num_lsub; ++lsub) {
                                    if (!allowed_sub.at(begin, mid, ltag)[lsub]) continue;
                                    auto & score_list_pl = score_list_p[lsub];
                                    if (score_list_pl.empty()) continue;
                                    double li = inside.at(begin, mid, ltag)[lsub];

                                    for (int rsub = 0; rsub < num_rsub; ++rsub) {
                                        if (!allowed_sub.at(mid, end, rtag)[rsub]) continue;
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
                        }
                    }
                }
            } else {
                // process lexicon
                int wid = wid_list[begin];
                double word_scaling = fine_lexicon.getScalingFactor(wid);

                for (int tag = 0; tag < num_tags; ++tag) {
                    if (!allowed_tag.at(begin, end, tag)) continue;
                    if (!smoother.prepare(tag, wid)) continue;
                    int num_sub = tag_set_->numSubtags(tag, final_level_to_try);
                    double rule_score = 0.0;

                    for (int sub = 0; sub < num_sub; ++sub) {
                        if (!allowed_sub.at(begin, end, tag)[sub]) continue;
                        double po = outside.at(begin, end, tag)[sub];
                        double beta = word_scaling * smoother.getScore(sub);
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
                if (!allowed_tag.at(begin, end, ptag)) continue;
                if (fine_lexicon.hasEntry(ptag)) continue; // semi-terminal
                auto & unary_rules_p = fine_grammar.getUnaryRuleListByPC()[ptag];
                int num_psub = tag_set_->numSubtags(ptag, final_level_to_try);
                
                for (const UnaryRule * rule : unary_rules_p) {
                    int ctag = rule->child();
                    if (!allowed_tag.at(begin, end, ctag)) continue;
                    if (len > 1 && fine_lexicon.hasEntry(ctag)) continue; // semi-terminal
                    if (ctag == ptag) continue;
                    auto & score_list = rule->getScoreList();
                    int num_csub = tag_set_->numSubtags(ctag, final_level_to_try);

                    double cur_log_score = maxc_log_score.at(begin, end, ctag);
                    if (cur_log_score < after_unary[ptag]) continue;

                    double rule_score = 0.0;

                    for (int psub = 0; psub < num_psub; ++psub) {
                        if (!allowed_sub.at(begin, end, ptag)[psub]) continue;
                        auto & score_list_p = score_list[psub];
                        if (score_list_p.empty()) continue;
                        double po = outside.at(begin, end, ptag)[psub];
                        
                        for (int csub = 0; csub < num_csub; ++csub) {
                            if (!allowed_sub.at(begin, end, ctag)[csub]) continue;
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

    function<Tree<string> *(int, int, int, bool, int)> buildTree
        = [&](int begin, int end, int ptag, bool first_time, int depth) -> Tree<string> * {

        //for (int i = 0; i < depth; ++i) cout << ".  ";
        //cout << begin << ' ' << end << ' ' << tag_set_->getTagName(ptag) << endl;
        
        if (ptag < 0) return nullptr; // for ptag = -1

        int ctag = maxc_child.at(begin, end, ptag);
        if (ctag == -1 && ptag == root_tag) return nullptr;

        Tree<string> * parent_tree = nullptr;

        if (ctag != -1 && first_time) {
            // make unary derivation

            Tree<string> * child_tree = buildTree(begin, end, ctag, false, depth + 1);

            if (child_tree) {
                parent_tree = new Tree<string>(tag_set_->getTagName(ptag));
                addChildOrCoalesce(*parent_tree, child_tree);
            }
        } else if (end - begin > 1) {
            // make binary derivation
            
            int ltag = maxc_left.at(begin, end, ptag);
            int rtag = maxc_right.at(begin, end, ptag);
            int mid = maxc_mid.at(begin, end, ptag);

            Tree<string> * left_tree = buildTree(begin, mid, ltag, true, depth + 1);
            Tree<string> * right_tree = buildTree(mid, end, rtag, true, depth + 1);
            
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

    Tree<string> * parse = buildTree(0, num_words, root_tag, true, 0);
    if (parse) {
        return ParserResult { shared_ptr<Tree<string> >(parse), true, final_level_to_try };
    } else {
        Tracer::println(1, "  No any possible max-rule parse.");
        return ParserResult { getDefaultParse(), false, final_level_to_try };
    }
}

void LAPCFGParser::setFineLevel(int value) {
    int depth = tag_set_->getDepth();
    if (value < 0 || value >= depth) {
        fine_level_ = depth - 1;
    } else {
        fine_level_ = value;
    }
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

vector<int> LAPCFGParser::makeWordIdList(const vector<string> & sentence) const {
    const int num_words = sentence.size();
    vector<int> wid_list(num_words);
    
    Tracer::print(2, "  WID:");

    for (int i = 0; i < num_words; ++i) {
        int wid = word_table_->getId(sentence[i]);
        if (wid == -1) {
            // estimate signature
            string signature = sig_est_->getSignature(sentence, i);
            wid = word_table_->getId(signature);
            Tracer::print(2, (boost::format(" %d(%s)") % wid % signature).str());
        } else {
            Tracer::print(2, (boost::format(" %d") % wid).str());
        }
        wid_list[i] = wid;
    }
    
    Tracer::println(2);
    return wid_list;
}

vector<int> LAPCFGParser::makeTagIdList(const vector<string> & sentence) const {
    const int num_words = sentence.size();
    vector<int> tid_list(num_words);

    Tracer::print(2, "  TID:");

    for (int i = 0; i < num_words; ++i) {
        const string & word = sentence[i];
        int tid = -1;
        // grammar tag representation is [FOO]
        if (word.size() >= 2 && word[0] == '[' && word[word.size() - 1] == ']') {
            const string name = word.substr(1, word.size() - 2);
            tid = tag_set_->getTagId(name);
            Tracer::print(2, (boost::format(" %d(%s)") % tid % name).str());
        } else {
            Tracer::print(2, (boost::format(" %d") % tid).str());
        }
        tid_list[i] = tid;
    }

    Tracer::println(2);
    return tid_list;
}

void LAPCFGParser::initializeCharts(
    CKYTable<bool> & allowed_tag,
    CKYTable<vector<bool> > & allowed_sub,
    CKYTable<vector<double> > & inside,
    CKYTable<vector<double> > & outside,
    vector<vector<Extent> > & extent,
    int cur_level) const {

    const int num_words = allowed_tag.numWords();
    const int num_tags = allowed_tag.numTags();
   
    unique_ptr<Mapping> mapping(nullptr);
    if (cur_level > 0) {
        mapping.reset(new Mapping(*tag_set_, cur_level - 1, cur_level));
    }

    for (int begin = 0; begin < num_words; ++begin) {
        for (int end = begin + 1; end <= num_words; ++end) {
            for (int tag = 0; tag < num_tags; ++tag) {
                if (cur_level > 0) {
                    if (allowed_tag.at(begin, end, tag)) {
                        // initialize subtag constraints
                        int num_subtags_fine = tag_set_->numSubtags(tag, cur_level);
                        inside.at(begin, end, tag).assign(num_subtags_fine, 0.0);
                        outside.at(begin, end, tag).assign(num_subtags_fine, 0.0);
                        int num_subtags_coarse = tag_set_->numSubtags(tag, cur_level - 1);
                        vector<bool> & allowed_sub_fine = allowed_sub.at(begin, end, tag);
                        vector<bool> allowed_sub_coarse = allowed_sub_fine;
                        allowed_sub_fine.assign(num_subtags_fine, false);
                        
                        // do coarse-to-fine mapping
                        for (int subtag_coarse = 0; subtag_coarse < num_subtags_coarse; ++subtag_coarse) {
                            if (!allowed_sub_coarse[subtag_coarse]) continue;
                            for (int subtag_fine : mapping->getCoarseToFineMaps(tag, subtag_coarse)) {
                                allowed_sub_fine[subtag_fine] = true;
                            }
                        }
                    } else {
                        // delete unnecessary data to suppress memory
                        inside.at(begin, end, tag).clear();
                        outside.at(begin, end, tag).clear();
                    }
                } else {
                    // only 1 subtag is possible for the first time
                    inside.at(begin, end, tag).assign(1, 0.0);
                    outside.at(begin, end, tag).assign(1, 0.0);
                    allowed_tag.at(begin, end, tag) = true;
                    allowed_sub.at(begin, end, tag).assign(1, true);
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

void LAPCFGParser::setTerminalScores(
    const CKYTable<bool> & allowed_tag,
    const CKYTable<vector<bool> > & allowed_sub,
    CKYTable<vector<double> > & inside,
    const vector<int> & wid_list,
    int cur_level) const {

    const int num_words = allowed_tag.numWords();
    const int num_tags = allowed_tag.numTags();
    const Lexicon & cur_lexicon = getLexicon(cur_level);
    OOVLexiconSmoother smoother(cur_lexicon, *word_table_, smooth_unklex_);

    for (int begin = 0; begin < num_words; ++begin) {
        int end = begin + 1;
        int wid = wid_list[begin];
        double word_scaling = cur_lexicon.getScalingFactor(wid);
        
        for (int tag = 0; tag < num_tags; ++tag) {
            if (!allowed_tag.at(begin, end, tag)) continue;
            if (!smoother.prepare(tag, wid)) continue;
            int num_sub = tag_set_->numSubtags(tag, cur_level);
            
            for (int sub = 0; sub < num_sub; ++sub) {
                if (!allowed_sub.at(begin, end, tag)[sub]) continue;
                inside.at(begin, end, tag)[sub] = word_scaling * smoother.getScore(sub);
                
                //cerr << begin << "(" << sentence[begin] << ")->"
                //    << tag_set_->getTagName(tag) << "[" << sub << "] = "
                //    << inside.at(begin, end, tag)[sub] << endl;
            }
        }
    }
}

void LAPCFGParser::calculateInsideScores(
    const CKYTable<bool> & allowed_tag,
    const CKYTable<vector<bool> > & allowed_sub,
    CKYTable<vector<double> > & inside,
    vector<vector<Extent> > & extent,
    int cur_level) const {

    const int num_words = allowed_tag.numWords();
    const int num_tags = allowed_tag.numTags();
    const Lexicon & cur_lexicon = getLexicon(cur_level);
    const Grammar & cur_grammar = getGrammar(cur_level);

    for (int len = 1; len <= num_words; ++len) {
        for (int begin = 0; begin < num_words - len + 1; ++begin) {
            int end = begin + len;

            // process binary rules

            if (len > 1) {
                for (int ptag = 0; ptag < num_tags; ++ptag) {
                    if (!allowed_tag.at(begin, end, ptag)) continue;
                    if (cur_lexicon.hasEntry(ptag)) continue; // semi-terminal
                    auto & binary_rules_p = cur_grammar.getBinaryRuleList(ptag);
                    int num_psub = tag_set_->numSubtags(ptag, cur_level);
                    bool changed = false;
                
                    for (int psub = 0; psub < num_psub; ++psub) {
                        if (!allowed_sub.at(begin, end, ptag)[psub]) continue;
                        double sum = 0.0;

                        for (const BinaryRule * rule : binary_rules_p) {
                            int ltag = rule->left();
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

                            int num_lsub = tag_set_->numSubtags(ltag, cur_level);
                            int num_rsub = tag_set_->numSubtags(rtag, cur_level);
                            auto & score_list_p = rule->getScoreList()[psub];
                            if (score_list_p.empty()) continue;
                            
                            for (int mid = min; mid <= max; ++mid) {
                                if (!allowed_tag.at(begin, mid, ltag)) continue;
                                if (!allowed_tag.at(mid, end, rtag)) continue;
                                if (mid - begin > 1 && cur_lexicon.hasEntry(ltag)) continue; // semi-terminal
                                if (end - mid > 1 && cur_lexicon.hasEntry(rtag)) continue; // semi-terminal

                                auto & allowed_sub_lsubs = allowed_sub.at(begin, mid, ltag);
                                auto & allowed_sub_rsubs = allowed_sub.at(mid, end, rtag);
                                auto & inside_lsubs = inside.at(begin, mid, ltag);
                                auto & inside_rsubs = inside.at(mid, end, rtag);
                        
                                for (int lsub = 0; lsub < num_lsub; ++lsub) {
                                    if (!allowed_sub_lsubs[lsub]) continue;
                                    auto & score_list_pl = score_list_p[lsub];
                                    if (score_list_pl.empty()) continue;
                                    double left_score = inside_lsubs[lsub];
                                    if (left_score == 0.0) continue;
                    
                                    for (int rsub = 0; rsub < num_rsub; ++rsub) {
                                        if (!allowed_sub_rsubs[rsub]) continue;
                                        double rule_score = score_list_pl[rsub];
                                        if (rule_score == 0.0) continue;
                                        double right_score = inside_rsubs[rsub];
                                        if (right_score == 0.0) continue;
                            
                                        sum += scaling_factor_ * rule_score * left_score * right_score;
                                        changed = true;
                                    }
                                }
                            }
                        }

                        inside.at(begin, end, ptag)[psub] = sum;
                        /*
                        cout << (boost::format("%d : %3d-%3d : %8s %3d = %.6e")
                            % cur_level
                            % begin % end % tag_set_->getTagName(ptag) % psub
                            % inside.at(begin, end, ptag)[psub]) << endl;
                        */
                    } // psub

                    if (!changed) continue;

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
                if (!allowed_tag.at(begin, end, ptag)) continue;
                if (cur_lexicon.hasEntry(ptag)) continue; // semi-terminal
                auto & unary_rules_p = cur_grammar.getUnaryRuleListByPC()[ptag];
                int num_psub = tag_set_->numSubtags(ptag, cur_level);
                delta_unary[ptag].assign(num_psub, 0.0);
                
                for (int psub = 0; psub < num_psub; ++psub) {
                    if (!allowed_sub.at(begin, end, ptag)[psub]) continue;

                    for (const UnaryRule * rule : unary_rules_p) {
                        int ctag = rule->child();
                        if (!allowed_tag.at(begin, end, ctag)) continue;
                        if (len > 1 && cur_lexicon.hasEntry(ctag)) continue; // semi-terminal
                        if (ctag == ptag) continue;
                        int num_csub = tag_set_->numSubtags(ctag, cur_level);
                        auto & score_list_p = rule->getScoreList()[psub];
                        if (score_list_p.empty()) continue;
                        
                        for (int csub = 0; csub < num_csub; ++csub) {
                            if (!allowed_sub.at(begin, end, ctag)[csub]) continue;
                            delta_unary[ptag][psub] +=
                                score_list_p[csub] *
                                inside.at(begin, end, ctag)[csub];
                        }
                    }
                }
            }

            for (int ptag = 0; ptag < num_tags; ++ptag) {
                if (!allowed_tag.at(begin, end, ptag)) continue;
                if (cur_lexicon.hasEntry(ptag)) continue; // semi-terminal
                int num_psub = tag_set_->numSubtags(ptag, cur_level);
                for (int psub = 0; psub < num_psub; ++psub) {
                    if (!allowed_sub.at(begin, end, ptag)[psub]) continue;
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
    const CKYTable<bool> & allowed_tag,
    const CKYTable<vector<bool> > & allowed_sub,
    const CKYTable<vector<double> > & inside,
    CKYTable<vector<double> > & outside,
    vector<vector<Extent> > & extent,
    int cur_level) const {

    const int num_words = allowed_tag.numWords();
    const int num_tags = allowed_tag.numTags();
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
                if (!allowed_tag.at(begin, end, ctag)) continue;
                if (len > 1 && cur_lexicon.hasEntry(ctag)) continue; // semi-terminal
                auto & unary_rules_c = cur_grammar.getUnaryRuleListByCP()[ctag];
                int num_csub = tag_set_->numSubtags(ctag, cur_level);
                delta_unary[ctag].assign(num_csub, 0.0);

                for (int csub = 0; csub < num_csub; ++csub) {
                    if (!allowed_sub.at(begin, end, ctag)[csub]) continue;
                    
                    for (const UnaryRule * rule : unary_rules_c) {
                        int ptag = rule->parent();
                        if (!allowed_tag.at(begin, end, ptag)) continue;
                        if (ptag == ctag) continue;
                        int num_psub = tag_set_->numSubtags(ptag, cur_level);
                        auto & score_list = rule->getScoreList();

                        for (int psub = 0; psub < num_psub; ++psub) {
                            if (!allowed_sub.at(begin, end, ptag)[psub]) continue;
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
                if (!allowed_tag.at(begin, end, ctag)) continue;
                if (len > 1 && cur_lexicon.hasEntry(ctag)) continue; // semi-terminal
                int num_csub = tag_set_->numSubtags(ctag, cur_level);
                for (int csub = 0; csub < num_csub; ++csub) {
                    outside.at(begin, end, ctag)[csub] += delta_unary[ctag][csub];
                }
            }

            // process binary rules

            if (len > 1) {
                for (int ptag = 0; ptag < num_tags; ++ptag) {
                    if (!allowed_tag.at(begin, end, ptag)) continue;
                    if (cur_lexicon.hasEntry(ptag)) continue; // semi-terminal
                    auto & binary_rules_p = cur_grammar.getBinaryRuleList(ptag);
                    int num_psub = tag_set_->numSubtags(ptag, cur_level);

                    for (int psub = 0; psub < num_psub; ++psub) {
                        if (!allowed_sub.at(begin, end, ptag)[psub]) continue;
                        double parent_score = outside.at(begin, end, ptag)[psub];
                        if (parent_score == 0.0) continue;

                        for (const BinaryRule * rule : binary_rules_p) {
                            int ltag = rule->left();
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

                            int num_lsub = tag_set_->numSubtags(ltag, cur_level);
                            int num_rsub = tag_set_->numSubtags(rtag, cur_level);
                            auto & score_list_p = rule->getScoreList()[psub];
                            if (score_list_p.empty()) continue;

                            for (int mid = min; mid <= max; ++mid) {
                                if (!allowed_tag.at(begin, mid, ltag)) continue;
                                if (!allowed_tag.at(mid, end, rtag)) continue;
                                if (mid - begin > 1 && cur_lexicon.hasEntry(ltag)) continue; // semi-terminal
                                if (end - mid > 1 && cur_lexicon.hasEntry(rtag)) continue; // semi-terminal

                                auto & allowed_sub_lsubs = allowed_sub.at(begin, mid, ltag);
                                auto & allowed_sub_rsubs = allowed_sub.at(mid, end, rtag);
                                auto & inside_lsubs = inside.at(begin, mid, ltag);
                                auto & inside_rsubs = inside.at(mid, end, rtag);
                                auto & outside_lsubs = outside.at(begin, mid, ltag);
                                auto & outside_rsubs = outside.at(mid, end, rtag);

                                for (int lsub = 0; lsub < num_lsub; ++lsub) {
                                    if (!allowed_sub_lsubs[lsub]) continue;
                                    auto & score_list_pl = score_list_p[lsub];
                                    if (score_list_pl.empty()) continue;
                                    double left_score = inside_lsubs[lsub];
                                    if (left_score == 0.0) continue;

                                    for (int rsub = 0; rsub < num_rsub; ++rsub) {
                                        if (!allowed_sub_rsubs[rsub]) continue;
                                        double rule_score = score_list_pl[rsub];
                                        if (rule_score == 0.0) continue;
                                        double right_score = inside_rsubs[rsub];
                                        if (right_score == 0.0) continue;

                                        double common = scaling_factor_ * rule_score * parent_score;
                                        outside_lsubs[lsub] += common * right_score;
                                        outside_rsubs[rsub] += common * left_score;
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
    CKYTable<bool> & allowed_tag,
    CKYTable<vector<bool> > & allowed_sub,
    const CKYTable<vector<double> > & inside,
    const CKYTable<vector<double> > & outside,
    int cur_level) const {
    
    const int num_words = allowed_tag.numWords();
    const int num_tags = allowed_tag.numTags();
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
                if (!allowed_tag.at(begin, end, tag)) continue;

                int num_sub = tag_set_->numSubtags(tag, cur_level);
                bool joined = false;

                for (int sub = 0; sub < num_sub; ++sub) {
                    double posterior =
                        inside.at(begin, end, tag)[sub] *
                        outside.at(begin, end, tag)[sub] /
                        sentence_score;
                    if (posterior < prune_threshold_) {
                        allowed_sub.at(begin, end, tag)[sub] = false;
                        //++num_pruned;
                    }

                    joined = joined || allowed_sub.at(begin, end, tag)[sub];

                    //if (score > best_score) {
                    //    best_score = score;
                    //    best_tag = tag;
                    //    best_sub = sub;
                    //}
                }

                allowed_tag.at(begin, end, tag) = joined;
            }

            //fprintf(stderr, "best[%d:%d] ... %s[%d] = %e\n",
            //    begin, end, tag_set_->getTagName(best_tag).c_str(), best_sub, best_score);
            
        } // begin
    } // len

    //cerr << "pruned: " << num_pruned << endl;
}

} // namespace Ckylark

