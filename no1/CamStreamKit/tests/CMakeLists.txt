add_executable(test_buffer unit/test_buffer.cpp)
target_link_libraries(test_buffer PRIVATE camstreamkit_lib Catch2::Catch2WithMain)
add_test(NAME test_buffer COMMAND test_buffer)

add_executable(test_rtp unit/test_rtp.cpp)
target_link_libraries(test_rtp PRIVATE camstreamkit_lib Catch2::Catch2WithMain)
add_test(NAME test_rtp COMMAND test_rtp)

add_executable(test_h264_parser unit/test_h264_parser.cpp)
target_link_libraries(test_h264_parser PRIVATE camstreamkit_lib Catch2::Catch2WithMain)
add_test(NAME test_h264_parser COMMAND test_h264_parser)

add_executable(test_rtsp_parser unit/test_rtsp_parser.cpp)
target_link_libraries(test_rtsp_parser PRIVATE camstreamkit_lib Catch2::Catch2WithMain)
add_test(NAME test_rtsp_parser COMMAND test_rtsp_parser)

add_executable(test_media_hub unit/test_media_hub.cpp)
target_link_libraries(test_media_hub PRIVATE camstreamkit_lib Catch2::Catch2WithMain)
add_test(NAME test_media_hub COMMAND test_media_hub)
