/*
 * Created by Ivan B on 2022/3/23.
 */

#include "rum_pingpong.h"

using namespace std;

int main(int argc, char** argv){
    constexpr int kNumCycle = 1e4;

    // rum::NodeParam param;
    // param.enable_ipc_txrx = false;
    // rum::Init(param);

    string cmd = string(argv[0]) + "_comp " + to_string(kNumCycle);
    thread pong_t([&]{system(cmd.c_str());});

    rum::RumPing ping(kNumCycle);

    this_thread::sleep_for(500ms);

    printf("start\n");
    ping.start();
    double time = ping.sillyWait();
    // 1e5: 55ms
    printf("%d ping-pong takes %.1fms\n", kNumCycle, time);

    pong_t.join();
    return 0;
}