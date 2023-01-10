file(GLOB headers include/*)
file(GLOB sources *.c*)

add_library(UsageEnvironment OBJECT
        ${headers}
        ${sources}
        )

target_include_directories(UsageEnvironment
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # This is a cyclic dependency :(
        $<BUILD_INTERFACE:${live555_SOURCE_DIR}/groupsock/include>)