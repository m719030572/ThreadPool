include_directories(.)
add_executable(main main.cpp)
target_link_libraries(main threadpool ${CONAN_LIBS_GLOG} ${CONAN_LIBS_GFLAGS})
target_include_directories(main PRIVATE ${CONAN_INCLUDE_DIRS_GLOG})