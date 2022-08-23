/*
 * Created by Ivan B on 2022/1/10.
 */

#ifndef RUM_CPPRUM_RUM_H_
#define RUM_CPPRUM_RUM_H_

#include "rum/core/node_base.h"
#include "publisher.h"
#include "subscriber.h"
#include "client.h"
#include "server.h"

#ifndef RUM_DEF_SERIALIZER
#include "serialization/flatbuffers/serializer_fbs.h"
#define RUM_DEF_SERIALIZER SerializerFbs
#endif

namespace rum {

static inline bool Init(const NodeParam &param = NodeParam()) {
    return NodeBase::Init(param);
}

template<class MsgT, class SubSerializerT = RUM_DEF_SERIALIZER>
[[nodiscard]] std::unique_ptr<Subscriber> CreateSubscriber(
        const std::string &topic,
        const SubFunc<MsgT> &callback_f,
        size_t queue_size = 1000,
        const std::shared_ptr<ThreadPool> &tp = std::make_shared<ThreadPool>(1));

template <class MsgT, class PubSerializerT = RUM_DEF_SERIALIZER>
[[nodiscard]] std::unique_ptr<Publisher<MsgT>> CreatePublisher(const std::string &topic);

template<class ReqT, class RepT, class ReqSerializerT = RUM_DEF_SERIALIZER, class RepSerializerT = ReqSerializerT>
[[nodiscard]] std::unique_ptr<Client<ReqT,RepT>> CreateClient(const std::string &srv_name);

// todo ivan. extra srv status if server dropped due to hwm
template<class ReqT, class RepT, class ReqSerializerT = RUM_DEF_SERIALIZER, class RepSerializerT = ReqSerializerT>
[[nodiscard]] std::unique_ptr<Server> CreateServer(
        const std::string &srv_name,
        const SrvFunc<ReqT, RepT> &callback_f,
        size_t queue_size = 2,
        const std::shared_ptr<ThreadPool> &tp = std::make_shared<ThreadPool>(1));

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class MsgT, class SubSerializerT>
Subscriber::UniquePtr CreateSubscriber(
        const std::string &topic, const SubFunc<MsgT> &callback_f,
        size_t queue_size, const std::shared_ptr<ThreadPool> &tp) {

    SubSerializerT serializer;
    static_assert(std::is_base_of<Serializer<SubSerializerT>, SubSerializerT>::value);

    return std::make_unique<Subscriber>(
        NodeBase::GlobalNode(true)->addSubscriber(
            topic, tp, queue_size,
            [serializer, callback_f](const std::shared_ptr<const void>& msg) mutable {
                callback_f(serializer.template interProcTypeConvert<MsgT>(msg));
            },
            [serializer, callback_f](const std::shared_ptr<const void>& msg) mutable {
                callback_f(serializer.template intraProcTypeConvert<MsgT>(msg));
            },
            [serializer](std::shared_ptr<const Message> &msg, const std::string& protocol) mutable {
                return serializer.template deserialize<MsgT>(msg, protocol);
            },
            SubSerializerT::Protocol()
        )
    );
}

template<class MsgT, class PubSerializerT>
std::unique_ptr<Publisher<MsgT>> CreatePublisher(const std::string &topic) {
    PubSerializerT serializer;
    static_assert(std::is_base_of<Serializer<PubSerializerT>, PubSerializerT>::value);

    return std::make_unique<Publisher<MsgT>>(
            NodeBase::GlobalNode(true)->addPublisher(topic, PubSerializerT::Protocol()),
            [serializer = std::move(serializer)](const std::shared_ptr<const MsgT>&obj) mutable {
                return serializer.template serialize<MsgT>(obj);}
    );
}

template<class ReqT, class RepT, class ReqSerializerT, class RepSerializerT>
std::unique_ptr<Client<ReqT, RepT>> CreateClient(const std::string &srv_name) {
    using namespace std;
    ReqSerializerT req_serializer;
    RepSerializerT rep_serializer;
    static_assert(std::is_base_of<Serializer<ReqSerializerT>, ReqSerializerT>::value);
    static_assert(std::is_base_of<Serializer<RepSerializerT>, RepSerializerT>::value);

    return std::make_unique<Client<ReqT,RepT>>(
            NodeBase::GlobalNode(true)->addClient(srv_name, req_serializer.Protocol()),
            [req_serializer = move(req_serializer)](const shared_ptr<const ReqT>&req){
                return req_serializer.template serialize<ReqT>(req);
            },
            [rep_serializer = move(rep_serializer)]
            (shared_ptr<const Message>&rep_msg, const string& protocol){
                return rep_serializer.template interProcTypeConvert<RepT>(
                        rep_serializer.template deserialize<RepT>(rep_msg, protocol));
            },
            [rep_serializer = move(rep_serializer)](const shared_ptr<const void>& rep_obj){
                return rep_serializer.template intraProcTypeConvert<RepT>(rep_obj);
            }
    );
}

template<class ReqT, class RepT, class ReqSerializerT, class RepSerializerT>
std::unique_ptr<Server> CreateServer(const std::string &srv_name,
                                     const SrvFunc<ReqT, RepT> &callback_f,
                                     size_t queue_size,
                                     const std::shared_ptr<ThreadPool> &tp) {
    using namespace std;
    ReqSerializerT req_serializer;
    RepSerializerT rep_serializer;
    static_assert(std::is_base_of<Serializer<ReqSerializerT>, ReqSerializerT>::value);
    static_assert(std::is_base_of<Serializer<RepSerializerT>, RepSerializerT>::value);

    return std::make_unique<Server>(
        NodeBase::GlobalNode(true)->addServer(
            srv_name, tp, queue_size,
            [req_serializer, rep_serializer, callback_f]
            (shared_ptr<const Message>& request, const string& req_protocol, shared_ptr<Message>& response){
                return SrvInterProcCallback(req_serializer,
                                            rep_serializer,
                                            callback_f,
                                            request,
                                            req_protocol,
                                            response);},
            [req_serializer, callback_f](shared_ptr<const void>& request, shared_ptr<void>& response){
                return SrvIntraProcCallback(req_serializer, callback_f, request, response);},
            ReqSerializerT::Protocol(),
            RepSerializerT::Protocol()
        )
    );
}

}
#endif //RUM_CPPRUM_RUM_H_
