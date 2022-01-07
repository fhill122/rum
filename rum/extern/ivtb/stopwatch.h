//
// A stopwatch implementation with std::chrono::high_resolution_clock
// Created by Ivan B on 2021/8/26.
//

#ifndef IVTB_TOOLS_STOPWATCH_H_
#define IVTB_TOOLS_STOPWATCH_H_

#include <chrono>

namespace ivtb{

class Stopwatch{
  public:
    using Timestamp = std::chrono::high_resolution_clock::time_point;
    using Duration = std::chrono::high_resolution_clock::duration;

  private:
    Timestamp start_t;
    Duration accumulation{};
    bool paused = false;

  public:
    inline explicit Stopwatch(bool paused = false){
        if(paused){
            this->paused = true;
        }
        else{
            start();
        }
    }

    // start all over
    inline void start(){
        start_t = std::chrono::high_resolution_clock::now();
        accumulation = Duration();
        paused = false;
    }

    inline void pause(){
        if(paused) return;
        paused = true;
        accumulation += (std::chrono::high_resolution_clock::now() - start_t);
    }

    inline void unpause(){
        paused = false;
        start_t = std::chrono::high_resolution_clock::now();
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
            return std::chrono::high_resolution_clock::now() - start_t + accumulation;
        }
    }

};


}

#endif //IVTB_TOOLS_STOPWATCH_H_
