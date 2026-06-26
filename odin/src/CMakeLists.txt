add_library(odin
    core/dump.cpp
)

target_include_directories(odin
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(odin PUBLIC cxx_std_17)
