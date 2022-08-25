add_executable(zmq_r2r zmq_r2r.cc)
target_link_libraries(zmq_r2r zmq)

add_executable(zmq_pubsub zmq_pubsub.cc)
target_link_libraries(zmq_pubsub zmq)

add_executable(trivially_copyable trivially_copyable.cc)

add_executable(simple_service simple_service.cc)
target_link_libraries(simple_service rumcore cpprum)