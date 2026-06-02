file(GLOB headers include/*)
file(GLOB sources *.c*)

add_library(groupsock OBJECT
        ${headers}
        ${sources})

target_include_directories(groupsock
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${CMAKE_CURRENT_SOURCE_DIR}/../UsageEnvironment/include/
        )

if (WIN32)
    target_link_libraries(groupsock OBJECT
            UsageEnvironment
            $<$<PLATFORM_ID:Windows>:ws2_32 mswsock>)
endif ()
