#ifndef CKYLARK_LEXICON_SMOOTHER_H_
#define CKYLARK_LEXICON_SMOOTHER_H_

#include <ckylark/Lexicon.h>
#include <ckylark/M1Lexicon.h>

namespace Ckylark {

// interface of the strategy to smooth lexicon probabilities.
class LexiconSmoother {

    LexiconSmoother() = delete;
    LexiconSmoother(const LexiconSmoother &) = delete;
    LexiconSmoother & operator=(const LexiconSmoother &) = delete;

public:
    LexiconSmoother(const Lexicon & lexicon)
        : lexicon_(lexicon) {}

    virtual ~LexiconSmoother() {}

    // preparing the calculation of smoothing.
    // if false is returned, smoothed probabilities are 0.
    virtual bool prepare(int tag_id, int word_id) = 0;

    // retrieve the score of specific subclass of the tag.
    virtual double getScore(int subtag_id) const = 0;

    // retrieve the target lexicon.
    inline const Lexicon & getLexicon() const { return lexicon_; }

private:
    const Lexicon & lexicon_;

}; // class LexiconSmoother

// interface of the strategy to smooth G-1 lexicon probabilities.
class M1LexiconSmoother {

    M1LexiconSmoother() = delete;
    M1LexiconSmoother(const M1LexiconSmoother &) = delete;
    M1LexiconSmoother & operator=(const M1LexiconSmoother &) = delete;

public:
    M1LexiconSmoother(const M1Lexicon & lexicon)
        : lexicon_(lexicon) {}
    
    virtual ~M1LexiconSmoother() {}

    // retrieve the score of specific tag.
    virtual double getScore(int tag_id, int word_id) = 0;

    // retrieve the target lexicon.
    inline const M1Lexicon & getLexicon() const { return lexicon_; }

private:
    const M1Lexicon & lexicon_;

}; // class M1LexiconSmoother

} // namespace Ckylark

#endif // CKYLARK_LEXICON_SMOOTHER_H_

