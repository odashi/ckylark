#ifndef CKYLARK_STRING_UTIL_H_
#define CKYLARK_STRING_UTIL_H_

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

}; // class StringUtil

} // namespace Ckylark

#endif // CKYLARK_STRING_UTIL_H_

