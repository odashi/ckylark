#include "BerkeleySignatureEstimator.h"

#include "StringUtil.h"

#include <stdexcept>

using namespace std;

/*
    NOTE:
    this processes does not support the Unicode codepoints.
*/

namespace Ckylark {

BerkeleySignatureEstimator::BerkeleySignatureEstimator(
    BerkeleySignatureEstimator::Language lang,
    const Dictionary & known_words)
    : lang_(lang)
    , known_words_(known_words) {}

string BerkeleySignatureEstimator::getSignature(const vector<string> & sentence, size_t location) {
    if (location >= sentence.size())
        throw runtime_error("BerkeleySignatureEstimator::getSignature: invalid location");

    switch (lang_) {
        case Other: return getOtherSignature(sentence, location);
        case English: return getEnglishSignature(sentence, location);
        default:
            throw runtime_error("BerkeleySignatureEstimator::getSignature: unknown language type");
    }
}

string BerkeleySignatureEstimator::getOtherSignature(const vector<string> & sentence, size_t location) {
    return "UNK";
}

string BerkeleySignatureEstimator::getEnglishSignature(const vector<string> & sentence, size_t location) {
    string signature = "UNK";
    /*
    int len = word.size();
    if (len == 0) return signature;

    int num_digit = StringUtil.numDigit(word);
    int num_lower = StringUtil.numLower(word);
    int num_dash = StringUtil.numChar(word, '-');
    int num_caps = len - num_digit - num_lower - num_dash;

    char c0 = word[0];
    */
    return signature;
}

} // namespace Ckylark

