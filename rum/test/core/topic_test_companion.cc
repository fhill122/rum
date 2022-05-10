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
    PublisherBaseImpl *pub_ = nullptr;
    SubscriberBaseImpl *sub_ = nullptr;

  private:

  public:
    ImplTest() {}

    void init(NodeParam param = NodeParam()){
        node_ = make_unique<NodeBaseImpl>("", move(param));
        node_->connect(GetMasterInAddr(), GetMasterOutAddr());
        this_thread::sleep_for(50ms);
        pub_ = node_->addPublisher(kTopic, kProtocol);
        sub_ = node_->addSubscriber(kTopic, make_shared<ivtb::ThreadPool>(1), 100,
                                    [](const shared_ptr<const void> &){},
                                    [](const shared_ptr<const void> &){},
                                    [](shared_ptr<const Message> &msg, const string&){return move(msg);},
                                    kProtocol);
        // have to be greater than heartbeat
        this_thread::sleep_for((kNodeHbPeriod+50)*1ms);
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
    atomic_int shared_intra_proc_count{0};

    void init(int n_topics, int n_pubs,
              NodeParam param = NodeParam()){
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

        // have to be greater than heartbeat
        this_thread::sleep_for((kNodeHbPeriod+50)*1ms);
    }
};

void IpcBasic_both(){
    Log::I(__func__, "start");
    ImplTest impl_test;
    impl_test.init();
    for (int i = 0; i < 10; ++i) {
        impl_test.pub_->publish(zmq::message_t(1));
    }
    this_thread::sleep_for(10ms);
    Log::I(__func__, "end");
}

void IpcBasic_ipc(){
    Log::I(__func__, "start");
    ImplTest impl_test;
    NodeParam param;
    param.enable_tcp_tx = false;
    impl_test.init(param);
    for (int i = 0; i < 10; ++i) {
        impl_test.pub_->publish(zmq::message_t(1));
    }
    this_thread::sleep_for(10ms);
    Log::I(__func__, "end");
}

void IpcBasic_tcp(){
    Log::I(__func__, "start");
    ImplTest impl_test;
    NodeParam param;
    param.enable_ipc_txrx = false;
    impl_test.init(param);
    for (int i = 0; i < 10; ++i) {
        impl_test.pub_->publish(zmq::message_t(1));
    }
    this_thread::sleep_for(10ms);
    Log::I(__func__, "end");
}

void RemoteCrash(){
    Log::I(__func__, "start");
    ImplTest impl_test;
    impl_test.init();

    this_thread::sleep_for(3*(kNodeHbPeriod+50)*1ms);

    Log::I(__func__, "crash!");
    abort();
}

void RemoteSubRemoval(){
    Log::I(__func__, "start");
    ImplTest impl_test;
    impl_test.init();

    this_thread::sleep_for(3*(kNodeHbPeriod+50)*1ms);
    Log::I(__func__, "remove sub!");
    impl_test.node_->removeSubscriber(impl_test.sub_);
    this_thread::sleep_for(50ms);

}

void MultiIpc(){
    constexpr int kNTopics = 10;
    constexpr int kNPubs = 10;

    Log::I(__func__, "start");
    ImplMultiTest test;
    test.init(kNTopics,kNPubs);
    for (int i=0; i<100; ++i){
        for (int j=0; j<kNTopics; ++j) {
            for (int k=0; k<kNPubs; ++k)
                test.pubs_[j][k]->publish(zmq::message_t(1));
        }
        this_thread::sleep_for(1ms);
    }
    this_thread::sleep_for(20ms);
    Log::I(__func__, "end");
}

void MultiTcp(){
    constexpr int kNTopics = 10;
    constexpr int kNPubs = 10;

    Log::I(__func__, "start");
    ImplMultiTest test;
    NodeParam param;
    param.enable_ipc_txrx = false;
    test.init(kNTopics,kNPubs, move(param));
    for (int i=0; i<100; ++i){
        for (int j=0; j<kNTopics; ++j) {
            for (int k=0; k<kNPubs; ++k)
                test.pubs_[j][k]->publish(zmq::message_t(1));
        }
        this_thread::sleep_for(1ms);
    }
    this_thread::sleep_for(20ms);
    Log::I(__func__, "end");
}

int main(int argc, char* argv[]){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::d);
    // zmq_force_delay = 50;

    AssertLog(argc>1, "input command");
    string cmd = argv[1];
    Log::I(__func__, "invoked with " + cmd );
    if (cmd == "IpcBasic_both"){
        IpcBasic_both();
    }
    else if (cmd == "IpcBasic_tcp"){
        IpcBasic_tcp();
    }
    else if (cmd == "IpcBasic_ipc"){
        IpcBasic_ipc();
    }
    else if (cmd == "RemoteCrash"){
        RemoteCrash();
    }
    else if (cmd == "RemoteSubRemoval"){
        RemoteSubRemoval();
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
