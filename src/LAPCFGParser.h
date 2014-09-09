#ifndef AHCP_LAPCFG_PARSER_H_
#define AHCP_LAPCFG_PARSER_H_

#include "Dictionary.h"
#include "TagSet.h"
#include "Lexicon.h"
#include "Grammar.h"
#include "Tree.h"

#include <boost/multi_array.hpp>

#include <memory>
#include <string>
#include <vector>

namespace AHCParser {

class LAPCFGParser {

    LAPCFGParser();
    LAPCFGParser(const LAPCFGParser &) = delete;

public:
    ~LAPCFGParser();

    static std::shared_ptr<LAPCFGParser> loadFromBerkeleyDump(const std::string & path);

    std::shared_ptr<Tree<std::string> > parse(const std::vector<std::string> & text) const;

    Dictionary & getWordTable() { return *word_table_; }
    TagSet & getTagSet() { return *tag_set_; }
    Lexicon & getLexicon(int level) { return *(lexicon_[level]); }
    Grammar & getGrammar(int level) { return *(grammar_[level]); }

private:
    std::shared_ptr<Dictionary> word_table_;
    std::shared_ptr<TagSet> tag_set_;
    std::vector<std::shared_ptr<Lexicon> > lexicon_;
    std::vector<std::shared_ptr<Grammar> > grammar_;

    void loadWordTable(const std::string & path);
    void loadTagSet(const std::string & path);
    void loadLexicon(const std::string & path);
    void loadGrammar(const std::string & path);
    void generateCoarseModels();

    std::shared_ptr<Tree<std::string> > getDefaultParse() const;

}; // struct Model

} // namespace AHCParser

#endif // AHCP_LAPCFG_PARSER_H_

