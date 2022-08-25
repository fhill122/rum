//
// Created by Ivan B on 2021/4/8.
//

#ifndef RUM_RUM_PUBLISHER_H_
#define RUM_RUM_PUBLISHER_H_

#include "rum/core/publisher_base_handler.h"
#include "rum/core/node_base.h"
#include "rum/common/log.h"
#include "rum/common/serialization.h"
#include <memory>
#include <string>

namespace rum {

template <class MsgT>
class Publisher : public PublisherBaseHandler{
  public:
    using SharedPtr = std::shared_ptr<Publisher<MsgT>>;
    using UniquePtr = std::unique_ptr<Publisher<MsgT>>;

  private:
    SerFunc<MsgT> ser_func_;

  private:
    void internalPub(const std::shared_ptr<const MsgT> &msg){
        // pub intra-proc
        bool scheduled = scheduleIntraProc(msg);

        // pub inter-proc
        if (isConnected()){
            auto message = ser_func_(msg);
            if(message){
                PublisherBaseHandler::pub(*message);
            } else{
                log.e(getTopic(), "Failed to serialize");
            }
        }
    }

  public:
    Publisher(PublisherBaseHandler &&base, SerFunc<MsgT> ser_func) :
            PublisherBaseHandler(base), ser_func_(std::move(ser_func)){}
    Publisher() : PublisherBaseHandler(nullptr){};
    Publisher(const Publisher&) = delete;
    Publisher& operator=(const Publisher&) = delete;
    Publisher(Publisher &&other) = delete;
    Publisher& operator=(Publisher &&other) = delete;

    ~Publisher() override{
        NodeBase::GlobalNode()->removePublisher(*this);
    }

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

    // taking r value ref, move instead of copy
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
