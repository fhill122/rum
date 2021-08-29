//
// Created by Ivan B on 2021/4/16.
//

#include "subscriber_base.h"

#include "internal/subscriber_base_impl.h"

namespace rum {

SubscriberBase::SubscriberBase(std::unique_ptr<SubscriberBaseImpl> pimpl) :
        pimpl_(move(pimpl)) {}

SubscriberBase::SubscriberBase(SubscriberBase &&rhs) noexcept {
    pimpl_ = move(rhs.pimpl_);
}

SubscriberBase& SubscriberBase::operator=(SubscriberBase &&rhs) noexcept {
    pimpl_ = move(rhs.pimpl_);
    return *this;
}

SubscriberBase::~SubscriberBase() = default;

}