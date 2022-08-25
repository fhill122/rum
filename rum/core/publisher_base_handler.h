//
// Created by Ivan B on 2021/4/8.
//

#ifndef RUM_CORE_PUBLISHER_BASE_HANDLER_H_
#define RUM_CORE_PUBLISHER_BASE_HANDLER_H_

#include <memory>

#include "rum/common/message.h"

namespace rum {

class PublisherBaseImpl;
class NodeBase;

class PublisherBaseHandler {
  private:
    friend NodeBase;
    PublisherBaseImpl* pimpl_ = nullptr;
    inline PublisherBaseHandler() = default;

  protected:
    explicit PublisherBaseHandler(PublisherBaseImpl* pimpl);

  public:
    inline virtual ~PublisherBaseHandler() = default;

    /**
     * @return Whether has any subscriber (inter or intra process) connected to it
     */
    bool hasSubscribers() const;

    /**
     * @return whether is remotely (inter proc) connected with any subscriber
     */
    bool isConnected() const;

    const std::string & getTopic() const;

    bool pub(Message &message);

    bool scheduleIntraProc(const std::shared_ptr<const void> &msg);
};

}

#endif //RUM_CORE_PUBLISHER_BASE_HANDLER_H_
