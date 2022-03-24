/*
 * Created by Ivan B on 2022/3/23.
 */

#include "ros_pingpong.h"

using namespace std;

int main(int argc, char** argv){
    constexpr int kNumCycle = 1e4;

    string cmd = string(argv[0]) + "_comp " + to_string(kNumCycle);
    thread pong_t([&]{system(cmd.c_str());});

    ros::init(argc, argv, "Pingpong2");
    ros::NodeHandle n;
    RosPing ping(kNumCycle, n);

    ros::AsyncSpinner spinner(1);
    spinner.start();
    this_thread::sleep_for(1000ms);

    printf("start\n");
    ping.start();
    double time = ping.sillyWait();
    // 1e5: 700ms - 1500ms
    printf("%d ping-pong takes %.1fms\n", kNumCycle, time);

    pong_t.join();
    ros::shutdown();
    return 0;
}
