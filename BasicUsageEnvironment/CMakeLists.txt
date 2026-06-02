file(GLOB headers include/*)
file(GLOB sources *.cpp)
add_library(BasicUsageEnvironment OBJECT
        ${headers}
        ${sources}
        )

target_include_directories(BasicUsageEnvironment PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${CMAKE_CURRENT_SOURCE_DIR}/../UsageEnvironment/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/../groupsock/include/
        )

if (WIN32)
    target_link_libraries(BasicUsageEnvironment OBJECT UsageEnvironment)
endif ()
