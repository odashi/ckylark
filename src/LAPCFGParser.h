#ifndef CKYLARK_LAPCFG_PARSER_H_
#define CKYLARK_LAPCFG_PARSER_H_

#include "CKYTable.h"
#include "Dictionary.h"
#include "TagSet.h"
#include "Lexicon.h"
#include "Grammar.h"
#include "Tree.h"
#include "ParserResult.h"

#include <boost/multi_array.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Ckylark {

class LAPCFGParser {

    struct Extent {
        int narrow_right;
        int narrow_left;
        int wide_right;
        int wide_left;
    }; // struct Extent

    LAPCFGParser();
    LAPCFGParser(const LAPCFGParser &) = delete;
    LAPCFGParser & operator=(const LAPCFGParser &) = delete;

public:
    ~LAPCFGParser();

    static std::shared_ptr<LAPCFGParser> loadFromBerkeleyDump(const std::string & path);

    ParserResult parse(const std::vector<std::string> & sentence) const;

    const Dictionary & getWordTable() const { return *word_table_; }
    const TagSet & getTagSet() const { return *tag_set_; }
    const Lexicon & getLexicon(int level) const { return *(lexicon_[level]); }
    const Grammar & getGrammar(int level) const { return *(grammar_[level]); }

    int getFineLevel() const { return fine_level_; }
    void setFineLevel(int value);

    double getPruningThreshold() const { return prune_threshold_; }
    void setPruningThreshold(double value);

    double getUNKLexiconSmoothing() const { return smooth_unklex_; }
    void setUNKLexiconSmoothing(double value);

private:
    std::shared_ptr<Dictionary> word_table_;
    std::shared_ptr<TagSet> tag_set_;
    std::vector<std::shared_ptr<Lexicon> > lexicon_;
    std::vector<std::shared_ptr<Grammar> > grammar_;

    int fine_level_;
    double prune_threshold_;
    double smooth_unklex_;

    double scaling_factor_;

    void loadWordTable(const std::string & path);
    void loadTagSet(const std::string & path);
    void loadLexicon(const std::string & path);
    void loadGrammar(const std::string & path);
    void generateCoarseModels();
    
    std::shared_ptr<Tree<std::string> > getDefaultParse() const;

    std::vector<int> makeWordIDList(const std::vector<std::string> & sentence) const;

    void initializeCharts(
        CKYTable<bool> & allowed_tag,
        CKYTable<std::vector<bool> > & allowed_sub,
        CKYTable<std::vector<double> > & inside,
        CKYTable<std::vector<double> > & outside,
        std::vector<std::vector<Extent> > & extent,
        int cur_level) const;

    void setInsideScoresByLexicon(
        const CKYTable<bool> & allowed_tag,
        const CKYTable<std::vector<bool> > & allowed_sub,
        CKYTable<std::vector<double> > & inside,
        const std::vector<int> & wid_list,
        int cur_level) const;

    void calculateInsideScores(
        const CKYTable<bool> & allowed_tag,
        const CKYTable<std::vector<bool> > & allowed_sub,
        CKYTable<std::vector<double> > & inside,
        std::vector<std::vector<Extent> > & extent,
        int cur_level) const;

    void calculateOutsideScores(
        const CKYTable<bool> & allowed_tag,
        const CKYTable<std::vector<bool> > & allowed_sub,
        const CKYTable<std::vector<double> > & inside,
        CKYTable<std::vector<double> > & outside,
        std::vector<std::vector<Extent> > & extent,
        int cur_level) const;

    void pruneCharts(
        CKYTable<bool> & allowed_tag,
        CKYTable<std::vector<bool> > & allowed_sub,
        const CKYTable<std::vector<double> > & inside,
        const CKYTable<std::vector<double> > & outside,
        int cur_level) const;

}; // struct Model

} // namespace Ckylark

#endif // CKYLARK_LAPCFG_PARSER_H_

