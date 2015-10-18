#ifndef Timer_hpp
#define Timer_hpp

#include <chrono>

class Timer {
  public:
    Timer() { tic(); }
    void tic() { StartTime = clock::now(); }
    auto toc() const { return (clock::now() - StartTime).count(); }
    static auto ticksPerSecond() {
        double den = clock::duration::period::den;
        return den / clock::duration::period::num;
    }

  private:
    using clock = std::chrono::high_resolution_clock;
    clock::time_point StartTime;
};
#endif
