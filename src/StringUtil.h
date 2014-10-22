#ifndef CKYLARK_STRING_UTIL_H_
#define CKYLARK_STRING_UTIL_H_

#include "CharUtil.h"

#include <algorithm>
#include <string>

namespace Ckylark {

// utility functions for string processing
class StringUtil {

    StringUtil() = delete;
    StringUtil(const StringUtil &) = delete;
    StringUtil & operator=(const StringUtil &) = delete;

public:

    // true if str contains c, false otherwise
    inline static bool hasChar(const std::string & str, char c) {
        for (char x : str) {
            if (x == c) return true;
        }
        return false;
    }

    // true if str contains any character in the range [low, high], false otherwise
    inline static bool hasCharRange(const std::string & str, char low, char high) {
        for (char x : str) {
            if (x >= low && x <= high) return true;
        }
        return false;
    }

    // true if str contains digit, false otherwise
    inline static bool hasDigit(const std::string & str) { return hasCharRange(str, '0', '9'); }

    // true if str contains uppercase alphabet, false otherwise
    inline static bool hasUpper(const std::string & str) { return hasCharRange(str, 'A', 'Z'); }

    // true if str contains lowercase alphabet, false otherwise
    inline static bool hasLower(const std::string & str) { return hasCharRange(str, 'a', 'z'); }

    // count the number of character c in str
    inline static size_t numChar(const std::string & str, char c) {
        size_t n = 0;
        for (char x : str) {
            if (x == c) ++n;
        }
        return n;
    }

    // count the number of characters in str which is in the range [low, high]
    inline static size_t numCharRange(const std::string & str, char low, char high) {
        size_t n = 0;
        for (char x : str) {
            if (x >= low && x <= high) ++n;
        }
        return n;
    }

    // count the number of digits in str
    inline static size_t numDigit(const std::string & str) { return numCharRange(str, '0', '9'); }

    // count the number of uppercase alphabet in str
    inline static size_t numUpper(const std::string & str) { return numCharRange(str, 'A', 'Z'); }

    // count the number of lowercase alphabet in str
    inline static size_t numLower(const std::string & str) { return numCharRange(str, 'a', 'z'); }

    // make uppercase string
    inline static std::string toUpper(const std::string & str) {
        std::string ret = str;
        std::transform(ret.begin(), ret.end(), ret.begin(), CharUtil::toUpper);
        return ret;
    }

    // make lowercase string
    inline static std::string toLower(const std::string & str) {
        std::string ret = str;
        std::transform(ret.begin(), ret.end(), ret.begin(), CharUtil::toLower);
        return ret;
    }

}; // class StringUtil

} // namespace Ckylark

#endif // CKYLARK_STRING_UTIL_H_

