/*
 * Created by Ivan B on 2022/3/23.
 */

#include "rum_pingpong.h"

using namespace std;

int main(int argc, char** argv){
    AssertLog(argc==2, "");
    const int kNumCycle = stoi(argv[1]);
    printf("pong back %d times\n", kNumCycle);

    // rum::NodeParam param;
    // param.enable_ipc_txrx = false;
    // rum::Init(param);
    rum::RumPong pong;

    while (pong.progress < kNumCycle){
        std::this_thread::sleep_for(100ms);
    }
}
