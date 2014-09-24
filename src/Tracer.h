#ifndef CKYLARK_TRACER_H_
#define CKYLARK_TRACER_H_

#include <string>

namespace Ckylark {

class Tracer {

    Tracer() = delete;
    Tracer(const Tracer &) = delete;

public:
    static void print(unsigned int level, const std::string & text);
    static void println(unsigned int level, const std::string & text);
    static void println(unsigned int level);
    static unsigned int getTraceLevel();
    static void setTraceLevel(unsigned int value);

private:
    static unsigned int trace_level_;

}; // class Tracer

} // namespace Ckylark

#endif // CKYLARK_TRACER_H_

