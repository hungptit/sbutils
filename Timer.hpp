#ifndef Timer_hpp
#define Timer_hpp

#include <chrono>

namespace utils {
    enum DispOpt { MICROSECOND = 1000000, MILLISECOND = 1000, SECOND = 1 };
    template <DispOpt val> std::string getUnitString();

    template <> std::string getUnitString<MICROSECOND>() {
        return " microseconds";
    }
    template <> std::string getUnitString<MILLISECOND>() {
        return " milliseconds";
    }
    template <> std::string getUnitString<SECOND>() { return " seconds"; }

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


    template <DispOpt val> class ElapsedTime {
      public:
        ~ElapsedTime() {
            std::cout << "Elapsed time: "
                      << Timer_.toc() * val / Timer_.ticksPerSecond()
                      << " " << getUnitString<val>() << std::endl;
        }

      private:
        Timer Timer_;
    };
}
#endif
