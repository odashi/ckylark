#include <Tracer.h>

#include <iostream>

using namespace std;

namespace AHCParser {

unsigned int Tracer::trace_level_ = 0;

void Tracer::print(const string & text, unsigned int level) {
    if (level > trace_level_) return;
    cerr << text;
}

void Tracer::println(const string & text, unsigned int level) {
    if (level > trace_level_) return;
    cerr << text << endl;
}

unsigned int Tracer::getTraceLevel() {
    return trace_level_;
}

void Tracer::setTraceLevel(unsigned int value) {
    trace_level_ = value;
}

} // namespace AHCParser

