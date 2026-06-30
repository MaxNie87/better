#include <asio.hpp>
#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

#include "camstreamkit/version.h"
#include "core/config.h"
#include "core/logger.h"
#include "core/media_hub.h"
#include "http/api_server.h"
#include "rtsp/rtsp_server.h"
#include "webrtc/whep_server.h"

int main(int argc, char *argv[]) {
    std::string config_path = "config.json";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-c" || arg == "--config") && i + 1 < argc) {
            config_path = argv[++i];
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "CamStreamKit " << CSK_VERSION_STRING << std::endl;
            return 0;
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: camstreamkit [options]\n"
                      << "  -c, --config <path>  Config file path (default: config.json)\n"
                      << "  -v, --version        Print version\n"
                      << "  -h, --help           Print help\n";
            return 0;
        }
    }

    auto config = csk::Config::from_file(config_path);
    csk::setup_logger(config.log);

    CSK_LOG_I("CamStreamKit {} starting...", CSK_VERSION_STRING);

    asio::io_context io;

    auto &hub = csk::MediaHub::instance();

    // RTSP Server
    csk::RtspServer rtsp_server(io, config.server.rtsp_port, hub);
    rtsp_server.start();

    // WebRTC/WHEP Server
    auto whep_server = std::make_shared<csk::WhepServer>(io, hub, config.webrtc.udp_port);
    if (config.webrtc.enabled) {
        whep_server->start();
        CSK_LOG_I("WebRTC/WHEP on UDP port {}", whep_server->udp_port());
    }

    // HTTP API Server (with WHEP integration)
    csk::ApiServer api_server(io, config.server.http_port, hub,
                              config.webrtc.enabled ? whep_server : nullptr);
    api_server.start();

    CSK_LOG_I("RTSP server on port {}", config.server.rtsp_port);
    CSK_LOG_I("HTTP API on port {}", config.server.http_port);

    // Signal handling for graceful shutdown
    asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait([&](const asio::error_code &, int sig) {
        CSK_LOG_I("Received signal {}, shutting down...", sig);
        rtsp_server.stop();
        api_server.stop();
        hub.stop_all();
        io.stop();
    });

    // Run io_context on multiple threads
    unsigned int thread_count = std::thread::hardware_concurrency();
    if (thread_count == 0) thread_count = 4;

    std::vector<std::thread> threads;
    for (unsigned int i = 1; i < thread_count; ++i) {
        threads.emplace_back([&io] { io.run(); });
    }

    io.run();

    for (auto &t : threads) {
        if (t.joinable()) t.join();
    }

    CSK_LOG_I("CamStreamKit stopped.");
    return 0;
}
