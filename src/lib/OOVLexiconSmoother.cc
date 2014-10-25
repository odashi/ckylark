#include <ckylark/OOVLexiconSmoother.h>

#include <stdexcept>

using namespace std;

namespace Ckylark {

OOVLexiconSmoother::OOVLexiconSmoother(const Lexicon & lexicon, double ratio)
    : LexiconSmoother(lexicon)
    , ratio_(ratio)
    , ent_word_(nullptr)
    , ent_oov_(nullptr) {
    
    if (ratio < 0.0 || ratio > 1.0) {
        throw runtime_error("OOVLexiconSmoother::OOVLexiconSmoother: invalid value: ratio");
    }
}

bool OOVLexiconSmoother::prepare(int tag_id, int word_id) {
    auto & lexicon = getLexicon();
    ent_word_ = lexicon.getEntry(tag_id, word_id);
    ent_oov_ = lexicon.getEntry(tag_id, -1);
    return (ent_word_ || ent_oov_);
}

double OOVLexiconSmoother::getScore(int subtag_id) const {
    return
        (1.0 - ratio_) * (ent_word_ ? ent_word_->getScore(subtag_id) : 0.0) +
        ratio_ * (ent_oov_ ? ent_oov_->getScore(subtag_id) : 0.0);
}

} // namespace Ckylark

