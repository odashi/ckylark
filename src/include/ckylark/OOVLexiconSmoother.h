#ifndef CKYLARK_OOV_LEXICON_SMOOTHER_H_
#define CKYLARK_OOV_LEXICON_SMOOTHER_H_

#include <ckylark/LexiconSmoother.h>

namespace Ckylark {

// OOV probability based lexicon smoothing
class OOVLexiconSmoother : public LexiconSmoother {

    OOVLexiconSmoother(const OOVLexiconSmoother &) = delete;
    OOVLexiconSmoother & operator=(const OOVLexiconSmoother &) = delete;

public:
    OOVLexiconSmoother(const Lexicon & lexicon, double ratio);

    bool prepare(int tag_id, int word_id);
    double getScore(int subtag_id) const;

private:
    double ratio_;
    const LexiconEntry * ent_word_;
    const LexiconEntry * ent_oov_;

}; // class OOVLexiconSmoother

} // namespace Ckylark

#endif // CKYLARK_OOV_LEXICON_SMOOTHER_H_

