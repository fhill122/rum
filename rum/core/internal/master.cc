//
// Created by Ivan B on 2021/4/5.
//

#include "master.h"

#include <rum/common/common.h>
#include <rum/common/log.h>
#include "rum/common/zmq_helper.h"

using namespace std;

constexpr char TAG[] = "Master";

namespace rum {

unique_ptr<Master> CreateGlobalMaster(){
    return make_unique<Master>(shared_context());
}

unique_ptr<Master> Master::master_ = CreateGlobalMaster();

Master::Master(std::shared_ptr<zmq::context_t> context) :
        context_(std::move(context)) {
    sub_container_ = make_unique<SubContainer>(context_);
    auto sync_tp = std::make_shared<ThreadPool>(1);
    sub_ = make_unique<SubscriberBaseImpl>(kSyncTopic, move(sync_tp), 0,
           [this](zmq::message_t& msg){syncForward(msg);}, nullptr);
    sub_container_->addSub(sub_.get());

    pub_ = make_unique<PublisherBaseImpl>(kSyncTopic, "", context_, true);

    // start binding thread. This is a must as monitoring may fail
    bind_t_ = make_unique<thread>([this]{
        tu::NameThread("RumMasterBind");
        unique_lock<mutex> lock(bind_mu_);
        while(to_bind_){
            // bind inbound addr if not before
            if (sub_container_->getTcpAddr().empty()){
                if (sub_container_->bindTcpRaw(GetMasterInAddr())){
                    if (pub_->bindTcpRaw(GetMasterOutAddr())){
                        goto success;
                    }
                }
                else{
                    // log.v(TAG, "bind failed for inward addr");
                }
            }
            // bind outbound addr if inbound is already bound
            else if (pub_->bindTcpRaw(GetMasterOutAddr())){
                goto success;
            }
            else{
                log.w(TAG, "inward bound, but binding failed for outward addr.");
            }

            // two wakeup sources: disconnection monitoring, destructor
            // bind_cv_.wait_for(lock, int(trial_period_)*1ms,[this]{return !to_bind_;});
            // log.v(TAG, "wait");
            bind_cv_.wait_for(lock, int(kMasterTrialPeriod)*1ms);
            continue;

            success:
            active.store(true, memory_order_release);
            sub_container_->start();
            log.i(TAG, "Won master election!");
            return;
        }
    });

    // [optional] monitor active master live, immediately try to bind if dead.
    // But this cannot guarantee a timely manner
    monitor_socket_ = make_unique<zmq::socket_t>(*context_, ZMQ_PUB);
    // int reconnect_period = 500;
    // monitor_socket_->setsockopt(ZMQ_RECONNECT_IVL, &reconnect_period, sizeof(reconnect_period));
    monitor_socket_->connect(GetMasterInAddr());
    monitor_ = make_unique<ZmqMonitor>(*monitor_socket_);
    monitor_->start([this](const zmq_event_t &event, const char *address){
        switch (event.event) {
            case ZMQ_EVENT_DISCONNECTED:
                // log.w(TAG, "monitored disconnection from active master");
                bind_cv_.notify_one();
                break;
            default: break;
        }
    });
}

Master::~Master() {
    if (bind_t_){
        if (bind_t_->joinable()){
            {
                lock_guard<mutex> lock(bind_mu_);
                to_bind_ = false;
            }
            bind_cv_.notify_one();
            bind_t_->join();
        }
    }

    /*
     * to prevent program quiting too quickly:
     * https://github.com/zeromq/libzmq/issues/3442
     * we add sleep here, and removed context closing
     * maybe ditch multipart message
     */

    // sub_container_->stop();
    // monitor_.reset();
    this_thread::sleep_for(100ms);
    // shared_context()->close();  // this will crash immediately

    /* err source
    void zmq::session_base_t::clean_pipes ()
    {
        zmq_assert (_pipe != NULL);

        //  Get rid of half-processed messages in the out pipe. Flush any
        //  unflushed messages upstream.
        _pipe->rollback ();
        _pipe->flush ();

        //  Remove any half-read message from the in pipe.
        while (_incomplete_in) {
            msg_t msg;
            int rc = msg.init ();
            errno_assert (rc == 0);
            rc = pull_msg (&msg);
            errno_assert (rc == 0);    // <-- crash here
            rc = msg.close ();
            errno_assert (rc == 0);
        }
    }
    */
}

void Master::syncForward(zmq::message_t& msg) {
    pub_->publishIpc(msg);
    log.v(TAG, "forwarded a sync msg");
}

}