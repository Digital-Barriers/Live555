cmake_minimum_required(VERSION 3.5)
project(hlsProxy)
set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../liveMedia/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../groupsock/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../UsageEnvironment/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../BasicUsageEnvironment/include)

add_executable(hlsProxy
        live555HLSProxy.cpp)

target_link_libraries(hlsProxy live555)
