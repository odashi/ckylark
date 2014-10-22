#ifndef CKYLARK_CHAR_UTIL_H_
#define CKYLARK_CHAR_UTIL_H_

namespace Ckylark {

// utility functions for character processing
class CharUtil {

    CharUtil() = delete;
    CharUtil(const CharUtil &) = delete;
    CharUtil & operator=(const CharUtil &) = delete;

public:

    // make uppercase character
    inline static char toUpper(char c) { return (c >= 'a' && c <= 'z') ? c + 'A' - 'a' : c; }
    
    // make lowercase character
    inline static char toLower(char c) { return (c >= 'A' && c <= 'Z') ? c + 'a' - 'A' : c; }

}; // class CharUtil

} // namespace Ckylark

#endif // CKYLARK_CHAR_UTIL_H_

