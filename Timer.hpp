#ifndef Timer_hpp
#define Timer_hpp

#include <chrono>

class Timer {
  public:
    Timer() {
        tic();
        TicksPerSeconds =
            clock::duration::period::den / clock::duration::period::num;
    }

    void tic() { StartTime = clock::now(); }

    auto toc() const { return (clock::now() - StartTime).count(); }

    auto ticksPerSecond() const { return TicksPerSeconds; }

  private:
    using clock = std::chrono::high_resolution_clock;
    clock::time_point StartTime;
    double TicksPerSeconds;
};

#endif
