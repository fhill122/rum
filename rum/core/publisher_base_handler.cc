//
// Created by Ivan B on 2021/4/8.
//

#include "publisher_base_handler.h"
#include "internal/publisher_base_impl.h"
#include "internal/intra_proc_manager.h"

namespace rum {

PublisherBaseHandler::PublisherBaseHandler(PublisherBaseImpl* pimpl): pimpl_(pimpl){}


bool PublisherBaseHandler::isConnected() const{
    return pimpl_->isConnected();
}

const std::string &PublisherBaseHandler::getTopic() const {
    return pimpl_->topic_;
}


bool PublisherBaseHandler::pub(Message &body) {
    return pimpl_->publish(body);
}

bool PublisherBaseHandler::scheduleIntraProc(const std::shared_ptr<const void> &msg) {
    return pimpl_->scheduleIntraProc(msg);
}

}
