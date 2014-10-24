#ifndef CKYLARK_LEXICON_H_
#define CKYLARK_LEXICON_H_

#include "TagSet.h"
#include "Dictionary.h"
#include "Stream.h"

#include <iostream>
#include <map>
#include <memory>
#include <vector>

namespace Ckylark {

class LexiconEntry {

    LexiconEntry() = delete;
    LexiconEntry(const LexiconEntry &) = delete;
    LexiconEntry & operator=(const LexiconEntry &) = delete;

public:
    LexiconEntry(int tag_id, int word_id, size_t num_subtags)
        : tag_id_(tag_id)
        , word_id_(word_id)
        , score_(num_subtags, 0.0) {
    }

    ~LexiconEntry() {}

    inline int tagId() const { return tag_id_; }
    inline int wordId() const { return word_id_; }
    inline size_t numSubtags() const { return score_.size(); }

    inline double getScore(int subtag) const { return score_[subtag]; }
    inline void setScore(int subtag, double value) { score_[subtag] = value; }
    inline void addScore(int subtag, double delta) { score_[subtag] += delta; }

    double getMaxScore() const;

private:
    int tag_id_;
    int word_id_;
    std::vector<double> score_;

}; // class LexiconEntry

class Lexicon {

    Lexicon() = delete;
    Lexicon(const Lexicon &) = delete;
    Lexicon & operator=(const Lexicon &) = delete;

public:
    Lexicon(const TagSet & tag_set, int level);
    ~Lexicon();

    static std::shared_ptr<Lexicon> loadFromStream(
        InputStream & stream,
        const Dictionary & word_table,
        const TagSet & tag_set);

    const LexiconEntry * getEntry(int tag_id, int word_id) const;
    LexiconEntry & getEntryOrCreate(int tag_id, int word_id);

    const std::vector<std::map<int, LexiconEntry *> > getEntryList() const { return entry_; }

    double getScalingFactor(int word_id) const;

    inline int getLevel() const { return level_; }

    inline bool hasEntry(int tag_id) const { return !entry_[tag_id].empty(); };

private:
    const TagSet & tag_set_;
    int level_;
    std::vector<std::map<int, LexiconEntry *> > entry_; // [category]{word}
    mutable std::map<int, double> scaling_;

}; // class Lexicon

} // namespace Ckylark

#endif // CKYLARK_LEXICON_H_

