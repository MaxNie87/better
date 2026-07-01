set(CSK_SOURCES
    core/buffer.cpp
    core/config.cpp
    core/thread_pool.cpp
    core/media_source.cpp
    core/media_hub.cpp
    core/gop_cache.cpp
    net/tcp_server.cpp
    net/udp_socket.cpp
    rtp/rtp_packet.cpp
    rtp/rtp_packetizer.cpp
    codec/h264_parser.cpp
    codec/h265_parser.cpp
    sdp/sdp_parser.cpp
    rtsp/rtsp_parser.cpp
    rtsp/rtsp_client.cpp
    rtsp/rtsp_server.cpp
    http/api_server.cpp
    webrtc/dtls_context.cpp
    webrtc/srtp_session.cpp
    webrtc/stun.cpp
    webrtc/webrtc_session.cpp
    webrtc/whep_server.cpp
    gb28181/sip_message.cpp
    gb28181/ps_demuxer.cpp
    gb28181/gb28181_server.cpp
)

find_package(OpenSSL REQUIRED)

add_library(camstreamkit_lib STATIC ${CSK_SOURCES})
target_include_directories(camstreamkit_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(camstreamkit_lib PUBLIC
    asio
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
)

add_executable(camstreamkit main.cpp)
target_link_libraries(camstreamkit PRIVATE camstreamkit_lib)
