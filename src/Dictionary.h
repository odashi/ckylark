#ifndef AHCP_DICTIONARY_H_
#define AHCP_DICTIONARY_H_

#include <map>
#include <string>
#include <vector>

namespace AHCParser {

class Dictionary {

public:
    Dictionary();
    ~Dictionary();

    void addWord(const std::string & word);

    int getId(const std::string & word) const;
    std::string getWord(int id) const;

    size_t size() const { return ids_.size(); }

private:
    std::map<std::string, int> ids_;
    std::vector<std::string> rev_;

}; // class Dictionary

} // namespace AHCParser

#endif // AHCP_DICTIONARY_H_

