//
// Created by Ivan B on 2021/4/16.
//

// #include "subscriber_base.h"
//
// #include "internal/subscriber_base_impl.h"
//
// namespace rum {
//
// SubscriberBase::SubscriberBase(SubscriberBaseImpl *pimpl) :
//         pimpl_(pimpl) {}
//
// // SubscriberBase::SubscriberBase(SubscriberBase &&rhs) noexcept {
// //     pimpl_ = move(rhs.pimpl_);
// // }
// //
// // SubscriberBase& SubscriberBase::operator=(SubscriberBase &&rhs) noexcept {
// //     pimpl_ = move(rhs.pimpl_);
// //     return *this;
// // }
//
// SubscriberBase::~SubscriberBase() = default;
//
// }

#include "subscriber_base_handler.h"

namespace rum{

SubscriberBaseHandler::SubscriberBaseHandler(SubscriberBaseImpl *pimpl): pimpl_(pimpl) {}

}
