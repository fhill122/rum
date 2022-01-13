/*
 * Created by Ivan B on 2022/1/12.
 */

#ifndef IVTB_THREAD_OPTIONAL_LOCK_H_
#define IVTB_THREAD_OPTIONAL_LOCK_H_

#include <mutex>

namespace ivtb{

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
};

}


#endif //IVTB_THREAD_OPTIONAL_LOCK_H_
