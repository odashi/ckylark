#ifndef AHCP_TIMER_H_
#define AHCP_TIMER_H_

#include <ctime>

namespace AHCParser {

class Timer {

    Timer(const Timer &) = delete;

public:
    Timer();
    ~Timer();

    void reset();

    void start();
    double stop();

    double elapsed() const;

private:
    clock_t elapsed_;
    clock_t start_time_;
    bool running_;

}; // class Timer

} // namespace AHCParser

#endif // AHCP_TIMER_H_

