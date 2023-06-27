cmake_minimum_required(VERSION 3.5)

# Parse version number from the header file
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/liveMedia/include/liveMedia_version.hh contents)
string(REGEX MATCH "define LIVEMEDIA_LIBRARY_VERSION_STRING.*\"([0-9\.]+)\"" _ ${contents})
set(project_version ${CMAKE_MATCH_1})

project(live555
        LANGUAGES C CXX
        VERSION ${project_version}
        )
message(STATUS "live555 version: ${project_version}")

list(APPEND CMAKE_MODULE_PATH ${live555_SOURCE_DIR}/cmake)

option(LIVE555_EPOLL_SUPPORT "Enable epoll task scheduler (Windows also)" ON)
option(LIVE555_BUILD_EXAMPLES "Build examples and test programs" OFF)

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
else ()
    add_compile_options(-DSOCKLEN_T=socklen_t)
    add_compile_options(-DALLOW_RTSP_SERVER_PORT_REUSE=1)
endif ()

add_definitions(-DNO_OPENSSL -DNO_GETIFADDRS)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(UsageEnvironment)
add_subdirectory(groupsock)
add_subdirectory(BasicUsageEnvironment)
add_subdirectory(liveMedia)

add_library(live555 SHARED
        $<TARGET_OBJECTS:UsageEnvironment>
        $<TARGET_OBJECTS:groupsock>
        $<TARGET_OBJECTS:BasicUsageEnvironment>
        $<TARGET_OBJECTS:liveMedia>)
target_link_libraries(live555)

set_target_properties(live555 PROPERTIES LINKER_LANGUAGE CXX)

if (WIN32)
    target_link_libraries(live555 ws2_32 mswsock)
endif ()

if (LIVE555_BUILD_EXAMPLES)
    add_subdirectory(proxyServer)
    add_subdirectory(mediaServer)
    add_subdirectory(testProgs)
    add_subdirectory(hlsProxy)
endif ()

