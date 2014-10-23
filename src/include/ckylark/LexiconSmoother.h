#ifndef CKYLARK_LEXICON_SMOOTHER_H_
#define CKYLARK_LEXICON_SMOOTHER_H_

#include <ckylark/Lexicon.h>

namespace Ckylark {

// interface of the strategy to smooth lexicon probabilities
class LexiconSmoother {

    LexiconSmoother(const LexiconSmoother &) = delete;
    LexiconSmoother & operator=(const LexiconSmoother &) = delete;

public:
    LexiconSmoother(const Lexicon & lexicon)
        : lexicon_(lexicon) {}

    virtual ~LexiconSmoother() {}

    // preparing the calculation of smoothing
    // if false is returned, smoothed probabilities are 0.
    virtual bool prepare(int tag_id, int word_id) = 0;

    // retrieve the score of specific subclass of the tag
    virtual double getScore(int subtag_id) const = 0;

    // get target lexicon
    const Lexicon & getLexicon() const { return lexicon_; }

private:
    const Lexicon & lexicon_;

}; // class LexiconSmoother

} // namespace Ckylark

#endif // CKYLARK_LEXICON_SMOOTHER_H_

