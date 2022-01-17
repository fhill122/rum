/*
 * Created by Ivan B on 2021/12/26.
 */

#include "remote_manager.h"

#include "rum/common/log.h"
#include "rum/common/misc.h"

using namespace std;

namespace rum {

bool RemoteManager::NodeInfo::isDisconnected() {
    return false;
}

std::string RemoteManager::NodeInfo::GetStrId(const void *sync_fb_p) {
    const auto *sync = (msg::SyncBroadcast *) sync_fb_p;
    return to_string(sync->node()->pid()) + "::" + sync->node()->tcp_addr()->str();
}

RemoteManager::NodeInfo::NodeInfo(const string &sync_fb) : sync_data(sync_fb) {
    str_id = GetStrId(msg::GetSyncBroadcast(sync_fb.data()));
}

RemoteManager::NodeInfo::NodeInfo() = default;

RemoteManager &RemoteManager::GlobalManager() {
    static RemoteManager manager;
    return manager;
}

RemoteManager::NodeUpdate RemoteManager::wholeSyncUpdate(const void *fb_data, size_t size) {
    NodeUpdate update;
    const auto *sync = msg::GetSyncBroadcast(fb_data);
    AssertLog(sync->type() == msg::SyncType_Whole, "");

    auto node_str = NodeInfo::GetStrId(sync);
    auto itr = remote_book.find(node_str);
    NodeInfo *node_p;
    if (itr == remote_book.end()) {
        auto new_node = make_unique<NodeInfo>(string((char *) fb_data, size));
        node_p = new_node.get();
        remote_book[node_str] = move(new_node);
        update.topics_new.reserve(sync->subscribers()->size());
        // uniqueness is guaranteed
        for (const auto *sub: *sync->subscribers()) {
            update.topics_new.push_back(sub->topic()->str());
        }
    } else {
        node_p = itr->second.get();
        const auto *old_sync = itr->second->getSyncFb();
        if (old_sync->version() == sync->version()) return update;

        set<std::string> old_subs;
        set<std::string> new_subs;

        for (auto sub: *old_sync->subscribers()) {
            old_subs.insert(sub->topic()->str());
        }
        for (auto sub: *sync->subscribers()) {
            new_subs.insert(sub->topic()->str());
        }

        update.topics_new.resize(new_subs.size());
        auto it_added = set_difference(new_subs.begin(), new_subs.end(),
                                       old_subs.begin(), old_subs.end(),
                                       update.topics_new.begin());
        update.topics_new.resize(it_added - update.topics_new.begin());

        update.topics_removed.resize(old_subs.size());
        auto it_removed = set_difference(old_subs.begin(), old_subs.end(),
                                         new_subs.begin(), new_subs.end(),
                                         update.topics_removed.begin());
        update.topics_removed.resize(it_removed - update.topics_removed.begin());

        itr->second->sync_data = string((char *) fb_data, size);
    }

    // update topic book
    for (const auto &t: update.topics_new) {
        MapVecAdd(topic_book, t, (NodeInfo *) {node_p});
    }
    for (const auto &t: update.topics_removed) {
        MapVecRemove(topic_book, t, node_p,
                     [node_p](NodeInfo *n) { return n == node_p; });
    }
    return update;
}

}