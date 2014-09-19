#ifndef AHCP_TRACER_H_
#define AHCP_TRACER_H_

#include <string>

namespace AHCParser {

class Tracer {

    Tracer() = delete;
    Tracer(const Tracer &) = delete;

public:
    static void print(const std::string & text, unsigned int level);
    static void println(const std::string & text, unsigned int level);
    static unsigned int getTraceLevel();
    static void setTraceLevel(unsigned int value);

private:
    static unsigned int trace_level_;

}; // class Tracer

} // namespace AHCParser

#endif // AHCP_TRACER_H_

