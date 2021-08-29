//
// Created by Ivan B on 2021/3/30.
//

#ifndef RUM_COMMON_LOCKER_HELPER_H_
#define RUM_COMMON_LOCKER_HELPER_H_

#include <mutex>

namespace rum{

template<class T>
class OptionalLock{
    const bool to_lock_;
    T &mu_;

  public:
    explicit OptionalLock(T &mu, bool enable=true) : to_lock_(enable), mu_(mu) {
        if (to_lock_) mu_.lock();
    }

    ~OptionalLock(){
        if (to_lock_) mu_.unlock();
    }

    // todo ivan. disable copy, move, assignment, etc.
};

}

#endif //RUM_COMMON_LOCKER_HELPER_H_
