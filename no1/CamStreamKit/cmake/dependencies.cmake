set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

# spdlog - logging
add_subdirectory(${THIRD_PARTY_DIR}/spdlog ${CMAKE_BINARY_DIR}/spdlog)

# nlohmann/json - JSON parsing
add_subdirectory(${THIRD_PARTY_DIR}/json ${CMAKE_BINARY_DIR}/json)

# standalone Asio - async I/O (header-only)
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${THIRD_PARTY_DIR}/asio/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
find_package(Threads REQUIRED)
target_link_libraries(asio INTERFACE Threads::Threads)

# Catch2 - testing
if(BUILD_TESTS)
    add_subdirectory(${THIRD_PARTY_DIR}/Catch2 ${CMAKE_BINARY_DIR}/Catch2)
endif()
