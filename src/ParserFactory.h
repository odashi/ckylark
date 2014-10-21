#ifndef CKYLARK_PARSER_FACTORY_H_
#define CKYLARK_PARSER_FACTORY_H_

#include "Parser.h"
#include "ArgumentParser.h"

#include <memory>

namespace Ckylark {

class ParserFactory {

    ParserFactory() = delete;
    ParserFactory(const ParserFactory &) = delete;
    ParserFactory & operator=(const ParserFactory &) = delete;

public:
    static std::shared_ptr<Parser> create(const ArgumentParser & args);

private:
    static std::shared_ptr<Parser> createLAPCFGParser(const ArgumentParser & args);

}; // class ParserFactory

} // namespace Ckylark

#endif // CKYLARK_PARSER_FACTORY_H_

