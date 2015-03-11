#include <ckylark/FormatterFactory.h>
#include <ckylark/Mapping.h>
#include <ckylark/Timer.h>
#include <ckylark/Tracer.h>
#include <ckylark/ParserFactory.h>
#include <ckylark/ParserResult.h>
#include <ckylark/ParserSetting.h>
#include <ckylark/StreamFactory.h>

#include <boost/algorithm/string.hpp>
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <cstdio>
#include <cmath>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace boost;
using namespace Ckylark;

namespace PO = boost::program_options;

PO::variables_map parseOptions(int argc, char * argv[]) {
    string description = "Ckylark - PCFG-LA based phrase structure parser.";
    string binname = "ckylark";

    // generic options
    PO::options_description opt_generic("Generic Options");
    opt_generic.add_options()
        ("help", "print this manual and exit")
        ("trace-level", PO::value<int>()->default_value(0), "detail level of tracing text");
        ;
    // input/output
    PO::options_description opt_io("I/O Options");
    opt_io.add_options()
        ("model", PO::value<string>(), "(required) prefix of model path")
        ("input", PO::value<string>()->default_value("/dev/stdin"), "input file")
        ("output", PO::value<string>()->default_value("/dev/stdout"), "output file")
        ;
    // parsing methods
    PO::options_description opt_parsing("Parsing Options");
    opt_parsing.add_options()
        ("method", PO::value<string>()->default_value("lapcfg"), "parsing strategy\n(candidates: 'lapcfg')")
        ("fine-level", PO::value<int>()->default_value(-1), "most fine level to parse, or -1 (use all levels)")
        ("prune-threshold", PO::value<double>()->default_value(1e-5), "coarse-to-fine pruning threshold")
        ("smooth-unklex", PO::value<double>()->default_value(1e-10), "smoothing strength using UNK lexicon")
        ("partial", "parse partial (grammar tag contained) sentence")
        ("do-m1-preparse", "do preparsing using G-1 grammar/lexicon")
        ("force-generate", "generate list-of-words tree if parsing fails")
        ;
    // formatting
    PO::options_description opt_formatting("Formatting Options");
    opt_formatting.add_options()
        ("output-format", PO::value<string>()->default_value("sexpr"), "output format\n(candidates: 'sexpr', 'postag')")
        ("add-root-tag", "add ROOT tag into output tree (for 'sexpr' format)")
        ("binarize", "generates parse tree by only unary/binary rules (for 'sexpr' format)")
        ("separator", PO::value<string>()->default_value("/"), "word-POS separator (for 'postag' format)")
        ;

    PO::options_description opt;
    opt.add(opt_generic).add(opt_io).add(opt_parsing).add(opt_formatting);

    // parse
    PO::variables_map args;
    PO::store(PO::parse_command_line(argc, argv, opt), args);
    PO::notify(args);

    // process usage
    if (args.count("help")) {
        cerr << description << endl;
        cerr << "Usage: " << binname << " [options] --model MODEL_PREFIX < INPUT_CORPUS" << endl;
        cerr << opt << endl;
        exit(1);
    }

    // check required options
    if (!args.count("model")) {
        cerr << "ERROR: insufficient required options" << endl;
        cerr << "(--help to show usage)" << endl;
        exit(1);
    }

    return move(args);
}

int main(int argc, char * argv[]) {

    auto args = parseOptions(argc, argv);

    Tracer::setTraceLevel(args["trace-level"].as<int>());
    
    // open input/output streams
    std::shared_ptr<InputStream> ifs = StreamFactory::createInputStream(args["input"].as<string>());
    std::shared_ptr<OutputStream> ofs = StreamFactory::createOutputStream(args["output"].as<string>());

    // create formatter
    map<string, any> formatter_args;
    formatter_args["output-format"] = args["output-format"].as<string>();
    formatter_args["add-root-tag"] = !!args.count("add-root-tag");
    formatter_args["separator"] = args["separator"].as<string>();
    std::shared_ptr<Formatter> formatter = FormatterFactory::create(formatter_args);

    // set parser settings
    ParserSetting setting;
    setting.partial = !!args.count("partial");
    setting.binarize = !!args.count("binarize");

    // create parser
    map<string, any> parser_args;
    parser_args["method"] = args["method"].as<string>();
    parser_args["model"] = args["model"].as<string>();
    parser_args["fine-level"] = args["fine-level"].as<int>();
    parser_args["prune-threshold"] = args["prune-threshold"].as<double>();
    parser_args["smooth-unklex"] = args["smooth-unklex"].as<double>();
    parser_args["do-m1-preparse"] = !!args.count("do-m1-preparse");
    parser_args["force-generate"] = !!args.count("force-generate");
    std::shared_ptr<Parser> parser = ParserFactory::create(parser_args);

    // make parser setting
    Timer timer;

    Tracer::println(1, "Ready");

    string line;
    int total_lines = 0;
    int total_words = 0;
    
    while (ifs->readLine(line)) {
        trim(line);
        vector<string> ls;
        if (!line.empty()) {
            split(ls, line, is_space(), boost::algorithm::token_compress_on);
        }
        
        ++total_lines;
        total_words += ls.size();
        
        Tracer::print(1, (format("Input %d:") % total_lines).str());
        for (const string & s : ls) {
            Tracer::print(1, " " + s);
        }
        Tracer::println(1);

        timer.start();
        ParserResult result = parser->parse(ls, setting);
        double lap = timer.stop();

        string repr = formatter->generate(*result.best_parse);
        
        Tracer::println(1, "  Parse: " + repr);
        Tracer::println(1, (format("  Time: %.3fs") % lap).str());

        ofs->writeLine(repr);
    }

    Tracer::println(1);
    Tracer::println(1, (format("Parsed %d sentences, %d words.") % total_lines % total_words).str());
    Tracer::println(1, (format("Total parsing time: %.3fs.") % timer.elapsed()).str());

    return 0;
}

