#include <ckylark/M1Lexicon.h>

using namespace std;

namespace Ckylark {

M1Lexicon::M1Lexicon(const Dictionary & word_table, const TagSet & tag_set)
    : tag_set_(tag_set)
    , score_(tag_set.numTags(), vector<double>(word_table.size(), 0.0))
    , scaling_() {
}

double M1Lexicon::getScalingFactor(int word_id) const {
    if (scaling_.empty()) {
        // calculate scaling factors
        int num_words = score_[0].size();
        int num_tags = score_.size();
        scaling_.assign(num_words, 0.0);
        for (int wid = 0; wid < num_words; ++wid) {
            for (int tag = 0; tag < num_tags; ++tag) {
                if (score_[tag][wid] > scaling_[wid]) {
                    scaling_[wid] = score_[tag][wid];
                }
            }
        }
    }

    return (scaling_[word_id] > 0.0) ? (1.0 / scaling_[word_id]) : 1.0;
}

} // namespace Ckylark

