/*
 * Created by Ivan B on 2022/1/15.
 */

#ifndef RUM_EXTRA_ASSEMBLY_COMPONENT_LOADER_H_
#define RUM_EXTRA_ASSEMBLY_COMPONENT_LOADER_H_

#include <vector>
#include <string>
#include <memory>
#include "component.h"

namespace rum{

class ComponentLoader {
  private:
    std::vector<std::string> args_;
    std::string lib_path_;

    std::unique_ptr<Component, std::function<void(void*)>> component_ = nullptr;
    void *dl = nullptr;
  public:

  private:
  public:
    ComponentLoader(const std::string &lib_path, const std::vector<std::string> &args);
    ComponentLoader(const ComponentLoader&) = delete;
    ComponentLoader(ComponentLoader&&) = default;
    ComponentLoader& operator=(ComponentLoader&&) = default;
    ComponentLoader& operator=(const ComponentLoader&) = delete;

    bool load();

    // run main, and unload lib
    void run();
};

}
#endif //RUM_EXTRA_ASSEMBLY_COMPONENT_LOADER_H_
