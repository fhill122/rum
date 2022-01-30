/*
 * Created by Ivan B on 2022/1/25.
 */

#ifndef RUM_CORE_INTERNAL_AWAITING_RESULT_H_
#define RUM_CORE_INTERNAL_AWAITING_RESULT_H_

#include <unordered_map>
#include <mutex>

#include "rum/common/srv_def.h"

namespace rum{

struct AwaitingResult{
    static inline std::atomic<unsigned int> id_pool{1};

    const unsigned int id;
    std::shared_ptr<const void> request = nullptr;
    std::shared_ptr<void> response = nullptr;
    SrvStatus status = SrvStatus::OK;
    // todo ivan. RemoteManager like heartbeat, so we could stop waiting smartly
    mutable std::mutex mu;
    mutable std::condition_variable cv;

    AwaitingResult(): id(id_pool.fetch_add(1, std::memory_order_relaxed)) {}
    explicit AwaitingResult(unsigned int id) : id(id){}
};

}
#endif //RUM_CORE_INTERNAL_AWAITING_RESULT_H_
