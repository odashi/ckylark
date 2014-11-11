#include <ckylark/FormatterFactory.h>

#include <ckylark/SExprFormatter.h>
#include <ckylark/POSTagFormatter.h>

#include <stdexcept>

using namespace std;

namespace Ckylark {

shared_ptr<Formatter> FormatterFactory::create(const ArgumentParser & args) {
    string type = args.getString("output-format");
    if (type == "sexpr") {
        bool add_root_tag = args.getSwitch("add-root-tag");
        return shared_ptr<Formatter>(new SExprFormatter(add_root_tag));
    } else if (type == "postag") {
        string separator = args.getString("separator");
        return shared_ptr<Formatter>(new POSTagFormatter(separator));
    } else {
        throw runtime_error("FormatterFactory::create(): unknown output format: " + type);
    }
}

} // namespace Ckylark

