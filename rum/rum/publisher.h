//
// Created by Ivan B on 2021/4/8.
//

#ifndef RUM_RUM_PUBLISHER_H_
#define RUM_RUM_PUBLISHER_H_

#include <rum/core/publisher_base.h>
#include <rum/serialization/serializer.h>
#include <memory>
#include <string>

namespace rum {

template <class SerializerT, class MsgT>
class Publisher : public PublisherBase{
  private:
    Serializer<SerializerT> s_;
  public:

  public:
    explicit Publisher(PublisherBase &&base) : PublisherBase(std::move(base)){}


    void pub(std::unique_ptr<MsgT> msg){
        std::shared_ptr<const MsgT> msg_sptr = move(msg);
        // pub itc
        scheduleItc(msg_sptr);

        // pub ipc
        if (isConnected()){
            auto zmq_msg = s_.serialize(msg_sptr);
            publish(*zmq_msg);
        }
    }

};

}

#endif //RUM_RUM_PUBLISHER_H_
