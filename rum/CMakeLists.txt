project(rum)

find_package(Threads REQUIRED)

include_directories(BEFORE ${CMAKE_SOURCE_DIR} extern)

add_library(rumcore SHARED
    common/common.cc
    common/misc.cc
    common/zmq_helper.cc

    core/internal/server_base_impl.cc
    core/internal/client_base_impl.cc
    core/internal/publisher_base_impl.cc
    core/internal/sub_container.cc
    core/internal/subscriber_base_impl.cc
    core/internal/node_base_impl.cc
    core/internal/master.cc
    core/internal/intra_proc_manager.cc
    core/internal/remote_manager.cc

    core/publisher_base_handler.cc
    core/subscriber_base_handler.cc
    core/server_base_handler.cc
    core/client_base_handler.cc
    core/node_base.cc

    extern/zmq/zmq.hpp

    # remove later, for syntax highlight only
    serialization/flatbuffers/serializer_fbs.h
    serialization/protobuf/serializer_proto.h
)
target_link_libraries(rumcore zmq Threads::Threads)
# Release or RelWithDebInfo
if (CMAKE_BUILD_TYPE MATCHES "Rel")
    target_compile_definitions(rumcore PRIVATE LOG_SUPRESS_I)
endif()

add_library(cpprum SHARED
    cpprum/rum.h
    cpprum/subscriber.h
    cpprum/publisher.h
    cpprum/server.h
    cpprum/client.h
)
set_target_properties(cpprum PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(cpprum rumcore)

add_subdirectory(test)
add_subdirectory(extra/assembly)
