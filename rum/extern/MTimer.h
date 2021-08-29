#ifndef __MTIMER_H__
#define __MTIMER_H__

#include <sys/time.h>
#include <cmath>

typedef unsigned long long timestamp_t;

class MTimer{
private:
    timestamp_t startingTime;
public:
    MTimer(){start();}
    void start(){ startingTime =  getTimestamp();}

    double passedMs_f(){ return 1. * (getTimestamp() - startingTime) / 1000;}
    timestamp_t passedMicroS(){ return getTimestamp() - startingTime; };
    timestamp_t passedMs(){ return lround(passedMs_f());}

    // get absolute us since Epoch
    static timestamp_t getTimestamp(){
      struct timeval now;
      gettimeofday (&now, NULL);
      return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000 + 0.5;
    }

    // get absolute ms since Epoch
    static timestamp_t getTimestampMs(){
      struct timeval now;
      gettimeofday (&now, NULL);
      return now.tv_usec/1000 + now.tv_sec*1000 + 0.5;
    }
};

#endif
