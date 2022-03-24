/*
 * Created by Ivan B on 2022/1/25.
 */

#ifndef RUM_CORE_INTERNAL_SRV_COMMON_H_
#define RUM_CORE_INTERNAL_SRV_COMMON_H_

#include <unordered_map>
#include <mutex>
#include <condition_variable>

#include "rum/common/srv_def.h"
#include "rum/common/common.h"
#include "rum/common/log.h"
#include "rum/common/misc.h"

namespace rum{

struct AwaitingResult{
    static inline std::atomic<unsigned int> id_pool{1};

    const unsigned int id;
    std::shared_ptr<const void> request = nullptr;
    // pair<shared_ptr<Message>,protocol string> for ipc case
    std::shared_ptr<void> response = nullptr;
    SrvStatus status = SrvStatus::OK;
    // todo ivan. RemoteManager like heartbeat, so we could stop waiting smartly
    mutable std::mutex mu;
    mutable std::condition_variable cv;

    // AwaitingResult(): id(id_pool.fetch_add(1, std::memory_order_relaxed)) {}
    explicit AwaitingResult(unsigned int id) : id(id){}
};

inline std::string GetRepTopic(const std::string &srv_name, const std::string &node_str){
    static std::atomic<unsigned int> n{0};
    return std::string(kSrvRepTopicPrefix).append(std::to_string(n.fetch_add(1, std::memory_order_relaxed))).
            append("@").append(node_str).append("_").append(srv_name);
}

inline std::string GetReqTopic(const std::string &srv_name){
    return kSrvReqTopicPrefix + srv_name;
}

inline bool IsRepTopic(const char* topic){
    return StrStartWith(topic, kSrvRepTopicPrefix);
}

inline std::string_view SrvFromRepTopic(const std::string &rep_topic){
    std::string_view out(rep_topic);
    auto pos = out.find('_', strlen(kSrvRepTopicPrefix));
    AssertLog(pos!=std::string_view::npos, "");
    out.remove_prefix(pos+1);
    return out;
}

inline std::string_view IdFromRepTopic(const std::string &rep_topic){
    std::string_view out(rep_topic);
    auto pos = out.find('_', strlen(kSrvRepTopicPrefix));
    AssertLog(pos!=std::string_view::npos, rep_topic);
    out.remove_prefix(strlen(kSrvRepTopicPrefix));
    out.remove_suffix(rep_topic.size()-pos-1);
    return out;
}

}
#endif //RUM_CORE_INTERNAL_SRV_COMMON_H_
