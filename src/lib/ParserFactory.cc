#include <ckylark/ParserFactory.h>

#include <ckylark/LAPCFGParser.h>
#include <ckylark/Tracer.h>

#include <boost/format.hpp>

using namespace std;

namespace Ckylark {

shared_ptr<Parser> ParserFactory::create(const ArgumentParser & args) {
    string method = args.getString("method");

    if (method == "lapcfg") return createLAPCFGParser(args);

    // factory do not know such parser
    return shared_ptr<Parser>(nullptr);
}

shared_ptr<Parser> ParserFactory::createLAPCFGParser(const ArgumentParser & args) {

    Tracer::println(1, "Parsing method: LAPCFG");

    shared_ptr<LAPCFGParser> parser = LAPCFGParser::loadFromBerkeleyDump(args.getString("model"));

    parser->setFineLevel(args.getInteger("fine-level"));
    parser->setPruningThreshold(args.getReal("prune-threshold"));
    parser->setUNKLexiconSmoothing(args.getReal("smooth-unklex"));
    parser->setDoM1Preparse(args.getSwitch("do-m1-preparse"));

    Tracer::println(1, (boost::format("fine-level: %d (requested: %d)") % parser->getFineLevel() % args.getInteger("fine-level")).str());
    Tracer::println(1, (boost::format("prune-threshold: %.3e") % parser->getPruningThreshold()).str());
    Tracer::println(1, (boost::format("smooth-unklex: %.3e") % parser->getUNKLexiconSmoothing()).str());
    Tracer::println(1, string("do-m1-preparse: ") + (parser->getDoM1Preparse() ? "yes" : "no"));

    return shared_ptr<Parser>(parser);
}

} // namespace Ckylark

