//
// Created by Ivan B on 2021/4/8.
//

#ifndef RUM_RUM_PUBLISHER_H_
#define RUM_RUM_PUBLISHER_H_

#include <rum/core/publisher_base_handler.h>
#include <rum/serialization/serializer.h>
#include <memory>
#include <string>

namespace rum {

template <class MsgT, class SerializerT>
class PublisherHandler : public PublisherBaseHandler{
  private:
  public:
    inline static Serializer<SerializerT> s_{};

  private:
    void internalPub(const std::shared_ptr<const MsgT> &msg){
        // pub itc
        bool scheduled = scheduleItc(msg);

        // pub ipc
        if (isConnected()){
            auto message = s_.serialize(msg);
            if(message) PublisherBaseHandler::pub(*message);
        }
    }

  public:
    explicit PublisherHandler(PublisherBaseHandler &&base) : PublisherBaseHandler(std::move(base)){}
    PublisherHandler() : PublisherBaseHandler(nullptr){};

    bool hasSubscribers();

    // taking ownership, no copy
    void pub(std::unique_ptr<MsgT> msg){
        internalPub(move(msg));
    }

    // taking shared ownership, copy
    void pub(const std::shared_ptr<MsgT> &msg){
        auto msg_ptr = std::make_shared<MsgT>(*msg);
        internalPub(msg_ptr);
    }

    // taking a const shared_ptr, no copy
    void pub(const std::shared_ptr<const MsgT> &msg){
        internalPub(msg);
    }

    // taking r value ref, no copy
    void pub(MsgT&& msg){
        auto msg_ptr = std::make_shared<MsgT>(std::move(msg));
        internalPub(msg_ptr);
    }

    // taking l value ref, copy
    void pub(const MsgT &msg){
        auto msg_ptr = std::make_shared<MsgT>(msg);
        internalPub(msg_ptr);
    }

};

}

#endif //RUM_RUM_PUBLISHER_H_
