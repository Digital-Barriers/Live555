file(GLOB headers include/*)

add_library(EpollTaskScheduler OBJECT ${headers} EpollTaskScheduler.cpp)

if (WIN32)
    target_sources(EpollTaskScheduler
            PRIVATE
            epoll.c
            epoll.h
            msafd.h
            ntapi.h
            tree.h
            )
endif ()

target_include_directories(EpollTaskScheduler
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${CMAKE_CURRENT_SOURCE_DIR}/../BasicUsageEnvironment/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/../UsageEnvironment/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/../groupsock/include/
        )

if (WIN32)
    target_link_libraries(EpollTaskScheduler OBJECT
            BasicUsageEnvironment
            )
endif ()

target_compile_definitions(EpollTaskScheduler INTERFACE HAVE_EPOLL_SCHEDULER)
