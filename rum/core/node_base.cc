//
// Created by Ivan B on 2021/3/20.
//

#include "node_base.h"

#include "internal/node_base_impl.h"
#include "internal/subscriber_base_impl.h"
#include "rum/common/log.h"
#include "rum/common/common.h"
#include "rum/common/misc.h"


using namespace std;

namespace rum{

bool NodeBase::Init(const NodeParam &param) {
    static std::mutex mtx;
    lock_guard lock(mtx);
    if (GlobalNode())
        return false;
    GlobalNode() = unique_ptr<NodeBase>(new NodeBase("__global", param));
    GlobalNode()->pimpl_->connect(GetMasterInAddr(), GetMasterOutAddr());
    return true;
}

std::unique_ptr<NodeBase> &NodeBase::GlobalNode(bool auto_init) {
    static unique_ptr<NodeBase> node;
    if (auto_init)
        Init();
    return node;
}

NodeBase::NodeBase(const string &name, const NodeParam &param):
        pimpl_(make_unique<NodeBaseImpl>(name, param)){}

NodeBase::~NodeBase() =  default;

SubscriberBaseHandler
NodeBase::addSubscriber(const std::string &topic,
                        const shared_ptr<ThreadPool> &tp,
                        size_t queue_size,
                        const IpcFunc &ipc_cb,
                        const ItcFunc &itc_cb,
                        const DeserFunc<> &deserialize_f,
                        const std::string &protocol) {
    AssertLog(!StrStartWith(topic, kTopicReserve), "reserved topic name");
    AssertLog(tp->threads()>0, "empty thread pool");
    return SubscriberBaseHandler(pimpl_->addSubscriber(topic, tp, queue_size, ipc_cb, itc_cb, deserialize_f, protocol));
}

void NodeBase::removeSubscriber(SubscriberBaseHandler &subscriber_handler) {
    if (subscriber_handler.pimpl_)
        pimpl_->removeSubscriber(subscriber_handler.pimpl_);
}

PublisherBaseHandler NodeBase::addPublisher(const std::string &topic,
                                            const std::string &protocol) {
    AssertLog(!StrStartWith(topic, kTopicReserve), "reserved topic name");
    return PublisherBaseHandler(pimpl_->addPublisher(topic, protocol));
}

void NodeBase::removePublisher(PublisherBaseHandler &publisher_handler) {
    if (publisher_handler.pimpl_)
        pimpl_->removePublisher(publisher_handler.pimpl_);
}

ClientBaseHandler NodeBase::addClient(const string &srv_name, const string &req_protocol) {
    AssertLog(!StrStartWith(srv_name, kTopicReserve), "reserved srv name");
    return ClientBaseHandler(pimpl_->addClient(srv_name, req_protocol));
}

void NodeBase::removeClient(ClientBaseHandler &client_handler) {
    if (client_handler.pimpl_)
        pimpl_->removeClient(client_handler.pimpl_);
}

ServerBaseHandler NodeBase::addServer(const string &srv_name,
                                      const shared_ptr<ThreadPool> &tp,
                                      size_t queue_size,
                                      const SrvIpcFunc &ipc_func,
                                      const SrvItcFunc &itc_func,
                                      const string &req_protocol,
                                      const string &rep_protocol) {
    AssertLog(!StrStartWith(srv_name, kTopicReserve), "reserved srv name");
    return ServerBaseHandler(pimpl_->addServer(srv_name, tp, queue_size,
                                               ipc_func, itc_func, req_protocol, rep_protocol));
}

void NodeBase::removeServer(ServerBaseHandler &server_handler) {
    if (server_handler.pimpl_)
        pimpl_->removeServer(server_handler.pimpl_);
}

}