//
// A stopwatch implementation with any std::chrono clock type, default with high_resolution_clock
// Created by Ivan B on 2021/8/26.
//

#ifndef IVTB_TOOLS_STOPWATCH_H_
#define IVTB_TOOLS_STOPWATCH_H_

#include <chrono>
#include <cstdio>
#include <iostream>
#include <cfloat>

namespace ivtb{


template<class ClockType = std::chrono::high_resolution_clock>
class StopwatchAny{
  public:
    using Timestamp = typename ClockType::time_point;
    using Duration = typename ClockType::duration;

    inline static double GetEpochOffset(){
        if constexpr(std::is_same<std::chrono::system_clock,ClockType>::value) return 0;

        double t_diff_abs_min = DBL_MAX;
        double t_diff_final;
        // try to find minimum time offset
        for (int i = 0; i < 100; ++i) {
            double t_diff =
                    std::chrono::duration_cast<std::chrono::duration<double>>(ClockType::now().time_since_epoch() ).count()
                    - std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::system_clock::now().time_since_epoch() ).count();
            if(abs(t_diff)<t_diff_abs_min){
                t_diff_abs_min = abs(t_diff);
                t_diff_final = t_diff;
                // printf("updated offset with %f on trial %d \n", t_diff_final, i);
            }
        }
        if (t_diff_abs_min<1e-3) t_diff_final = 0;
        // printf("offset of %f seconds\n", t_diff_final);
        return -t_diff_final;
    }

    inline static const double kEpochOffset = GetEpochOffset();

  private:
    Timestamp start_t;
    Duration accumulation{};
    bool paused = false;

  public:
    inline explicit StopwatchAny(bool paused = false){
        if(paused){
            this->paused = true;
        }
        else{
            start();
        }
    }

    // start all over
    inline void start(){
        start_t = ClockType::now();
        accumulation = Duration();
        paused = false;
    }

    inline void pause(){
        if(paused) return;
        paused = true;
        accumulation += (ClockType::now() - start_t);
    }

    inline void unpause(){
        paused = false;
        start_t = ClockType::now();
    }

    inline bool isPaused(){
        return paused;
    }

    inline double passedSeconds(){
        auto duration = passedTime();
        return std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
    }

    inline double passedMs(){
        auto duration = passedTime();
        return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1,1000>>>(duration).count();
    }

    inline Duration passedTime(){
        if (paused){
            return accumulation;
        }
        else{
            return ClockType::now() - start_t + accumulation;
        }
    }

    /**
     * Get current time since epoch in seconds
     * @return seconds
     */
    static inline double Now(){
        return std::chrono::duration_cast<std::chrono::duration<double>>(
                ClockType::now().time_since_epoch() ).count()  + kEpochOffset;
    }

};


// default clock type, high resolution
using Stopwatch = ivtb::StopwatchAny<>;

// monotonic timer
using StopwatchMono = ivtb::StopwatchAny<std::chrono::steady_clock>;

// system clock
using StopwatchSystem = ivtb::StopwatchAny<std::chrono::system_clock>;

}

#endif //IVTB_TOOLS_STOPWATCH_H_
