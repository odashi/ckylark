#ifndef CKYLARK_FORMATTER_FACTORY_H_
#define CKYLARK_FORMATTER_FACTORY_H_

#include <ckylark/Formatter.h>
#include <ckylark/ArgumentParser.h>

#include <memory>

namespace Ckylark {

class FormatterFactory {

    FormatterFactory() = delete;
    FormatterFactory(const FormatterFactory &) = delete;
    FormatterFactory & operator=(const FormatterFactory &) = delete;

public:
    static std::shared_ptr<Formatter> create(const ArgumentParser & args);

}; // class FormatterFactory

} // namespace Ckylark

#endif // CKYLARK_FORMATTER_FACTORY_H_

