//
// Created by Ivan B on 2021/8/6.
//

#ifndef RUM_CORE_INTERNAL_INTRA_PROC_MANAGER_H_
#define RUM_CORE_INTERNAL_INTRA_PROC_MANAGER_H_


#include <unordered_map>
#include <string>
#include <mutex>
#include <vector>
#include <memory>

#include <rum/common/def.h>

namespace rum {

class SubscriberBaseImpl;

// note: we assume all connect to same master here
struct InatraProcManager {

    std::string domain;
    std::mutex mu;
    std::unordered_map<std::string, std::vector<SubscriberBaseImpl*>> subs; RUM_LOCK_BY(mu)

  public:
    static std::shared_ptr<InatraProcManager>& GlobalManager();

    void addSub(SubscriberBaseImpl* sub); RUM_THREAD_SAFE
    void removeSub(SubscriberBaseImpl* sub); RUM_THREAD_SAFE
    void batchRemove(const std::vector<SubscriberBaseImpl*> &subs_to_remove); RUM_THREAD_SAFE;

    bool scheduleMessage(const std::string &topic, const std::shared_ptr<const void> &msg); RUM_THREAD_SAFE

    bool haveSub(const std::string &topic); RUM_THREAD_SAFE

    // on pub creation

};

}

#endif //RUM_CORE_INTERNAL_INTRA_PROC_MANAGER_H_
