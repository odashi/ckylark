#ifndef CKYLARK_CHAR_UTIL_H_
#define CKYLARK_CHAR_UTIL_H_

namespace Ckylark {

// utility functions for character processing
class CharUtil {

    CharUtil() = delete;
    CharUtil(const CharUtil &) = delete;
    CharUtil & operator=(const CharUtil &) = delete;

public:

    // true if c is one of whitespace or control code of ASCII, false otherwise
    inline static bool isControl(char c) { return ((c >= 0x00 && c <= 0x20) || c == 0x7f); }

    // true if c is real symbol character, false otherwise
    inline static bool isLetter(char c) { return !isControl(c); }

    // true if c is one of whitespace, false otherwise
    inline static bool isSpace(char c) { return ((c >= 0x09 && c <= 0x0d) || c == 0x20); }

    // true if c is one of uppercase alphabet, false otherwise
    inline static bool isUpper(char c) { return (c >= 'A' && c <= 'Z'); }
    
    // true if c is one of lowercase alphabet, false otherwise
    inline static bool isLower(char c) { return (c >= 'a' && c <= 'z'); }

    // make uppercase character
    inline static char toUpper(char c) { return isLower(c) ? c + 'A' - 'a' : c; }
    
    // make lowercase character
    inline static char toLower(char c) { return isUpper(c) ? c + 'a' - 'A' : c; }

}; // class CharUtil

} // namespace Ckylark

#endif // CKYLARK_CHAR_UTIL_H_

