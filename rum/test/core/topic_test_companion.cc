/*
 * Created by Ivan B on 2022/1/3.
 */

#include <string>
#include <rum/common/log.h>
#include <rum/core/internal/node_base_impl.h>
#include <rum/common/common.h>

using namespace std;
using namespace rum;

class ImplTest{
  private:
  public:
    static constexpr char kTopic[] = "TestTopic";
    static constexpr char kProtocol[] = "protocol";
    unique_ptr<NodeBaseImpl> node_;
    PublisherBaseImpl *pub_;
    SubscriberBaseImpl *sub_ = nullptr;

  private:

  public:
    ImplTest() {}

    void init(NodeBaseImpl::Param param = NodeBaseImpl::Param()){
        node_ = make_unique<NodeBaseImpl>("", move(param));
        node_->connect(GetMasterInAddr(), GetMasterOutAddr());
        this_thread::sleep_for(50ms);
        pub_ = node_->addPublisher(kTopic, kProtocol);
    }

};

void IpcBasic(){
    Log::I(__func__, "start");
    ImplTest impl_test;
    impl_test.init();
    // have to be greater than heartbeat
    this_thread::sleep_for((kNodeHbPeriod+50)*1ms);
    for (int i = 0; i < 10; ++i) {
        impl_test.pub_->publishIpc(zmq::message_t(1));
    }
    this_thread::sleep_for(10ms);
    Log::I(__func__, "end");
}

int main(int argc, char* argv[]){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::d);
    AssertLog(argc>1, "input command");
    string cmd = argv[1];
    if (cmd == "IpcBasic"){
        IpcBasic();
    }
    else if (cmd == "TcpBasic"){
        IpcBasic();
    }
    else {
        AssertLog(false, "no command found");
    }
}
