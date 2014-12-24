#ifndef CKYLARK_M1_MODEL_PROJECTOR_H_
#define CKYLARK_M1_MODEL_PROJECTOR_H_

#include <ckylark/Dictionary.h>
#include <ckylark/TagSet.h>
#include <ckylark/Lexicon.h>
#include <ckylark/Grammar.h>
#include <ckylark/M1Lexicon.h>
#include <ckylark/M1Grammar.h>

#include <memory>
#include <vector>

namespace Ckylark {

class M1ModelProjector {

    M1ModelProjector() = delete;
    M1ModelProjector(const M1ModelProjector &) = delete;
    M1ModelProjector & operator=(const M1ModelProjector &) = delete;

public:
    M1ModelProjector(
        const Dictionary & word_table,
        const TagSet & tag_set,
        const Lexicon & g0_lexicon,
        const Grammar & g0_grammar);
    ~M1ModelProjector();

    std::shared_ptr<M1Lexicon> generateLexicon() const;
    std::shared_ptr<M1Grammar> generateGrammar() const;

private:
    const Dictionary & word_table_;
    const TagSet & tag_set_;
    const Lexicon & lexicon_;
    const Grammar & grammar_;
    std::vector<double> cond_prob_;

}; // class M1ModelProjector

} // namespace Ckylark

#endif // CKYLARK_M1_MODEL_PROJECTOR_H_

