//
// Created by Ivan B on 2021/4/8.
//

#include "publisher_base_handler.h"
#include "internal/publisher_base_impl.h"
#include "internal/itc_manager.h"

namespace rum {

PublisherBaseHandler::PublisherBaseHandler(PublisherBaseImpl* pimpl): pimpl_(pimpl){}


bool PublisherBaseHandler::isConnected() {
    return pimpl_->isConnected();
}

bool PublisherBaseHandler::pub(Message &body) {
    return pimpl_->publishIpc(body);
}

bool PublisherBaseHandler::scheduleItc(const std::shared_ptr<const void> &msg) {
    return pimpl_->scheduleItc(msg);
}

}
