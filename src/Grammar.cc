#include "Grammar.h"

#include <boost/algorithm/string.hpp>

#include <cmath>
#include <map>
#include <string>
#include <iostream>

using namespace std;

namespace Ckylark {

Grammar::Grammar(const TagSet & tag_set, int level)
    : tag_set_(tag_set)
    , level_(level)
    , binary_parent_(tag_set.numTags())
    //, binary_parent_left_(tag_set.numTags(), vector<vector<BinaryRule *> >(tag_set.numTags()))
    //, binary_parent_right_(tag_set.numTags(), vector<vector<BinaryRule *> >(tag_set.numTags()))
    //, binary_left_right_(tag_set.numTags(), vector<vector<BinaryRule *> >(tag_set.numTags()))
    , unary_parent_(tag_set.numTags())
    , unary_child_(tag_set.numTags()) {
}

Grammar::~Grammar() {
    // delete all binary rules
    for (auto& it1 : binary_parent_) {
        for (auto it2 : it1) {
            delete it2;
        }
    }
    for (auto& it1 : unary_parent_) {
        for (auto it2 : it1) {
            delete it2;
        }
    }
}

shared_ptr<Grammar> Grammar::loadFromStream(std::istream & stream, const TagSet & tag_set) {
    Grammar * grm = new Grammar(tag_set, tag_set.getDepth()-1);
    shared_ptr<Grammar> pgrm(grm);

    BinaryRule * cur_binary = &grm->getBinaryRule(0, 0, 0); // dummy
    UnaryRule * cur_unary = &grm->getUnaryRule(0, 0); // dummy

    string line;
    while (getline(stream, line)) {
        boost::trim(line);
        vector<string> ls;
        boost::split(ls, line, boost::is_any_of("_ "));
        int pc, lc, rc, psc, lsc, rsc;
        double score;

        switch (ls.size()) {
        case 8: // binary
            pc = tag_set.getTagId(ls[0]);
            lc = tag_set.getTagId(ls[3]);
            rc = tag_set.getTagId(ls[5]);
            psc = stoi(ls[1]);
            lsc = stoi(ls[4]);
            rsc = stoi(ls[6]);
            score = stod(ls[7]);
            //cout << "binary: " << pc << ' ' << lc << ' ' << rc << ' ' << psc << ' ' << lsc << ' ' << rsc << endl;
            if (cur_binary->parent() != pc || cur_binary->left() != lc || cur_binary->right() != rc) {
                cur_binary = &grm->getBinaryRule(pc, lc, rc);
            }
            cur_binary->setScore(psc, lsc, rsc, score);
            break;

        case 6: // unary
            pc = tag_set.getTagId(ls[0]);
            lc = tag_set.getTagId(ls[3]);
            psc = stoi(ls[1]);
            lsc = stoi(ls[4]);
            score = stod(ls[5]);
            //cout << "unary: " << pc << ' ' << lc << ' ' << psc << ' ' << lsc << endl;
            if (cur_unary->parent() != pc || cur_unary->child() != lc) {
                cur_unary = &grm->getUnaryRule(pc, lc);
            }
            cur_unary->setScore(psc, lsc, score);
            break;

        default:
            throw runtime_error("Grammar: invalid file format");
        }
    }
    
    return pgrm;
}

BinaryRule & Grammar::getBinaryRule(int parent, int left, int right) {

    auto& rules_p = binary_parent_[parent];
    for (auto* rule : rules_p) {
        if (rule->left() == left && rule->right() == right) {
            return *rule;
        }
    }

    //auto& rules_pl = binary_parent_left_[parent][left];
    //auto& rules_pr = binary_parent_right_[parent][right];
    //auto& rules_lr = binary_left_right_[left][right];
    size_t np = tag_set_.numSubtags(parent, level_);
    size_t nl = tag_set_.numSubtags(left, level_);
    size_t nr = tag_set_.numSubtags(right, level_);
    BinaryRule * rule = new BinaryRule(parent, left, right, np, nl, nr);
    rules_p.push_back(rule);
    //rules_pl.push_back(rule);
    //rules_pr.push_back(rule);
    //rules_lr.push_back(rule);
    return *rule;
}

UnaryRule & Grammar::getUnaryRule(int parent, int child) {
    auto& rules_p = unary_parent_[parent];
    for (auto* rule : rules_p) {
        if (rule->child() == child) {
            return *rule;
        }
    }

    auto& rules_c = unary_child_[child];
    size_t np = tag_set_.numSubtags(parent, level_);
    size_t nc = tag_set_.numSubtags(child, level_);
    UnaryRule * rule = new UnaryRule(parent, child, np, nc);
    rules_p.push_back(rule);
    rules_c.push_back(rule);
    return *rule;
}

} // namespace Ckylark

