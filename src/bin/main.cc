#include <ckylark/ArgumentParser.h>
#include <ckylark/Formatter.h>
#include <ckylark/ParserFactory.h>
#include <ckylark/Mapping.h>
#include <ckylark/Timer.h>
#include <ckylark/Tracer.h>
#include <ckylark/ParserResult.h>
#include <ckylark/StreamFactory.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <cstdio>
#include <cmath>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace Ckylark;

unique_ptr<ArgumentParser> parseArgs(int argc, char * argv[]) {
    unique_ptr<ArgumentParser> ap(new ArgumentParser("ckylark -model <model-prefix> [options]"));

    ap->addStringArgument("method", "lapcfg", "parsing strategy", false);

    ap->addSwitchArgument("help", "print this manual and exit");
    ap->addIntegerArgument("trace-level", 0, "detail level of trace text", false);
    
    ap->addStringArgument("model", "", "prefix of model path", true);
    
    ap->addStringArgument("input", "/dev/stdin", "input file", false);
    ap->addStringArgument("output", "/dev/stdout", "output file", false);

    ap->addIntegerArgument("fine-level", -1, "most fine level to parse, or -1 (use all level)", false);
    ap->addRealArgument("prune-threshold", 1e-5, "coarse-to-fine pruning threshold", false);
    ap->addRealArgument("smooth-unklex", 1e-10, "smoothing strength using UNK lexicon", false);

    ap->addSwitchArgument("add-root-tag", "add ROOT tag into output tree");

    bool ret = ap->parseArgs(argc, argv);

    if (ap->getSwitch("help")) {
        ap->printUsage();
        exit(0);
    }

    if (!ret) {
        exit(0);
    }

    return ap;
}

int main(int argc, char * argv[]) {
    unique_ptr<ArgumentParser> ap = ::parseArgs(argc, argv);

    Tracer::setTraceLevel(ap->getInteger("trace-level"));
    
    // open input/output streams
    shared_ptr<InputStream> ifs = StreamFactory::createInputStream(ap->getString("input"));
    shared_ptr<OutputStream> ofs = StreamFactory::createOutputStream(ap->getString("output"));

    // select parser
    shared_ptr<Parser> parser = ParserFactory::create(*ap);

    Timer timer;

    Tracer::println(1, "Ready");

    string line;
    int total_lines = 0;
    int total_words = 0;
    
    while (ifs->readLine(line)) {
        boost::trim(line);
        vector<string> ls;
        if (!line.empty()) {
            boost::split(ls, line, boost::is_space(), boost::algorithm::token_compress_on);
        }
        
        ++total_lines;
        total_words += ls.size();
        
        Tracer::print(1, (boost::format("Input %d:") % total_lines).str());
        for (const string & s : ls) {
            Tracer::print(1, " " + s);
        }
        Tracer::println(1);

        timer.start();
        ParserResult result = parser->parse(ls);
        double lap = timer.stop();

        string repr = Formatter::ToPennTreeBank(*result.best_parse, ap->getSwitch("add-root-tag"));
        
        Tracer::println(1, "  Parse: " + repr);
        Tracer::println(1, (boost::format("  Time: %.3fs") % lap).str());

        ofs->writeLine(repr);
    }

    Tracer::println(1);
    Tracer::println(1, (boost::format("Parsed %d sentences, %d words.") % total_lines % total_words).str());
    Tracer::println(1, (boost::format("Total parsing time: %.3fs.") % timer.elapsed()).str());

    return 0;
}

