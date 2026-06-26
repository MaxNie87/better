# CMake generated Testfile for 
# Source directory: /code3/better/odin
# Build directory: /code3/better/odin/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(odin_tests "/code3/better/odin/build/tests/odin_tests")
set_tests_properties(odin_tests PROPERTIES  _BACKTRACE_TRIPLES "/code3/better/odin/CMakeLists.txt;22;add_test;/code3/better/odin/CMakeLists.txt;0;")
subdirs("_deps/spdlog-build")
subdirs("_deps/catch2-build")
subdirs("src")
subdirs("tests")
subdirs("examples")
