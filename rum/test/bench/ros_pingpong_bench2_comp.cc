/*
 * Created by Ivan B on 2022/3/23.
 */

#include "ros_pingpong.h"

using namespace std;

int main(int argc, char** argv){
    AssertLog(argc==2, "");
    const int kNumCycle = stoi(argv[1]);
    printf("pong back %d times\n", kNumCycle);

    ros::init(argc, argv, "Pingpong2Companion");
    ros::NodeHandle nh;

    RosPong pong(nh);
    ros::AsyncSpinner spinner(1);
    spinner.start();

    while (pong.progress < kNumCycle){
        std::this_thread::sleep_for(100ms);
    }

    ros::shutdown();
}
