//
// A stopwatch implementation with any std::chrono clock type, default with high_resolution_clock
// Created by Ivan B on 2021/8/26.
//

#ifndef IVTB_TOOLS_STOPWATCH_H_
#define IVTB_TOOLS_STOPWATCH_H_

#include <chrono>

namespace ivtb{


template<class ClockType = std::chrono::high_resolution_clock>
class StopwatchAny{
  public:
    using Timestamp = typename ClockType::time_point;
    using Duration = typename ClockType::duration;

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

};

using Stopwatch = ivtb::StopwatchAny<>;
using StopwatchMono = ivtb::StopwatchAny<std::chrono::steady_clock>;

}

#endif //IVTB_TOOLS_STOPWATCH_H_
