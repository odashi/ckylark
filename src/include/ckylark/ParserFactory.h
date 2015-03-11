#ifndef CKYLARK_PARSER_FACTORY_H_
#define CKYLARK_PARSER_FACTORY_H_

#include <ckylark/Parser.h>

#include <boost/any.hpp>

#include <map>
#include <memory>

namespace Ckylark {

class ParserFactory {

    ParserFactory() = delete;
    ParserFactory(const ParserFactory &) = delete;
    ParserFactory & operator=(const ParserFactory &) = delete;

public:
    static std::shared_ptr<Parser> create(const std::map<std::string, boost::any> & args);

}; // class ParserFactory

} // namespace Ckylark

#endif // CKYLARK_PARSER_FACTORY_H_

