/*
 * Created by Ivan B on 2022/1/14.
 */

#include <thread>

#include "component_loader.h"
#include "rum/common/log.h"
#include "rum/extern/args/args.hxx"

#define TAG "rum-assembly"

using namespace std;
using namespace rum;

rum::Log logger(rum::Log::Level::v);

void printHelp(){
    logger.i(TAG, "run with: rumassemble component_lib_a component_lib_b ...");
}

// todo ivan. support passing arguments
int main(int argc, char** argv){
    if (argc<=1){
        printHelp();
        return 0;
    }

    // load components
    vector<ComponentLoader> components;
    components.reserve(argc-1);
    for (int i=1; i<argc; ++i){
        string lib = argv[i];
        vector<string> args;
        args.push_back(lib);
        ComponentLoader component(lib, args);
        logger.v(TAG, "loading component " +lib);
        if (!component.load()) return 1;
        components.push_back(move(component));
    }

    // run components
    vector<thread> threads;
    threads.reserve(argc-1);
    for (auto &component : components){
        threads.emplace_back([&](){component.run();});
    }
    for (auto &thread : threads){
        thread.join();
    }

    return 0;
}

#undef TAG
