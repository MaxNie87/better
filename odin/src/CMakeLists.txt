add_library(odin
    core/config.cpp
    core/file_handle.cpp
    core/logger.cpp
    core/socket.cpp
    core/thread_pool.cpp
    core/version.cpp
)

target_include_directories(odin
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(odin PUBLIC cxx_std_17)

target_link_libraries(odin
    PUBLIC
        spdlog::spdlog
        Threads::Threads
)
