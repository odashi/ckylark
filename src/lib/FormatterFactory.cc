#include <ckylark/FormatterFactory.h>

#include <ckylark/SExprFormatter.h>
#include <ckylark/POSTagFormatter.h>

#include <stdexcept>

using namespace std;
using namespace boost;

namespace Ckylark {

std::shared_ptr<Formatter> FormatterFactory::create(const map<string, any> & args) {
    
    string type = any_cast<string>(args.at("output-format"));

    if (type == "sexpr") {
        bool add_root_tag = any_cast<bool>(args.at("add-root-tag"));
        return std::shared_ptr<Formatter>(new SExprFormatter(add_root_tag));
    } else if (type == "postag") {
        string separator = any_cast<string>(args.at("separator"));
        return std::shared_ptr<Formatter>(new POSTagFormatter(separator));
    } else {
        throw runtime_error("FormatterFactory::create(): unknown output format: " + type);
    }
}

} // namespace Ckylark

