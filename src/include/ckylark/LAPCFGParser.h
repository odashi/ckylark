#ifndef CKYLARK_LAPCFG_PARSER_H_
#define CKYLARK_LAPCFG_PARSER_H_

#include <ckylark/Parser.h>

#include <ckylark/CKYTable.h>
#include <ckylark/Dictionary.h>
#include <ckylark/TagSet.h>
#include <ckylark/Lexicon.h>
#include <ckylark/Grammar.h>
#include <ckylark/Tree.h>
#include <ckylark/SignatureEstimator.h>

#include <memory>

namespace Ckylark {

class LAPCFGParser : public Parser {

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
    virtual ~LAPCFGParser();

    static std::shared_ptr<LAPCFGParser> loadFromBerkeleyDump(const std::string & path);

    virtual ParserResult parse(
        const std::vector<std::string> & sentence,
        const ParserSetting & setting) const;

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
    std::shared_ptr<SignatureEstimator> sig_est_;

    int fine_level_;
    double prune_threshold_;
    double smooth_unklex_;

    double scaling_factor_;

    ParserResult generateMaxRuleOneBestParse(
        const std::vector<std::string> & sentence,
        const ParserSetting & setting,
        int final_level_to_try) const;

    void loadWordTable(const std::string & path);
    void loadTagSet(const std::string & path);
    void loadLexicon(const std::string & path);
    void loadGrammar(const std::string & path);
    void generateCoarseModels();
    
    std::shared_ptr<Tree<std::string> > getDefaultParse() const;

    std::vector<int> makeWordIdList(const std::vector<std::string> & sentence) const;

    std::vector<int> makeTagIdList(const std::vector<std::string> & sentence) const;

    void initializeCharts(
        CKYTable<bool> & allowed_tag,
        CKYTable<std::vector<bool> > & allowed_sub,
        CKYTable<std::vector<double> > & inside,
        CKYTable<std::vector<double> > & outside,
        std::vector<std::vector<Extent> > & extent,
        int cur_level) const;

    void setTerminalScores(
        const CKYTable<bool> & allowed_tag,
        const CKYTable<std::vector<bool> > & allowed_sub,
        CKYTable<std::vector<double> > & inside,
        const std::vector<int> & wid_list,
        const std::vector<int> & tid_list,
        int cur_level,
        bool partial) const;

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

