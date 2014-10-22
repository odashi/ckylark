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
    const string & word = sentence[location];
    string signature = "UNK";
    
    size_t len = word.size();
    if (len == 0) return signature;

    size_t num_digit = StringUtil::numDigit(word);
    size_t num_caps = StringUtil::numUpper(word);
    size_t num_lower = StringUtil::numLower(word);
    size_t num_dash = StringUtil::numChar(word, '-');
    char c0 = word[0];
    string lowered = StringUtil::toLower(word);

    if (CharUtil::isUpper(c0)) {
        if (location == 0 && num_caps == 1) {
            signature += "-INITC";
            if (known_words_.getId(lowered) != -1) {
                signature += "-KNOWNLC";
            }
        } else {
            signature += "-CAPS";
        }
    } else if (!CharUtil::isLetter(c0) && num_caps > 0) {
        signature += "-CAPS";
    } else if (num_lower > 0) {
        signature += "-LC";
    }

    if (num_digit > 0) {
        signature += "-NUM";
    }

    if (num_dash > 0) {
        signature += "-DASH";
    }

    if (StringUtil::endsWith(lowered, "s") && len >= 3) {
        char c2 = lowered[len - 2];
        if (c2 != 's' && c2 != 'i' && c2 != 'u') {
            signature += "-s";
        }
    } else if (len >= 5 && num_dash == 0 && !(num_digit > 0 && num_caps > 0)) {
        if (StringUtil::endsWith(lowered, "ed")) signature += "-ed";
        else if (StringUtil::endsWith(lowered, "ing")) signature += "-ing";
        else if (StringUtil::endsWith(lowered, "ion")) signature += "-ion";
        else if (StringUtil::endsWith(lowered, "er")) signature += "-er";
        else if (StringUtil::endsWith(lowered, "est")) signature += "-est";
        else if (StringUtil::endsWith(lowered, "ly")) signature += "-ly";
        else if (StringUtil::endsWith(lowered, "ity")) signature += "-ity";
        else if (StringUtil::endsWith(lowered, "y")) signature += "-y";
        else if (StringUtil::endsWith(lowered, "al")) signature += "-al";
    }

    return signature;
}

} // namespace Ckylark

