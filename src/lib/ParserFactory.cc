#include <ckylark/ParserFactory.h>

#include <ckylark/LAPCFGParser.h>
#include <ckylark/Tracer.h>

#include <boost/format.hpp>

#include <stdexcept>

using namespace std;
using namespace boost;

namespace Ckylark {

std::shared_ptr<Parser> ParserFactory::create(const map<string, any> & args) {

    string method = any_cast<string>(args.at("method"));

    if (method == "lapcfg") {
        Tracer::println(1, "Parsing method: LAPCFG");
        std::shared_ptr<LAPCFGParser> parser = LAPCFGParser::loadFromBerkeleyDump(any_cast<string>(args.at("model")));
        int fine_level = any_cast<int>(args.at("fine-level"));
        parser->setFineLevel(fine_level);
        parser->setPruningThreshold(any_cast<double>(args.at("prune-threshold")));
        parser->setUNKLexiconSmoothing(any_cast<double>(args.at("smooth-unklex")));
        parser->setDoM1Preparse(any_cast<bool>(args.at("do-m1-preparse")));
        parser->setForceGenerate(any_cast<bool>(args.at("force-generate")));
        Tracer::println(1, (format("fine-level: %d (requested: %d)") % parser->getFineLevel() % fine_level).str());
        Tracer::println(1, (format("prune-threshold: %.3e") % parser->getPruningThreshold()).str());
        Tracer::println(1, (format("smooth-unklex: %.3e") % parser->getUNKLexiconSmoothing()).str());
        Tracer::println(1, string("do-m1-preparse: ") + (parser->getDoM1Preparse() ? "yes" : "no"));
        return std::shared_ptr<Parser>(parser);
    } else {
        // factory does not know such parser
        throw runtime_error("ParserFactory::create(): unknown parser method: " + method);
    }
}

} // namespace Ckylark

