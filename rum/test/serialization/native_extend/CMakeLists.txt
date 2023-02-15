find_package(OpenCV)
if (OpenCV_FOUND)
    message(STATUS "Build OpenCV serialization test")
    add_executable(ser_native_cv_test
        ser_native_cv_test.cc)
    target_include_directories(ser_native_cv_test PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(ser_native_cv_test gtest rumcore ${OpenCV_LIBS})
endif ()

find_package(Eigen3)
if (Eigen3_FOUND)
    message(STATUS "Build Eigen serialization test")
    add_executable(ser_native_eigen_test
        ser_native_eigen_test.cc)
    target_link_libraries(ser_native_eigen_test gtest rumcore Eigen3::Eigen)

endif()