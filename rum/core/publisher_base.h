//
// Created by Ivan B on 2021/4/8.
//

#ifndef RUM_CORE_PUBLISHER_BASE_H_
#define RUM_CORE_PUBLISHER_BASE_H_

#include <memory>
#include <rum/extern/zmq/zmq.hpp>

namespace rum {

class PublisherBaseImpl;

class PublisherBase {
  private:
    std::unique_ptr<PublisherBaseImpl> pimpl_;
  public:

  protected:
  public:
    explicit PublisherBase(std::unique_ptr<PublisherBaseImpl> pimpl);
    PublisherBase(PublisherBase&& rhs) noexcept;
    PublisherBase& operator=(PublisherBase&& rhs) noexcept;
    PublisherBase(const PublisherBase&) = delete;
    PublisherBase& operator=(const PublisherBase&&) = delete;
    virtual ~PublisherBase();

    bool isConnected();
    bool publish(zmq::message_t &body);
    bool scheduleItc(const std::shared_ptr<const void> &msg);
};

}

#endif //RUM_CORE_PUBLISHER_BASE_H_
