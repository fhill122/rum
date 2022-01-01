find_library(GTEST_LIB gtest)
if (GTEST_LIB)

add_executable(discovery_raw_tet
    discovery_raw.cc)
target_link_libraries(discovery_raw_tet gtest rumcore)

add_executable(publish_test
    publish.cc)
target_link_libraries(publish_test gtest rumcore)

add_executable(zmq_test
    zmq.cc)
target_link_libraries(zmq_test gtest rumcore)

add_executable(master_test
    core/master_test.cc)
target_link_libraries(master_test gtest rumcore)

add_executable(node_test
    core/node_test.cc)
target_link_libraries(node_test gtest rumcore)


add_executable(serialization_test
    serialization/serialization_test.cc)
target_link_libraries(serialization_test gtest rumcore)

endif()

add_subdirectory(dumb)
add_subdirectory(bench)
