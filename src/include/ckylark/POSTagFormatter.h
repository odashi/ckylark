#ifndef CKYLARK_POS_TAG_FORMATTER_H_
#define CKYLARK_POS_TAG_FORMATTER_H_

#include <ckylark/Formatter.h>

namespace Ckylark {

// generates only words and POS-tags
class POSTagFormatter : public Formatter {

    POSTagFormatter(const POSTagFormatter &) = delete;
    POSTagFormatter & operator=(const POSTagFormatter &) = delete;

public:
    POSTagFormatter(const std::string & separator);
    ~POSTagFormatter();

    std::string generate(const Tree<std::string> & parse) const;

private:
    std::string separator_;

}; // class POSTagFormatter

} // namespace Ckylark

#endif // CKYLARK_POS_TAG_FORMATTER_H_

