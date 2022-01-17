/*
 * Created by Ivan B on 2022/1/17.
 */

#ifndef RUM_TEST_TEST_UTILS_PROCESS_UTILS_H_
#define RUM_TEST_TEST_UTILS_PROCESS_UTILS_H_

#include <string>

namespace rum{

// unlike std::system, this can be called in multiple threads
inline bool LaunchProcess(std::string command){
    // redirect stderr as well
    command += " 2>&1";

    FILE* proc;
    char buff[1024];

    proc = popen(command.c_str(), "r");
    if (!proc) {
        fprintf(stderr, "err launch: %s\n", command.c_str());
        return false;
    }

    // print output
    while(fgets(buff, sizeof(buff), proc) != nullptr){
        std::cout<<buff;
    }

    return true;
}


}

#endif //RUM_TEST_TEST_UTILS_PROCESS_UTILS_H_
