/*
 * Created by Ivan B on 2022/1/14.
 */

#ifndef RUM_EXTRA_ASSEMBLY_COMPONENT_H_
#define RUM_EXTRA_ASSEMBLY_COMPONENT_H_

#include <type_traits>

namespace rum {

// todo ivan. think about the Init and Param

class Component {
  public:
    virtual int main(int argc, char** argv) = 0;
};

}

typedef void* CreateRumComponentF();
typedef void DestroyRumComponentF(void*);

#define RUM_EXPORT_COMPONENT(ClassT) \
void RumComponentCheck(){            \
    static_assert(std::is_base_of<rum::Component, ClassT>::value); \
}                                    \
                                     \
extern "C" {                         \
void* CreateRumComponent(){          \
    return new ClassT();             \
}                                    \
                                     \
void DestroyRumComponent(void* p){   \
    delete (ClassT*)p;               \
}                                    \
}

#endif //RUM_EXTRA_ASSEMBLY_COMPONENT_H_
