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
    static inline std::unordered_map<unsigned int, AwaitingResult*> wait_list_{};  RUM_LOCK_BY(wait_list_mu_)
    static inline std::mutex wait_list_mu_{};

    const unsigned int id;
    std::shared_ptr<const void> request = nullptr;
    // pair<shared_ptr<Message>,protocol string> for inter-proc case
    std::shared_ptr<void> response = nullptr;
    SrvStatus status = SrvStatus::OK;
    // todo ivan. RemoteManager like heartbeat, so we could stop waiting smartly
    mutable std::mutex mu;
    mutable std::condition_variable cv;

  private:
    explicit AwaitingResult(unsigned int id) : id(id){}

  public:
    ~AwaitingResult(){
        if (id==0) return;
        std::lock_guard lock(wait_list_mu_);
        wait_list_.erase(id);
    }

    inline static std::unique_ptr<AwaitingResult> CreateInterP(){
        std::unique_ptr<AwaitingResult> awaiting_result;
        std::lock_guard lock(wait_list_mu_);
        while(true){
            auto id = id_pool.fetch_add(1);
            if (id==0) continue;
            auto itr = wait_list_.find(id);
            if (itr==wait_list_.end()){
                awaiting_result = std::unique_ptr<AwaitingResult>(new AwaitingResult(id));
                wait_list_.emplace(awaiting_result->id, awaiting_result.get());
                return awaiting_result;
            }
        }
    }

    inline static std::unique_ptr<AwaitingResult> CreateIntraP(){
        return std::unique_ptr<AwaitingResult>(new AwaitingResult(0));
    }
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

inline std::string_view SrvFromReqTopic(const std::string &req_topic){
    return std::string_view(req_topic).substr(strlen(kSrvReqTopicPrefix));
}

inline std::string_view SrvFromRepTopic(const char *rep_topic){
    std::string_view out(rep_topic);
    auto pos = out.find('_', strlen(kSrvRepTopicPrefix));
    AssertLog(pos!=std::string_view::npos, "");
    out.remove_prefix(pos+1);
    return out;
}
inline std::string_view SrvFromRepTopic(const std::string &rep_topic){ return SrvFromRepTopic(rep_topic.c_str());}

inline std::string_view IdFromRepTopic(const std::string &rep_topic){
    std::string_view out(rep_topic);
    auto pos = out.find('_', strlen(kSrvRepTopicPrefix));
    AssertLog(pos!=std::string_view::npos, rep_topic);
    out.remove_prefix(strlen(kSrvRepTopicPrefix));
    out.remove_suffix(rep_topic.size()-pos-1);
    return out;
}

inline std::string StrippedRepTopic(const std::string &rep_topic){
    std::string out(kSrvRepTopicPrefix);
    out.append(SrvFromRepTopic(rep_topic));
    return out;
}

}
#endif //RUM_CORE_INTERNAL_SRV_COMMON_H_
