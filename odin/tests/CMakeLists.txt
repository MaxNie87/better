add_executable(odin_tests
    test_main.cpp
)

target_link_libraries(odin_tests
    PRIVATE
        odin
        Catch2::Catch2WithMain
)
