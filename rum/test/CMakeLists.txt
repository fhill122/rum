find_library(GTEST_LIB gtest)
if (GTEST_LIB)

add_executable(discovery_raw_tet
    discovery_raw.cc)
target_link_libraries(discovery_raw_tet gtest rumcore)

# extern

add_executable(zmq_test
    extern/zmq.cc)
target_link_libraries(zmq_test gtest rumcore)

# core

add_executable(core_master_test
    core/master_test.cc)
target_link_libraries(core_master_test gtest rumcore)

add_executable(core_node_test
    core/node_test.cc)
target_link_libraries(core_node_test gtest rumcore)

add_executable(core_topic_test
    core/topic_test.cc)
target_link_libraries(core_topic_test gtest rumcore)

add_executable(core_topic_test_companion
    core/topic_test_companion.cc)
target_link_libraries(core_topic_test_companion rumcore)

# serialization

add_executable(ser_native_test
    serialization/ser_native_test.cc)
target_link_libraries(ser_native_test gtest rumcore)

add_executable(ser_fb_test
    serialization/ser_fb_test.cc)
target_link_libraries(ser_fb_test gtest rumcore)

add_executable(ser_io_test
    serialization/ser_io_test.cc)
target_link_libraries(ser_io_test gtest rumcore)

# cpprum

add_executable(cpprum_topic_test
    cpprum/topic_test.cc)
target_link_libraries(cpprum_topic_test gtest cpprum)

add_executable(cpprum_topic_test_companion
    cpprum/topic_test_companion.cc)
target_link_libraries(cpprum_topic_test_companion cpprum)

add_executable(srv_test
    cpprum/srv_test.cc)
target_link_libraries(srv_test gtest cpprum)

add_executable(srv_test_companion
    cpprum/srv_test_companion.cc)
target_link_libraries(srv_test_companion cpprum)

endif()

add_subdirectory(dumb)
add_subdirectory(bench)
