/*
 * Created by Ivan B on 2021/12/26.
 */

#ifndef RUM_CORE_INTERNAL_REMOTE_MANAGER_H_
#define RUM_CORE_INTERNAL_REMOTE_MANAGER_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>

#include "rum/common/def.h"
#include "rum/extern/ivtb/stopwatch.h"
#include "../msg/rum_sync_generated.h"

namespace rum{


struct RemoteManager {

    struct NodeUpdate{
        std::vector<std::string> topics_new;
        std::vector<std::string> topics_removed;
    };


    struct NodeInfo{
        std::string sync_data;
        // std::string str_id;
        int offline_check_count = 0;
        ivtb::Stopwatch last_observation;

        // NodeInfo();
        NodeInfo(const std::string &sync_fb);

        bool isDisconnected();

        [[nodiscard]] inline const msg::SyncBroadcast* getSyncFb() const {
            return msg::GetSyncBroadcast(sync_data.data());
        };

        [[nodiscard]] static std::string GetStrId(const void *sync_fb_p);

        // refresh offline check statistics
        void observe();

        bool shouldRemove();
    };

    unsigned long sync_count = 0;
    // <StrId, NodeInfo>
    std::unordered_map<std::string, std::unique_ptr<NodeInfo>> remote_book;
    // <topic, NodeInfo>
    std::unordered_map<std::string, std::vector<NodeInfo*>> topic_book;

    static RemoteManager& GlobalManager();

    NodeUpdate wholeSyncUpdate(const void *fb_data, size_t size); RUM_THREAD_UNSAFE

    // this should be called in the same thread as update() to prevent sync
    std::vector<std::string> checkAndRemove(); RUM_THREAD_UNSAFE
};

}

#endif //RUM_CORE_INTERNAL_REMOTE_MANAGER_H_
