/*
 * Created by Ivan B on 2022/3/23.
 */

#ifndef RUM_TEST_BENCH_RUM_PINGPONG_H_
#define RUM_TEST_BENCH_RUM_PINGPONG_H_

#define RUM_DEF_SERIALIZER SerializerNative
#include <rum/cpprum/serialization/native/serializer_native.h>
#include <rum/cpprum/rum.h>
#include <rum/extern/ivtb/stopwatch.h>
#include <geometry_msgs/Pose.h>

namespace rum {

using Pose = ::geometry_msgs::Pose;

static constexpr char kRumTopicPing[] = "Ping";
static constexpr char kRumTopicPong[] = "Pong";

static std::shared_ptr<ThreadPool> shared_tp = std::make_shared<ThreadPool>(1);

struct RumPong {
    unsigned int progress = 0;
    Subscriber::UniquePtr ping_sub;
    Publisher<Pose>::UniquePtr pong_pub;

    RumPong(){
        pong_pub = CreatePublisher<Pose>(kRumTopicPong);
        ping_sub = CreateSubscriber<Pose>(kRumTopicPing,
            [this](const std::shared_ptr<const Pose>& msg){ pingCb(msg);}, 2, shared_tp);
    }

    void pingCb(const std::shared_ptr<const Pose>& msg){
        // printf("received ping of %.0f\n", msg->position.x);
        pong_pub->pub(msg);
        ++progress;
    }
};

struct RumPing {
    unsigned int n_cycle;
    unsigned int progress = 0;
    std::atomic_bool done{false};
    ivtb::Stopwatch stopwatch;

    Publisher<Pose>::UniquePtr ping_pub;
    Subscriber::UniquePtr pong_sub;

    RumPing(unsigned int n): n_cycle(n){
        ping_pub = CreatePublisher<Pose>(kRumTopicPing);
        pong_sub = CreateSubscriber<Pose>(kRumTopicPong,
                [this](const std::shared_ptr<const Pose>& msg){pongCb(msg);}, 2, shared_tp);
    }

    void pongCb(const std::shared_ptr<const Pose>& msg){
        AssertLog((unsigned int)msg->position.x == progress, "");
        if (++progress == n_cycle){
            stopwatch.pause();
            done.store(true, std::memory_order_release);
        } else{
            Pose reply = *msg;
            reply.position.x = msg->position.x + 1;
            ping_pub->pub(reply);
        }
    }

    void start(){
        stopwatch.start();
        Pose pose;
        pose.position.x = 0;
        pose.position.y = 1;
        pose.position.z = 1;
        pose.orientation.w = 1;
        pose.orientation.x = 0;
        pose.orientation.y = 0;
        pose.orientation.z = 0;
        ping_pub->pub(pose);
    }

    double sillyWait(){
        using namespace std;
        while(!done.load(std::memory_order_acquire)){
            std::this_thread::sleep_for(100ms);
        }
        return stopwatch.passedMs();
    }
};

}


#endif //RUM_TEST_BENCH_RUM_PINGPONG_H_
