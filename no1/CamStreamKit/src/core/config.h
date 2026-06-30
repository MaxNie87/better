#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <fstream>

#include "core/logger.h"

namespace csk {

struct ServerConfig {
    uint16_t http_port = 8080;
    uint16_t rtsp_port = 554;
};

struct CameraConfig {
    int reconnect_base_ms = 1000;
    int reconnect_max_ms = 30000;
    int timeout_ms = 10000;
    int idle_timeout_s = 10;
};

struct Gb28181Config {
    bool enabled = false;
    uint16_t sip_port = 5060;
    std::string server_id = "34020000002000000001";
    std::string domain = "3402000000";
    std::string password = "12345678";
    int heartbeat_timeout_s = 180;
};

struct WebRtcConfig {
    bool enabled = false;
    uint16_t udp_port = 8555;
    std::string stun_server = "stun:stun.l.google.com:19302";
};

struct Config {
    ServerConfig server;
    CameraConfig camera;
    Gb28181Config gb28181;
    WebRtcConfig webrtc;
    LogConfig log;

    static Config from_file(const std::string &path);
    static Config defaults();
};

inline Config Config::defaults() {
    return Config{};
}

inline Config Config::from_file(const std::string &path) {
    Config config;
    std::ifstream file(path);
    if (!file.is_open()) {
        CSK_LOG_W("Config file not found: {}, using defaults", path);
        return config;
    }

    try {
        nlohmann::json j;
        file >> j;

        if (j.contains("server")) {
            auto &s = j["server"];
            if (s.contains("http_port")) config.server.http_port = s["http_port"];
            if (s.contains("rtsp_port")) config.server.rtsp_port = s["rtsp_port"];
        }

        if (j.contains("camera")) {
            auto &c = j["camera"];
            if (c.contains("reconnect_base_ms")) config.camera.reconnect_base_ms = c["reconnect_base_ms"];
            if (c.contains("reconnect_max_ms")) config.camera.reconnect_max_ms = c["reconnect_max_ms"];
            if (c.contains("timeout_ms")) config.camera.timeout_ms = c["timeout_ms"];
            if (c.contains("idle_timeout_s")) config.camera.idle_timeout_s = c["idle_timeout_s"];
        }

        if (j.contains("gb28181")) {
            auto &g = j["gb28181"];
            if (g.contains("enabled")) config.gb28181.enabled = g["enabled"];
            if (g.contains("sip_port")) config.gb28181.sip_port = g["sip_port"];
            if (g.contains("server_id")) config.gb28181.server_id = g["server_id"];
            if (g.contains("domain")) config.gb28181.domain = g["domain"];
            if (g.contains("password")) config.gb28181.password = g["password"];
            if (g.contains("heartbeat_timeout_s")) config.gb28181.heartbeat_timeout_s = g["heartbeat_timeout_s"];
        }

        if (j.contains("webrtc")) {
            auto &w = j["webrtc"];
            if (w.contains("enabled")) config.webrtc.enabled = w["enabled"];
            if (w.contains("udp_port")) config.webrtc.udp_port = w["udp_port"];
            if (w.contains("stun_server")) config.webrtc.stun_server = w["stun_server"];
        }

        if (j.contains("log")) {
            auto &l = j["log"];
            if (l.contains("level")) config.log.level = l["level"];
            if (l.contains("file")) config.log.file = l["file"];
        }
    } catch (const std::exception &e) {
        CSK_LOG_E("Failed to parse config: {}", e.what());
    }

    return config;
}

}  // namespace csk
