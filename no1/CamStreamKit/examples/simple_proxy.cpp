#include <asio.hpp>
#include <iostream>

#include "core/config.h"
#include "core/logger.h"
#include "core/media_hub.h"
#include "http/api_server.h"
#include "rtsp/rtsp_client.h"
#include "rtsp/rtsp_server.h"

int main() {
    csk::setup_logger({.level = "info"});
    CSK_LOG_I("CamStreamKit Simple Proxy Example");

    asio::io_context io;
    auto &hub = csk::MediaHub::instance();

    // Create RTSP server (serves streams to clients)
    csk::RtspServer rtsp_server(io, 554, hub);
    rtsp_server.start();

    // Create HTTP API server
    csk::ApiServer api_server(io, 8080, hub);
    api_server.start();

    // Add a camera source (can also be done via REST API)
    csk::RtspClient::Config cam_config;
    cam_config.url = "rtsp://localhost:8554/camera1";
    cam_config.transport = csk::TransportMode::TCP;

    auto source = std::make_shared<csk::RtspMediaSource>("cam1", cam_config, io);
    hub.add_source(source);
    source->start();

    CSK_LOG_I("Proxy running: rtsp://localhost:554/stream/cam1");
    CSK_LOG_I("API: http://localhost:8080/api/v1/streams");

    // Graceful shutdown
    asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait([&](auto, int) {
        CSK_LOG_I("Shutting down...");
        io.stop();
    });

    io.run();
    return 0;
}
