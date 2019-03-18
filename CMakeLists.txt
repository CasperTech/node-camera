cmake_minimum_required(VERSION 3.7)
project(node_camera)

set(CMAKE_BUILD_TYPE Debug)
IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
set(CMAKE_CXX_STANDARD 14)

include(NodeJS.cmake)
nodejs_init()

if(UNIX AND NOT APPLE)
    add_nodejs_module(${PROJECT_NAME} node-camera.cpp Camera.cpp CapturedImage.cpp V4LCamera.cpp V4LCameraData.cpp ManualResetEvent.cpp VideoListener.cpp jpegEncoder.cpp jpegEncoderData.cpp)
    target_link_libraries(${PROJECT_NAME} PUBLIC jpeg v4l2)
else()
    add_nodejs_module(${PROJECT_NAME} node-camera-dummy.cpp CameraDummy.cpp)
endif()