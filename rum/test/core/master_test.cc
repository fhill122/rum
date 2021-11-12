#include <gtest/gtest.h>
#include <rum/core/internal/master.h>
#include <rum/common/log.h>
#include <rum/extern/ivtb/stopwatch.h>

using namespace std;
using namespace rum;

// todo ivan. very rarely, this could fail.
//  most of the time, the failure reason is that system still holding the underlying socket,
//  so disconnection monitoring and periodic binding trial will fail.
//  ** Seems there's an exception: binding to 127.0.0.1 is always prompt
TEST(MasterTest, QuickCompete){
    // this destroys global master
    rum::Master::DbgGetGlobalMaster();
    this_thread::sleep_for(10ms);

    Log::I("QuickCompete", "create master 1");
    auto master1 = make_unique<rum::Master>(rum::shared_context());
    // make sure this wins
    this_thread::sleep_for(10ms);
    ASSERT_TRUE(master1->active.load(memory_order_relaxed));

    ivtb::Stopwatch timer;
    Log::I("QuickCompete", "create master 2");
    auto master2 = make_unique<rum::Master>(rum::shared_context());
    this_thread::sleep_for(10ms);

    Log::I("QuickCompete", "destroying master 1");
    master1.reset(nullptr);
    ivtb::Stopwatch timer2;
    while(timer2.passedMs() < 100 && !master2->active.load(memory_order_relaxed)){
        this_thread::sleep_for(1ms);
    }
    Log::I("QuickCompete", "rebind within %.1f ms", timer2.passedMs());
    ASSERT_TRUE(master2->active.load(memory_order_relaxed));

    Log::I("QuickCompete", "end");
}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
