add_executable(rumassemble
    component.h
    component_loader.cc
    main.cc)
# should we link cpprum?
target_link_libraries(rumassemble cpprum dl)

add_library(component_pub SHARED demo/compoent_pub.cc)
target_link_libraries(component_pub cpprum)

add_library(component_sub SHARED demo/component_sub.cc)
target_link_libraries(component_sub cpprum)