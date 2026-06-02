file(GLOB headers include/*)
file(GLOB sources *.c*)

add_library(liveMedia OBJECT
        ${headers}
        ${sources}
        )

target_include_directories(liveMedia
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${CMAKE_CURRENT_SOURCE_DIR}/../groupsock/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/../UsageEnvironment/include/
        )

if (WIN32)
    target_link_libraries(liveMedia OBJECT
            groupsock
            UsageEnvironment)
endif ()

if (LIVE555_EPOLL_SUPPORT)
    target_compile_definitions(liveMedia PRIVATE READ_FROM_FILES_SYNCHRONOUSLY)
endif ()
