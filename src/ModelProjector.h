#ifndef CKYLARK_MODEL_PROJECTOR_H_
#define CKYLARK_MODEL_PROJECTOR_H_

#include "TagSet.h"
#include "Grammar.h"
#include "Lexicon.h"
#include "Mapping.h"

#include <memory>
#include <vector>

namespace Ckylark {

class ModelProjector {

public:
    ModelProjector(
        const TagSet & tag_set,
        const Lexicon & lexicon,
        const Grammar & grammar,
        int fine_level,
        int coarse_level);

    ~ModelProjector();

    std::shared_ptr<Lexicon> generateLexicon() const;
    std::shared_ptr<Grammar> generateGrammar() const;

private:
    const TagSet & tag_set_;
    const Lexicon & lexicon_;
    const Grammar & grammar_;
    int fine_level_;
    int coarse_level_;
    Mapping mapping_;
    std::vector<double> cond_prob_;

}; // class ModelProjector

} // namespace Ckylark

#endif // CKYLARK_MODEL_PROJECTOR_H_

