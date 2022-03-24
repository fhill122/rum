/*
 * Created by Ivan B on 2022/3/23.
 */

#include "ros_pingpong.h"

using namespace std;

int main(int argc, char** argv){
    constexpr int kNumCycle = 1e5;

    ros::init(argc, argv, "Pingpong");
    ros::NodeHandle n;
    RosPing ping(kNumCycle, n);
    RosPong pong(n);

    ros::AsyncSpinner spinner(1);
    spinner.start();
    this_thread::sleep_for(500ms);

    ping.start();
    double time = ping.sillyWait();
    // 1e5: 700ms - 1500ms
    printf("%d ping-pong takes %.1fms\n", kNumCycle, time);

    ros::shutdown();
    return 0;
}