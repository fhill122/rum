/*
 * Created by Ivan B on 2022/1/15.
 */

#include "component_loader.h"

#include <dlfcn.h>
#include <rum/common/log.h>

#define TAG "rum-assembly"

using namespace std;

namespace rum{

ComponentLoader::ComponentLoader(const std::string &lib_path, const std::vector<std::string> &args)
        : lib_path_(lib_path), args_(args) {
    AssertLog(!args_.empty(), "");
    AssertLog(!lib_path_.empty(), "");
    for (const auto &s : args_){
        AssertLog(!s.empty(), "");
    }
}


bool ComponentLoader::load() {
    dl = dlopen(lib_path_.c_str(), RTLD_LAZY);
    if (!dl){
        log.e(TAG, "err load " + lib_path_);
        return false;
    }

    const char* dlsym_error;

    dlerror();
    auto* create_func = (CreateRumComponentF*) dlsym(dl, "CreateRumComponent");
    dlsym_error = dlerror();
    if (dlsym_error){
        log.e(TAG, "err load creation function in " + lib_path_);
        return false;
    }

    dlerror();
    auto* destroy_func = (DestroyRumComponentF*) dlsym(dl, "DestroyRumComponent");
    dlsym_error = dlerror();
    if (dlsym_error){
        log.e(TAG, "err load destruction function in " + lib_path_);
        return false;
    }

    component_= unique_ptr<Component, std::function<void(void*)>>(
                    (rum::Component*)create_func(), [destroy_func](void* p){destroy_func(p);} );

    return true;
}

void ComponentLoader::run() {
    std::vector<char*> cstrings;
    cstrings.reserve(args_.size());

    for(auto& s: args_)
        cstrings.push_back(&s[0]);

    component_->main(cstrings.size(), cstrings.data());

    component_.reset(nullptr);
    dlclose(dl);
}

}

#undef TAG


