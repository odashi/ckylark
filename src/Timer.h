#ifndef CKYLARK_TIMER_H_
#define CKYLARK_TIMER_H_

#include <ctime>

namespace Ckylark {

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

} // namespace Ckylark

#endif // CKYLARK_TIMER_H_

