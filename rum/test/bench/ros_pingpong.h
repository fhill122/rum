/*
 * Created by Ivan B on 2022/3/23.
 */

#ifndef RUM_TEST_BENCH_ROS_PINGPONG_H_
#define RUM_TEST_BENCH_ROS_PINGPONG_H_

#include <thread>
#include <atomic>
#include <ros/ros.h>
#include <geometry_msgs/Pose.h>
#include <rum/extern/ivtb/stopwatch.h>
#include <rum/common/log.h>

static constexpr char kRosTopicPing[] = "Ping";
static constexpr char kRosTopicPong[] = "Pong";

struct RosPong{
    unsigned int progress = 0;
    ros::Subscriber ping_sub;
    ros::Publisher pong_pub;

    RosPong(ros::NodeHandle &nh) {
        pong_pub = nh.advertise<geometry_msgs::Pose>(kRosTopicPong, 2);
        ping_sub = nh.subscribe(kRosTopicPing, 2,
            (boost::function<void(const geometry_msgs::Pose::ConstPtr&)>)
            [this](const geometry_msgs::Pose::ConstPtr& msg){
                pingCb(msg);}
        );
    }

    void pingCb(const geometry_msgs::Pose::ConstPtr& msg){
        // printf("received ping of %.0f\n", msg->position.x);
        pong_pub.publish(msg);
        ++progress;
    }
};

struct RosPing{
    unsigned int n_cycle;
    unsigned int progress = 0;
    std::atomic_bool done{false};
    ivtb::Stopwatch stopwatch;

    ros::Publisher ping_pub;
    ros::Subscriber pong_sub;

    RosPing(unsigned int n, ros::NodeHandle &nh) : n_cycle(n){
        ping_pub = nh.advertise<geometry_msgs::Pose>(kRosTopicPing, 2);
        pong_sub = nh.subscribe(kRosTopicPong, 2,
            (boost::function<void(const geometry_msgs::Pose::ConstPtr&)>)
            [this](const geometry_msgs::Pose::ConstPtr& msg){
                pongCb(msg);}
        );
    }

    void pongCb(const geometry_msgs::Pose::ConstPtr& msg){
        AssertLog((unsigned int)msg->position.x == progress, "");
        if (++progress == n_cycle){
            stopwatch.pause();
            done.store(true, std::memory_order_release);
        } else{
            geometry_msgs::Pose reply = *msg;
            reply.position.x = msg->position.x + 1;
            ping_pub.publish(reply);
        }
    }

    void start(){
        stopwatch.start();
        geometry_msgs::Pose pose;
        pose.position.x = 0;
        pose.position.y = 1;
        pose.position.z = 1;
        pose.orientation.w = 1;
        pose.orientation.x = 0;
        pose.orientation.y = 0;
        pose.orientation.z = 0;
        ping_pub.publish(pose);
    }

    double sillyWait(){
        using namespace std;
        while(!done.load(std::memory_order_acquire)){
            std::this_thread::sleep_for(100ms);
        }
        return stopwatch.passedMs();
    }
};



#endif //RUM_TEST_BENCH_ROS_PINGPONG_H_
