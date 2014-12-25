#ifndef CKYLARK_OOV_LEXICON_SMOOTHER_H_
#define CKYLARK_OOV_LEXICON_SMOOTHER_H_

#include <ckylark/LexiconSmoother.h>

#include <ckylark/Dictionary.h>

namespace Ckylark {

// OOV probability based lexicon smoothing
class OOVLexiconSmoother : public LexiconSmoother {

    OOVLexiconSmoother() = delete;
    OOVLexiconSmoother(const OOVLexiconSmoother &) = delete;
    OOVLexiconSmoother & operator=(const OOVLexiconSmoother &) = delete;

public:
    OOVLexiconSmoother(
        const Lexicon & lexicon,
        const Dictionary & word_table,
        double ratio);
    ~OOVLexiconSmoother();

    bool prepare(int tag_id, int word_id);
    double getScore(int subtag_id) const;

private:
    double ratio_;
    std::vector<LexiconEntry *> oov_entries_;

    const LexiconEntry * cur_ent_;
    int cur_tag_;

}; // class OOVLexiconSmoother

// OOV probabiliry based G-1 lexicon smoothing
class M1OOVLexiconSmoother : public M1LexiconSmoother {

    M1OOVLexiconSmoother() = delete;
    M1OOVLexiconSmoother(const M1OOVLexiconSmoother &) = delete;
    M1OOVLexiconSmoother & operator=(const M1OOVLexiconSmoother &) = delete;

public:
    M1OOVLexiconSmoother(
        const M1Lexicon & lexicon,
        const Dictionary & word_table, 
        double ratio);
    ~M1OOVLexiconSmoother();

    double getScore(int tag_id, int word_id);

private:
    double ratio_;
    std::vector<double> oov_scores_;

}; // class M1OOVLexiconSmoother

} // namespace Ckylark

#endif // CKYLARK_OOV_LEXICON_SMOOTHER_H_

