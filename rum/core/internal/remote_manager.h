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
        std::vector<std::string> rep_topics_new;
        std::vector<std::string> rep_topics_removed;

        inline std::string toString(){
            std::string out;
            for (const auto& t : topics_new){
                out += "+" + t + " ";
            }
            for (const auto& t : topics_removed){
                out += "-" + t + " ";
            }
            for (const auto& t : rep_topics_new){
                out += "+" + t + " ";
            }
            for (const auto& t : rep_topics_removed){
                out += "-" + t + " ";
            }
            return out;
        }

        inline bool empty(){
            return topics_new.empty() && topics_removed.empty() &&
                    rep_topics_new.empty() && rep_topics_removed.empty();
        }
    };


    struct NodeInfo{
        std::string sync_data;
        // std::string str_id;
        int offline_check_count = 0;
        ivtb::Stopwatch last_observation;

        // NodeInfo();
        NodeInfo(const std::string &sync_fb);

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
    // <topic, NodeInfo> sub topic includes normal topic, req topic, rep topic
    std::unordered_map<std::string, std::vector<NodeInfo*>> sub_book;
    // <srv name, NodeInfo> exclusive subs' info, rep topics
    std::unordered_map<std::string, std::vector<NodeInfo*>> exclusive_sub_book;

    static std::shared_ptr<RemoteManager>& GlobalManager();

    NodeUpdate wholeSyncUpdate(const void *fb_data, size_t size); RUM_THREAD_UNSAFE

    // this should be called in the same thread as update() to prevent sync
    std::vector<std::string> checkAndRemove(); RUM_THREAD_UNSAFE
};

}

#endif //RUM_CORE_INTERNAL_REMOTE_MANAGER_H_
