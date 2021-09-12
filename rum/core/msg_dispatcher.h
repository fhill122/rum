//
// Created by Ivan B on 2021/3/16.
//

//cb in tp hold a weak ptr
#ifndef RUM_CORE_MSG_DISPATCHER_H_
#define RUM_CORE_MSG_DISPATCHER_H_

#include <unordered_map>

#include <rum/extern/ivtb/thread_pool.h>

namespace rum {

class MsgDispatcher {
  public:
    struct MsgQueue {
        const int kSize;
        std::queue<std::shared_ptr<void>> queue;

        explicit MsgQueue(int size) : kSize(size) {}
        void add();
    };

    ivtb::ThreadPool tp_;
    // for controlling queue size_ only
    std::unordered_map<std::string, MsgQueue> task_queues;  // <topic/srv , msg>

  private:
    // std::unordered_map<std::string,
  public:

  private:
    void loop(){
        // receive msg
        // find msg queue and thread pool from its name
        // pop front if queue if full
        // enqueue a task on the queue to the thread pool
    }
  public:
};

}
#endif //RUM_CORE_MSG_DISPATCHER_H_
