#include "Timer.h"

#include <stdexcept>

using namespace std;

namespace AHCParser {

Timer::Timer()
    : elapsed_(0)
    , start_time_(0)
    , running_(false) {
}

Timer::~Timer() {}

void Timer::reset() {
    elapsed_ = 0;
    start_time_ = 0;
    running_ = false;
}

void Timer::start() {
    if (running_) {
        throw runtime_error("Timer: already running");
    }

    running_ = true;
    start_time_ = clock();
}

double Timer::stop() {
    clock_t lap = clock() - start_time_;
    
    if (!running_) {
        throw runtime_error("Timer: already stopping");
    }

    running_ = false;
    elapsed_ += lap;

    return lap / (double)CLOCKS_PER_SEC;
}

double Timer::elapsed() const {
    return elapsed_ / (double)CLOCKS_PER_SEC;
}

} // namespace AHCParser
