//
// Created by Ivan B on 2021/4/8.
//

#include "publisher_base.h"
#include "internal/publisher_base_impl.h"

namespace rum {

PublisherBase::PublisherBase(std::unique_ptr<PublisherBaseImpl> pimpl):
        pimpl_(move(pimpl)){}

PublisherBase::PublisherBase(PublisherBase &&rhs) noexcept {
    pimpl_ = move(rhs.pimpl_);
}

PublisherBase &PublisherBase::operator=(PublisherBase &&rhs) noexcept {
    pimpl_ = move(rhs.pimpl_);
    return *this;
}

PublisherBase::~PublisherBase() = default;

bool PublisherBase::isConnected() {
    return pimpl_->isConnected();
}

bool PublisherBase::publish(zmq::message_t &body) {
    return pimpl_->publishIpc(body);
}

bool PublisherBase::scheduleItc(const std::shared_ptr<const void> &msg) {
    return pimpl_->scheduleItc(msg);
}


}
