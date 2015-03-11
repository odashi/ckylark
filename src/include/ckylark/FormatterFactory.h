#ifndef CKYLARK_FORMATTER_FACTORY_H_
#define CKYLARK_FORMATTER_FACTORY_H_

#include <ckylark/Formatter.h>

#include <boost/any.hpp>

#include <map>
#include <memory>

namespace Ckylark {

class FormatterFactory {

    FormatterFactory() = delete;
    FormatterFactory(const FormatterFactory &) = delete;
    FormatterFactory & operator=(const FormatterFactory &) = delete;

public:
    static std::shared_ptr<Formatter> create(const std::map<std::string, boost::any> & args);

}; // class FormatterFactory

} // namespace Ckylark

#endif // CKYLARK_FORMATTER_FACTORY_H_

