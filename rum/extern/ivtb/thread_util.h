//
// Created by Ivan B on 2021/8/26.
//

#ifndef IVTB_THREAD_THREAD_UTIL_H_
#define IVTB_THREAD_THREAD_UTIL_H_

#include <thread>
#include <string>

namespace ivtb{

inline bool NameThread(const std::string &name){
    // pthread_setname_np();
#ifdef _PTHREAD_T
    // thread.native_handle().pthread_setname_np(name.c_str());
    pthread_setname_np(name.c_str());
    return true;
#endif
    return false;
}
}

#endif //IVTB_THREAD_THREAD_UTIL_H_
