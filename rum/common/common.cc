//
// Created by Ivan B on 2021/3/18.
//

#include "common.h"

#include <unistd.h>

#include <rum/common/zmq_helper.h>
#include "log.h"

using namespace std;

namespace rum {

string GetRumIp();

const string kIpStr = GetRumIp();
const pid_t kPid = getpid();

string GetRumIp() {
    if (const char *env_ip = std::getenv(kIpEnv)) {
        printer.i("rum", "Ip from $%s: %s", kIpEnv, env_ip);
        return env_ip;
    }

    auto interfaces = GetNetInterfaces();
    string ip_str = GuessIp(interfaces);

    // assign to 127.0.0.1 if possible
    if (ip_str.empty()){
        for (const auto &i : interfaces) {
            if (i.ip == "127.0.0.1") {
                ip_str = i.ip;
                log.w("rum", "Local mode, ip = %s", ip_str.c_str());
                break;
            }
        }
    }

    // failed to find an ip. printing debug info
    if (ip_str.empty()) {
        string net_str;
        int n = 0;
        for (const auto &i : interfaces) {
            if (!i.ip.empty()) {
                net_str += "\n\t" + i.name + "\t" + i.ip;
                ++n;
            }
        }

        printer.e("rum", "Failed to guess ip_str address from %d interfaces:%s",
                  n, net_str.c_str());
        exit(1);
    }

    printer.i("rum", "Rum started on %d@%s", getpid(), ip_str.c_str());
    return ip_str;
}

std::string GetMasterInAddr() {
    string master_port;
    if (const char *env_master_port = std::getenv(kMasterPortInEnv)) {
        printer.i("rum", "master inbound port from $%s: %s", kMasterPortInEnv, env_master_port);
        master_port = env_master_port;
    } else {
        master_port = to_string(rum::kDefMasterInPort);
    }

    return "tcp://" + rum::kIpStr + ":" + master_port;
}

std::string GetMasterOutAddr() {
    string master_port;
    if (const char *env_master_port = std::getenv(kMasterPortOutEnv)) {
        printer.i("rum", "master outbound port from $%s: %s", kMasterPortOutEnv, env_master_port);
        master_port = env_master_port;
    } else {
        master_port = to_string(rum::kDefMasterOutPort);
    }

    return "tcp://" + rum::kIpStr + ":" + master_port;
}

}
