#ifndef THREAD_UTIL_H
#define THREAD_UTIL_H

#include <thread>
#include <string>

namespace tu{

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


#endif