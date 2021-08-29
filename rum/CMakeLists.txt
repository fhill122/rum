project(rum)

include_directories(${CMAKE_SOURCE_DIR} extern/flatbuffers)

add_library(${PROJECT_NAME} SHARED
    common/common.cc
    common/log.cc
    common/zmq_helper.cc

    core/internal/server_base.cc
    core/internal/publisher_base_impl.cc
    core/internal/sub_container.cc
    core/internal/subscriber_base_impl.cc
    core/internal/node_base_impl.cc
    core/internal/master.cc
    core/internal/node_info.cc
    core/internal/socket_monitor.cc
    core/internal/itc_manager.cc

    core/publisher_base.cc
    core/subscriber_base.cc
    core/node_base.cc

    rum/node.h
    rum/subscriber.h
    rum/publisher.h

    extern/zmq/zmq.hpp

    # remove later, for syntax highlight only
    serialization/flatbuffers/serializer_fbs.h
    serialization/protobuf/serializer_proto.h
)
target_link_libraries(${PROJECT_NAME} zmq)

add_subdirectory(test/dumb)
add_subdirectory(test)
