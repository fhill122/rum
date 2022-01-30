//
// Created by Ivan B on 2021/3/16.
//

#ifndef RUM_COMMON_COMMON_H_
#define RUM_COMMON_COMMON_H_

#include <string>

namespace rum{

constexpr int kMaxQueueSize = 1000;
constexpr int kDefMasterInPort = 12580;
// todo ivan. implement this as a dynamic port that master gives
constexpr int kDefMasterOutPort = 12581;
constexpr int kMasterTrialPeriod = 100;  // ms
constexpr int kNodeHbPeriod = 200; //ms
// constexpr int kNodeHbPeriod = 0; //ms

/* node removal */
// longer than this is considered offline
constexpr int kNodeOfflineCriteria = 2*kNodeHbPeriod;
// offline checking period
constexpr int kNodeOfflineCheckPeriod = 500;  // ms
// node is removed if checked offline this number of times,
// this instead of a single timeout to ensure node will not be deleted in case of system change, system suspend, etc.
constexpr int kNodeOfflineCheckCounts = 2;

// reserved topic prefix (including srv name as it uses topic internally)
constexpr char kTopicReserve[] = "__";
// topic for synchronization
constexpr char kSyncTopic[] = "__sync";
// service request pub sub is prefixed with this
constexpr char kSrvReqTopicPrefix[] = "__q_";
// service response pub sub is prefixes with this + node_string + "_"
constexpr char kSrvRepTopicPrefix[] = "__p_";

constexpr char kIpEnv[] = "RUM_IP";
constexpr char kMasterPortInEnv[] = "RUM_MASTER_IN";
constexpr char kMasterPortOutEnv[] = "RUM_MASTER_OUT";

extern const std::string kIpStr;
extern const char kIp[4];
extern const pid_t kPid;

std::string GetMasterInAddr();
std::string GetMasterOutAddr();

}

#endif //RUM_COMMON_COMMON_H_
