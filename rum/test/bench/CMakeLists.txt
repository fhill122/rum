add_executable(zmq_bench zmq_bench.cc)
target_link_libraries(zmq_bench zmq)

find_package(roscpp QUIET)
if(${roscpp_FOUND})
    set(ROS_INCLUDES ${ROS_INCLUDES} ${roscpp_INCLUDE_DIRS})
    set(ROS_LIBS ${ROS_LIBS} ${roscpp_LIBRARIES})

    # ros
    add_executable(ros_pingpong_bench ros_pingpong_bench.cc)
    target_include_directories(ros_pingpong_bench PUBLIC ${roscpp_INCLUDE_DIRS})
    target_link_libraries(ros_pingpong_bench ${ROS_LIBS})

    add_executable(ros_pingpong_bench2 ros_pingpong_bench2.cc)
    target_include_directories(ros_pingpong_bench2 PUBLIC ${roscpp_INCLUDE_DIRS})
    target_link_libraries(ros_pingpong_bench2 ${ROS_LIBS})

    add_executable(ros_pingpong_bench2_comp ros_pingpong_bench2_comp.cc)
    target_include_directories(ros_pingpong_bench2_comp PUBLIC ${roscpp_INCLUDE_DIRS})
    target_link_libraries(ros_pingpong_bench2_comp ${ROS_LIBS})

    # rum
    add_executable(rum_pingpong_bench rum_pingpong_bench.cc)
    target_include_directories(rum_pingpong_bench PUBLIC ${roscpp_INCLUDE_DIRS})
    target_link_libraries(rum_pingpong_bench cpprum)

    add_executable(rum_pingpong_bench2 rum_pingpong_bench2.cc)
    target_include_directories(rum_pingpong_bench2 PUBLIC ${roscpp_INCLUDE_DIRS})
    target_link_libraries(rum_pingpong_bench2 cpprum)

    add_executable(rum_pingpong_bench2_comp rum_pingpong_bench2_comp.cc)
    target_include_directories(rum_pingpong_bench2_comp PUBLIC ${roscpp_INCLUDE_DIRS})
    target_link_libraries(rum_pingpong_bench2_comp cpprum)
endif()

