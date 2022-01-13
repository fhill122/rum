//
// Created by Ivan B on 2021/8/6.
//

#include "itc_manager.h"
#include "subscriber_base_impl.h"
#include "rum/common/log.h"
#include "rum/common/misc.h"


using namespace std;

namespace rum{

ItcManager &ItcManager::GlobalManager(){
    static ItcManager manager;
    return manager;
}

void ItcManager::addSub(SubscriberBaseImpl *sub) {
    lock_guard lock(mu);
    auto topic = sub->topic_;
    MapVecAdd<string, SubscriberBaseImpl*>(subs, topic, move(sub));
}

void ItcManager::removeSub(SubscriberBaseImpl* sub) {
    lock_guard lock(mu);
    MapVecRemove(subs, sub->topic_, sub,
                 [sub](SubscriberBaseImpl* s){return s==sub;});
}

void ItcManager::batchRemove(const vector<SubscriberBaseImpl*> &subs_to_remove) {
    lock_guard lock(mu);
    for (auto* sub : subs_to_remove){
        MapVecRemove(subs, sub->topic_, sub,
                     [sub](SubscriberBaseImpl* s){return s==sub;});
    }
}

bool ItcManager::scheduleItc(const string &topic, const shared_ptr<const void> &msg) {
    lock_guard lock(mu);
    auto itr = subs.find(topic);
    if (itr==subs.end())
        return false;

    auto sub_msg = make_shared<SubscriberBaseImpl::Msg>(msg, false, true);
    for (auto *sub : itr->second){
        sub->enqueue(sub_msg);
    }
    return true;
}

}
