//
// Created by Ivan B on 2021/8/6.
//

#ifndef RUM_CORE_INTERNAL_ITC_MANAGER_H_
#define RUM_CORE_INTERNAL_ITC_MANAGER_H_


#include <unordered_map>
#include <string>
#include <mutex>

namespace rum {

class NodeBaseImpl;

struct ItcManager {

    // <master address in, nodes>
    std::unordered_map<std::string, std::vector<NodeBaseImpl*>> nodes;
    std::mutex nodes_mu;

  public:
    // static ItcManager &Get();

    // on sub creation

    // on pub creation

};

}

#endif //RUM_CORE_INTERNAL_ITC_MANAGER_H_
