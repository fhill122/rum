/*
 * Created by Ivan B on 2021/12/26.
 */

#include "remote_manager.h"

#include "rum/common/log.h"
#include "rum/common/misc.h"
#include "rum/common/common.h"
#include "node_id.h"
#include "srv_common.h"

#define TAG "RemoteManager"

using namespace std;
using namespace flatbuffers;

namespace rum {

std::string RemoteManager::NodeInfo::GetStrId(const void *sync_fb_p) {
    const auto *sync = (msg::SyncBroadcast *) sync_fb_p;
    return GetNodeStrId(sync->node()->pid(), sync->node()->tcp_addr()->str());
}

RemoteManager::NodeInfo::NodeInfo(const string &sync_fb) : sync_data(sync_fb) {
    // str_id = GetStrId(msg::GetSyncBroadcast(sync_fb.data()));
}

void RemoteManager::NodeInfo::observe() {
    last_observation.start();
    offline_check_count = 0;
}

bool RemoteManager::NodeInfo::shouldRemove() {
    if (last_observation.passedMs() < kNodeOfflineCriteria) return false;
    bool res =  (++offline_check_count) >= kNodeOfflineCheckCounts;
    // if (res) log.e(TAG, "remove a node");
    return res;
}

std::shared_ptr<RemoteManager> &RemoteManager::GlobalManager() {
    static shared_ptr<RemoteManager> manager = make_shared<RemoteManager>();
    return manager;
}

RemoteManager::NodeUpdate RemoteManager::wholeSyncUpdate(const void *fb_data, size_t size) {
    auto findDifference = [](
            const flatbuffers::Vector<Offset<msg::SubscriberInfo>> *old_fb_subs,
            const flatbuffers::Vector<Offset<msg::SubscriberInfo>> *new_fb_subs,
            vector<string> &topics_new,
            vector<string> &topics_removed){
        set<std::string> old_subs;
        set<std::string> new_subs;

        for (auto sub: *(old_fb_subs)) {
            old_subs.insert(sub->topic()->str());
        }
        for (auto sub: *(new_fb_subs)) {
            new_subs.insert(sub->topic()->str());
        }

        topics_new.resize(new_subs.size());
        auto it_added = set_difference(new_subs.begin(), new_subs.end(),
                                       old_subs.begin(), old_subs.end(),
                                       topics_new.begin());
        topics_new.resize(it_added - topics_new.begin());

        topics_removed.resize(old_subs.size());
        auto it_removed = set_difference(old_subs.begin(), old_subs.end(),
                                         new_subs.begin(), new_subs.end(),
                                         topics_removed.begin());
        topics_removed.resize(it_removed - topics_removed.begin());
    };

    NodeUpdate update;
    const auto *sync = msg::GetSyncBroadcast(fb_data);
    AssertLog(sync->type() == msg::SyncType_Whole, "");

    auto node_str = NodeInfo::GetStrId(sync);
    auto itr = remote_book.find(node_str);
    NodeInfo *node_p;
    // new node
    if (itr == remote_book.end()) {
        auto new_node = make_unique<NodeInfo>(string((char *) fb_data, size));
        node_p = new_node.get();
        remote_book[node_str] = move(new_node);
        update.topics_new.reserve(sync->subscribers()->size());
        update.rep_topics_new.reserve(sync->clients()->size());
        // uniqueness is guaranteed
        for (const auto *sub: *sync->subscribers()) {
            update.topics_new.push_back(sub->topic()->str());
        }
        for (const auto *cli: *sync->clients()) {
            update.rep_topics_new.push_back(cli->topic()->str());
        }
    }
    // exist node
    else {
        node_p = itr->second.get();
        node_p->observe();
        const auto *old_sync = itr->second->getSyncFb();
        if (old_sync->version() == sync->version()) return update;

        findDifference(old_sync->subscribers(), sync->subscribers(),
                       update.topics_new, update.topics_removed);
        findDifference(old_sync->clients(), sync->clients(),
                       update.rep_topics_new, update.rep_topics_removed);

        itr->second->sync_data = string((char *) fb_data, size);
    }

    // update sub_book and cli_book
    for (const auto &t: update.topics_new) {
        MapVecAdd(sub_book, t, (NodeInfo *)node_p);
    }
    for (const auto &t: update.topics_removed) {
        MapVecRemove(sub_book, t, node_p,
                     [node_p](NodeInfo *n) { return n == node_p; });
    }
    for (const auto &t: update.rep_topics_new) {
        MapVecAdd(sub_book, t, (NodeInfo *)node_p);
    }
    for (const auto &t: update.rep_topics_removed) {
        MapVecRemove(sub_book, t, node_p,
                     [node_p](NodeInfo *n) { return n == node_p; });
    }

    return update;
}

std::vector<std::string> RemoteManager::checkAndRemove() {
    vector<string> removed_topics;

    // get the removal list
    // <string id, NodeInfo>
    vector<pair<string, std::unique_ptr<NodeInfo>>> to_remove;
    for (auto &id_node_pair : remote_book){
        auto &node = id_node_pair.second;
        if (node->shouldRemove())
            to_remove.emplace_back(id_node_pair.first, move(node));
    }

    // remove from topic book
    for (auto &id_node_pair : to_remove){
        const auto *sync = id_node_pair.second->getSyncFb();
        auto *node_p = id_node_pair.second.get();
        for (auto *sub : *sync->subscribers()){
            MapVecRemove(sub_book, sub->topic()->str(), node_p,
                         [node_p](NodeInfo *n){return n==node_p;} );
        }
        for (auto *cli : *sync->clients()){
            MapVecRemove(sub_book, cli->topic()->str(), node_p,
                         [node_p](NodeInfo *n){return n==node_p;} );
        }
    }

    // fill return. note order matters, sync_data is moved here
    vector<string> removed_node_sync;
    removed_node_sync.reserve(to_remove.size());
    for (auto &id_node_pair : to_remove){
        removed_node_sync.push_back(move(id_node_pair.second->sync_data));
    }

    // remove from remote book
    for (auto &id_node_pair : to_remove) {
        remote_book.erase(id_node_pair.first);
    }

    return removed_node_sync;
}

}

#undef TAG