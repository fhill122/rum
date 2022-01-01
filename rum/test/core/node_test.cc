#include <gtest/gtest.h>
#include <rum/common/log.h>
#include <rum/common/common.h>
#include <rum/core/node_base.h>
#include <rum/core/internal/node_base_impl.h>
#include <rum/extern/ivtb/stopwatch.h>


using namespace std;
using namespace rum;

// TEST(NodeTest, Basic){
//     rum::NodeBaseImpl node;
// }

// TEST(NodeTest, Heatbeat){
//     rum::NodeBaseImpl node1;
//     rum::NodeBaseImpl node2;
//
//     node1.connect({std::pair<string, string>(rum::GetMasterInAddr(), rum::GetMasterOutAddr())});
//     node2.connect({std::pair<string, string>(rum::GetMasterInAddr(), rum::GetMasterOutAddr())});
//
//     usleep(9e5);
//     node1.shutdown();
//     node2.shutdown();
// }

/**
 * would fail if too many nodes:
 * - seg fault during connection/shutdown, no idea the cause
 * - system limit: too many open files
 *
 */
TEST(NodeTest, ManyNodes){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::i);

    // timing does not go linearly with number of nodes
    int n=200;
    ivtb::Stopwatch timer;
    vector<rum::NodeBaseImpl> nodes(n);
    rum::printer.i(__func__, "create time %.1f ms", timer.passedMs());

    timer.start();
    for (auto &node :nodes)
        node.connect(rum::GetMasterInAddr(), rum::GetMasterOutAddr());
    rum::printer.i(__func__, "connect time %.1f ms", timer.passedMs());

    timer.start();
    // segfault
    for (auto &node : nodes)
        node.shutdown();
    rum::printer.i(__func__, "shutdown time %.1f ms", timer.passedMs());
}

TEST(NodeTest, CreatePubSub){
    constexpr char kTopic[] = "topic";
    constexpr char kProtocol[] = "protocol";

    rum::NodeBaseImpl node;
    node.connect(rum::GetMasterInAddr(), rum::GetMasterOutAddr());
    this_thread::sleep_for(10ms);

    auto pub = node.addPublisher(kTopic, kProtocol);
    auto sub = node.addSubscriber(kTopic, make_shared<ivtb::ThreadPool>(1), 100,
            [](zmq::message_t&){}, [](const void*){}, kProtocol);
    rum::printer.i(__func__, "created");

    this_thread::sleep_for(1000ms);
    rum::printer.i(__func__, "shutting down");
    node.removePublisher(pub);
    node.removeSubscriber(sub);
    node.shutdown();
}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
