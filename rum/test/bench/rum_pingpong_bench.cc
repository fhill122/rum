/*
 * Created by Ivan B on 2022/3/23.
 */

#include "rum_pingpong.h"

using namespace std;

int main(int argc, char** argv){
    constexpr int kNumCycle = 1e5;

    rum::RumPing ping(kNumCycle);
    rum::RumPong pong;

    this_thread::sleep_for(500ms);

    ping.start();
    double time = ping.sillyWait();
    // 1e5: 55ms
    printf("%d ping-pong takes %.1fms\n", kNumCycle, time);
    return 0;
}
