add_executable(zmq_r2r zmq_r2r.cc)
target_link_libraries(zmq_r2r zmq)

add_executable(zmq_pubsub zmq_pubsub.cc)
target_link_libraries(zmq_pubsub zmq)

add_executable(universal_ser universal_ser.cc)

add_executable(universal_ser_crtp universal_ser_crtp.cc)

add_executable(sptr sptr.cc)

add_executable(trivially_copyable trivially_copyable.cc)

add_executable(simple_node_1 simple_node_1.cc)
target_link_libraries(simple_node_1 rumcore cpprum)

add_executable(simple_node_2 simple_node_2.cc)
target_link_libraries(simple_node_2 rumcore)