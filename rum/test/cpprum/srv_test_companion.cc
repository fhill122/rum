/*
 * Created by Ivan B on 2022/3/10.
 */

#include <rum/common/log.h>
#include <rum/common/common.h>
#include <rum/extern/ivtb/stopwatch.h>

#include "srv_test_common.h"

using namespace std;
using namespace rum;

void BasicInterP(){
    atomic_int count{0};
    auto server = CreateServer<Message,FbsBuilder,SerializerFbs>(
            kSrv, bind(ServerFbCallback, placeholders::_1, placeholders::_2, 0, &count) );
    Log::I(__func__, "sleep");
    ivtb::StopwatchMono stopwatch;
    while(stopwatch.passedMs()<kNodeHbPeriod+1000){
        if (count.load()==1) break;
        this_thread::sleep_for(10ms);
    }
}

void BasicTcpInterP(){
    NodeParam param;
    param.enable_ipc_txrx = false;
    rum::Init(param);
    atomic_int count{0};
    auto server = CreateServer<Message,FbsBuilder,SerializerFbs>(
            kSrv, bind(ServerFbCallback, placeholders::_1, placeholders::_2, 0, &count) );
    Log::I(__func__, "sleep");
    ivtb::StopwatchMono stopwatch;
    while(stopwatch.passedMs()<kNodeHbPeriod+1000){
        if (count.load()==1) break;
        this_thread::sleep_for(10ms);
    }
}

void BasicInterP2(){
    auto client = CreateClient<FbsBuilder,Message,SerializerFbs>(kSrv);
    bool ping_ok = client->ping(kNodeHbPeriod+1000, 100);
    AssertLog(ping_ok, "ping failed");

    auto direct_res = client->callForeground(CreateReqeust(1,1,0), 50);
    AssertLog(direct_res.status==SrvStatus::ServerErr, "");
};

void Timeout(){
    atomic_int count{0};
    auto server = CreateServer<Message,FbsBuilder,SerializerFbs>(
            kSrv, bind(ServerFbCallback, placeholders::_1, placeholders::_2, 10, &count) );
    Log::I(__func__, "sleep");
    ivtb::StopwatchMono stopwatch;
    while(stopwatch.passedMs()<kNodeHbPeriod+1000){
        if (count.load()==2) break;
        this_thread::sleep_for(10ms);
    }
}

void SafeEnding(){
    atomic_int count{0};
    auto server = CreateServer<Message,FbsBuilder,SerializerFbs>(
            kSrv, bind(ServerFbCallback, placeholders::_1, placeholders::_2, 10, &count) );
    Log::I(__func__, "sleep");
    ivtb::StopwatchMono stopwatch;
    while(stopwatch.passedMs()<kNodeHbPeriod+1000){
        if (count.load()==1) break;
        this_thread::sleep_for(1ms);
    }
}

void Cancelling(){
    atomic_int count{0};
    auto server = CreateServer<Message,FbsBuilder,SerializerFbs>(
            kSrv, bind(ServerFbCallback, placeholders::_1, placeholders::_2, 10, &count) );
    Log::I(__func__, "sleep");
    ivtb::StopwatchMono stopwatch;
    while(stopwatch.passedMs()<kNodeHbPeriod+1000){
        if (count.load()==1) break;
        this_thread::sleep_for(1ms);
    }
}

void Overflow(){
    atomic_int count{0};
    auto server = CreateServer<Message,FbsBuilder,SerializerFbs>(
            kSrv, bind(ServerFbCallback, placeholders::_1, placeholders::_2, 10, &count), 1);
    Log::I(__func__, "sleep");
    ivtb::StopwatchMono stopwatch;
    while(stopwatch.passedMs()<kNodeHbPeriod+1000){
        if (count.load()==2) break;
        this_thread::sleep_for(10ms);
    }
}

int main(int argc, char** argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::i);

    AssertLog(argc>1, "input command");
    CompanionCmd cmd = static_cast<CompanionCmd>(stoi(argv[1]));
    Log::I(__func__, "invoked with %d", cmd );

    switch (cmd) {
        case CompanionCmd::BasicInterP:
            BasicInterP();
            break;
        case CompanionCmd::BasicInterP2:
            BasicInterP2();
            break;
        case CompanionCmd::BasicTcpInterP:
            BasicTcpInterP();
            break;
        case CompanionCmd::Timeout:
            Timeout();
            break;
        case CompanionCmd::SafeEnding:
            SafeEnding();
            break;
        case CompanionCmd::Cancelling:
            Cancelling();
            break;
        case CompanionCmd::Overflow:
            Overflow();
            break;
        default:
            Log::E(__func__, "not implemented for %d", cmd);
            return 1;
    }
    Log::I(__func__, "companion end");


    return 0;
}
