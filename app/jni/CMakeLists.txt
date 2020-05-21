cmake_minimum_required(VERSION 3.6)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(target native-lib)
set(target2 native-lib2)
project(${target} CXX)

set(ANDROID_OPENCV_COMPONENTS "opencv_java" CACHE STRING "")
message(STATUS "ANDROID_ABI=${ANDROID_ABI}")
find_package(OpenCV REQUIRED COMPONENTS ${ANDROID_OPENCV_COMPONENTS})

file(GLOB srcs *.cpp *.c)
file(GLOB hdrs *.hpp *.h)

# 设置dlib
set(NATIVE_DIR ${CMAKE_SOURCE_DIR})
include(${NATIVE_DIR}/dlib/dlib/cmake)
include_directories("../src/main/cpp")
add_library(${target} SHARED ${srcs} ${hdrs})
target_link_libraries(${target} ${ANDROID_OPENCV_COMPONENTS})

add_library(${target2} SHARED ${srcs} ${hdrs})
target_link_libraries(${target2} ${ANDROID_OPENCV_COMPONENTS})

target_link_libraries( # Specifies the target library.
        dlib
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} )
