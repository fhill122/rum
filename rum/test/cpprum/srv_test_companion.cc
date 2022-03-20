/*
 * Created by Ivan B on 2022/3/10.
 */

#include <rum/common/log.h>
#include <rum/common/common.h>

#include "srv_test_common.h"

using namespace std;
using namespace rum;

void BasicInterP(){
    auto server = CreateServer<Message,FbsBuilder,SerializerFbs>(
            kSrv, bind(ServerFbCallback, placeholders::_1, placeholders::_2, 0) );
    Log::I(__func__, "sleep");
    this_thread::sleep_for((kNodeHbPeriod+100)*1ms);
}


int main(int argc, char** argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::d);

    AssertLog(argc>1, "input command");
    CompanionCmd cmd = static_cast<CompanionCmd>(stoi(argv[1]));
    Log::I(__func__, "invoked with %d", cmd );

    switch (cmd) {
        case CompanionCmd::BasicInterP:
            BasicInterP();
            break;
        case CompanionCmd::BasicIpcInterP:
            break;
        case CompanionCmd::BasicTcpInterP:break;
    }
    Log::I(__func__, "companion end");


    return 0;
}
