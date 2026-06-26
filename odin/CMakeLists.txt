cmake_minimum_required(VERSION 3.16)

project(odin
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ODIN_BUILD_EXAMPLES "Build the examples" ON)

include(CTest)
enable_testing()
include(cmake/dependencies.cmake)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_test(NAME odin_tests COMMAND odin_tests)
endif()

if(ODIN_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
