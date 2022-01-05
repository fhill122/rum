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

class ImplMultiTest{
  private:
  public:
    const string kTopicPrefix = "TestTopic";
    const string kProtocol = "protocol";
    unique_ptr<NodeBaseImpl> node_;
    vector<vector<PublisherBaseImpl*>> pubs_;
    atomic_int shared_ipc_count{0};
    atomic_int shared_itc_count{0};

    void init(int n_topics, int n_pubs,
              NodeBaseImpl::Param param = NodeBaseImpl::Param()){
        node_ = make_unique<NodeBaseImpl>("", move(param));
        node_->connect(GetMasterInAddr(), GetMasterOutAddr());

        pubs_.resize(n_topics);
        for (int i=0; i<n_topics; ++i){
            string topic = kTopicPrefix + to_string(i);
            pubs_[i].resize(n_pubs);
            for(int j=0; j<n_pubs; ++j){
                pubs_[i][j] = node_->addPublisher(topic, kProtocol);
            }
        }
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

void TcpBasic(){
    Log::I(__func__, "start");
    ImplTest impl_test;
    NodeBaseImpl::Param param;
    param.enable_ipc_socket = false;
    impl_test.init(param);
    // have to be greater than heartbeat
    this_thread::sleep_for((kNodeHbPeriod+50)*1ms);
    for (int i = 0; i < 10; ++i) {
        impl_test.pub_->publishIpc(zmq::message_t(1));
    }
    this_thread::sleep_for(10ms);
    Log::I(__func__, "end");
}

void MultiIpc(){
    constexpr int kNTopics = 10;
    constexpr int kNPubs = 10;

    Log::I(__func__, "start");
    ImplMultiTest test;
    test.init(kNTopics,kNPubs);
    // have to be greater than heartbeat
    this_thread::sleep_for((kNodeHbPeriod+50)*1ms);
    for (int i=0; i<100; ++i){
        for (int j=0; j<kNTopics; ++j) {
            for (int k=0; k<kNPubs; ++k)
                test.pubs_[j][k]->publishIpc(zmq::message_t(1));
        }
        this_thread::sleep_for(100us);
    }
    this_thread::sleep_for(10ms);
    Log::I(__func__, "end");
}

void MultiTcp(){
    constexpr int kNTopics = 10;
    constexpr int kNPubs = 10;

    Log::I(__func__, "start");
    ImplMultiTest test;
    NodeBaseImpl::Param param;
    param.enable_ipc_socket = false;
    test.init(kNTopics,kNPubs, move(param));
    // have to be greater than heartbeat
    this_thread::sleep_for((kNodeHbPeriod+50)*1ms);
    for (int i=0; i<100; ++i){
        for (int j=0; j<kNTopics; ++j) {
            for (int k=0; k<kNPubs; ++k)
                test.pubs_[j][k]->publishIpc(zmq::message_t(1));
        }
        this_thread::sleep_for(100us);
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
        TcpBasic();
    }
    else if (cmd == "MultiIpc"){
        MultiIpc();
    }
    else if (cmd == "MultiTcp"){
        MultiTcp();
    }
    else {
        AssertLog(false, "no command found");
    }
}
