#include <ckylark/OOVLexiconSmoother.h>

#include <ckylark/StringUtil.h>

#include <stdexcept>

using namespace std;

namespace Ckylark {

OOVLexiconSmoother::OOVLexiconSmoother(const Lexicon & lexicon, const Dictionary & word_table, double ratio)
    : LexiconSmoother(lexicon)
    , ratio_(ratio)
    , cur_ent_(nullptr)
    , cur_tag_(-1) {
    
    if (ratio < 0.0 || ratio > 1.0) {
        throw runtime_error("OOVLexiconSmoother::OOVLexiconSmoother: invalid value: ratio");
    }

    const TagSet & tag_set = lexicon.getTagSet();
    int level = lexicon.getLevel();
    int num_tags = tag_set.numTags();

    // generate OOV lexicon entries
    for (int tag = 0; tag < num_tags; ++tag) {
        oov_entries_.push_back(new LexiconEntry(tag, -1, tag_set.numSubtags(tag, level)));
    }

    // sum all UNK* lexicon probabilities
    for (const string & word : word_table.getWordList()) {
        if (StringUtil::startsWith(word, "UNK")) {
            int wid = word_table.getId(word);

            for (int tag = 0; tag < num_tags; ++tag) {
                const LexiconEntry * ent_from = lexicon.getEntry(tag, wid);
                if (!ent_from) continue;
                LexiconEntry & ent_to = *oov_entries_[tag];
                int num_subs = tag_set.numSubtags(tag, level);

                for (int sub = 0; sub < num_subs; ++sub)
                    ent_to.addScore(sub, ent_from->getScore(sub));
            }
        }
    }
}

OOVLexiconSmoother::~OOVLexiconSmoother() {
    for (auto * entry : oov_entries_) {
        delete entry;
    }
}

bool OOVLexiconSmoother::prepare(int tag_id, int word_id) {
    auto & lexicon = getLexicon();
    cur_ent_ = lexicon.getEntry(tag_id, word_id);
    cur_tag_ = tag_id;
    return true;
}

double OOVLexiconSmoother::getScore(int subtag_id) const {
    return
        (1.0 - ratio_) * (cur_ent_ ? cur_ent_->getScore(subtag_id) : 0.0) +
        ratio_ * oov_entries_[cur_tag_]->getScore(subtag_id);
}

} // namespace Ckylark

