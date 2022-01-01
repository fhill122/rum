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

constexpr char kTopicReserve[] = "__";
constexpr char kSyncTopic[] = "__sync";

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
