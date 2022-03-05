//
// Created by Ivan B on 2021/8/26.
//

#ifndef IVTB_THREAD_THREAD_UTIL_H_
#define IVTB_THREAD_THREAD_UTIL_H_

#include <thread>
#include <string>

namespace ivtb{

/**
 * Name current thread
 * @param name Name
 * @return whether successful
 */
inline bool NameThread(const std::string &name){
    // pthread_setname_np();
#ifdef _PTHREAD_T
    // thread.native_handle().pthread_setname_np(name.c_str());
    pthread_setname_np(name.c_str());
    return true;
#endif
    return false;
}

/**
 * Set current thread priority
 * @param priority Percentage from 0-1.0
 * @return whether successful
 */
inline bool SetThreadPriority(float priority){
#if defined(__APPLE__) || defined(__linux__)
    // for macos, default is policy 1, prior 31. min prior 15, max prior 47.
    // able to set prior within 0-127 without err
    static int prior_min = sched_get_priority_min(SCHED_OTHER);
    static int prior_max = sched_get_priority_max(SCHED_OTHER);

    int prior_int = std::round((prior_max-prior_min)*priority + prior_min);
    sched_param sched_param{prior_int};
    int res = pthread_setschedparam(pthread_self(), SCHED_OTHER, &sched_param);

    // get for some testing
    // int policy;
    // pthread_getschedparam(pthread_self(), &policy, &sched_param);

    return res==0;

#elif defined(_WIN32)
    // todo
    // Win32:   BOOL SetThreadPriority(HANDLE hThread,int nPriority)
#endif
    return false;
}

/**
 * Put current thread to idle state (background)
 * @return whether successful
 */
inline bool IdleThread(){
#ifdef __linux__
    sched_param sched_param{0};
    pthread_setschedparam(pthread_self(), SCHED_IDLE, &sched_param);
    return true;
#endif
#ifdef __APPLE__
    int res = setpriority(PRIO_DARWIN_THREAD,0,PRIO_DARWIN_BG);
    // int priority = getpriority(PRIO_DARWIN_THREAD,0);
    // printf("priority: %d\n", priority);
    return res==0;
#endif
}

}

#endif //IVTB_THREAD_THREAD_UTIL_H_
