#include <Tracer.h>

#include <iostream>

using namespace std;

namespace Ckylark {

unsigned int Tracer::trace_level_ = 0;

void Tracer::print(unsigned int level, const string & text) {
    if (level > trace_level_) return;
    cerr << text;
}

void Tracer::println(unsigned int level, const string & text) {
    if (level > trace_level_) return;
    cerr << text << endl;
}

void Tracer::println(unsigned int level) {
    if (level > trace_level_) return;
    cerr << endl;
}

unsigned int Tracer::getTraceLevel() {
    return trace_level_;
}

void Tracer::setTraceLevel(unsigned int value) {
    trace_level_ = value;
}

} // namespace Ckylark

