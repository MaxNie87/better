# CamStreamKit

轻量级摄像头流媒体代理库 -- ZLMediaKit 的轻量替代

## 功能特性

- **RTSP 代理**: 按需拉流、断线重连、GOP 缓存秒开、1:N 多路分发
- **H.264/H.265 解析**: Annex-B 解析、RTP 打包/解包、FU-A 分片重组
- **GB28181 国标**: SIP 注册、Keepalive、INVITE 点播、RTP/PS 解封装
- **WebRTC 浏览器播放**: WHEP 协议、ICE-lite、DTLS/SRTP、无插件播放
- **RESTful API**: 流管理、统计信息、Prometheus 指标

## 快速开始

### 环境要求

- CMake >= 3.20
- GCC >= 9 或 Clang >= 11
- Linux (Ubuntu 20.04+)

### 准备依赖

项目依赖已放置在 `third_party/` 目录下，无需联网下载：

```
third_party/
├── asio/       # Standalone Asio (异步网络 I/O)
├── Catch2/     # 单元测试框架
├── json/       # nlohmann/json
└── spdlog/     # 日志库
```

如果 `third_party/` 为空，手动克隆：

```bash
mkdir -p third_party && cd third_party
git clone https://github.com/gabime/spdlog.git
git clone https://github.com/nlohmann/json.git
git clone https://github.com/chriskohlhoff/asio.git
git clone https://github.com/catchorg/Catch2.git
```

### 编译

```bash
cd CamStreamKit
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### 编译产物

| 路径 | 说明 |
|------|------|
| `build/src/camstreamkit` | 主程序可执行文件 |
| `build/src/libcamstreamkit_lib.a` | 核心静态库 |
| `build/examples/simple_proxy` | 示例程序 |
| `build/tests/test_*` | 单元测试 |

### 运行

```bash
./build/src/camstreamkit -c config.json
```

### 运行测试

```bash
cd build && ctest --output-on-failure
```

### 添加摄像头

```bash
curl -X POST http://localhost:8080/api/v1/streams \
  -H "Content-Type: application/json" \
  -d '{"id":"cam1","url":"rtsp://admin:123456@192.168.1.100/stream1"}'
```

### 播放

```bash
# VLC / ffplay
vlc rtsp://localhost:554/stream/cam1

# 列出所有流
curl http://localhost:8080/api/v1/streams | jq
```

## 项目结构

```
CamStreamKit/
├── CMakeLists.txt
├── cmake/
│   └── dependencies.cmake      # 依赖配置（指向 third_party/）
├── config.json                 # 示例配置
├── src/
│   ├── main.cpp                # 程序入口
│   ├── core/
│   │   ├── buffer.h/cpp        # 字节缓冲区 + Span + BufferReader/Writer
│   │   ├── config.h/cpp        # 配置解析
│   │   ├── gop_cache.h/cpp     # GOP 缓存（秒开）
│   │   ├── logger.h/cpp        # spdlog 日志封装
│   │   ├── media_frame.h       # 媒体帧定义
│   │   ├── media_hub.h/cpp     # 流注册中心（单例）
│   │   ├── media_source.h/cpp  # 媒体源 + 订阅者管理
│   │   ├── thread_pool.h/cpp   # 线程池
│   │   └── timer.h             # 定时器 + 重连策略
│   ├── net/
│   │   ├── tcp_server.h/cpp    # TCP 连接管理
│   │   └── udp_socket.h/cpp    # UDP + RTP 端口池
│   ├── rtp/
│   │   ├── rtp_packet.h/cpp    # RTP/RTCP 解析
│   │   └── rtp_packetizer.h/cpp # RTP 打包（FU-A 分片）
│   ├── codec/
│   │   ├── h264_parser.h/cpp   # H.264 NAL 解析
│   │   └── h265_parser.h/cpp   # H.265 NAL 解析
│   ├── sdp/
│   │   └── sdp_parser.h/cpp    # SDP 解析/生成
│   ├── rtsp/
│   │   ├── rtsp_parser.h/cpp   # RTSP 协议解析
│   │   ├── rtsp_client.h/cpp   # RTSP 拉流客户端
│   │   └── rtsp_server.h/cpp   # RTSP 服务端
│   ├── gb28181/
│   │   ├── sip_message.h/cpp  # SIP 协议解析/生成
│   │   ├── ps_demuxer.h/cpp   # PS 流解封装
│   │   └── gb28181_server.h/cpp # GB28181 SIP 服务 + 媒体源
│   ├── webrtc/
│   │   ├── dtls_context.h/cpp # DTLS 上下文管理
│   │   ├── srtp_session.h/cpp # SRTP 加密
│   │   ├── stun.h/cpp         # STUN 消息处理
│   │   ├── webrtc_session.h/cpp # WebRTC 会话
│   │   └── whep_server.h/cpp  # WHEP 协议服务
│   └── http/
│       └── api_server.h/cpp    # REST API + Prometheus 指标
├── tests/                      # Catch2 单元测试
├── examples/                   # 示例代码
├── third_party/                # 本地依赖库
├── Dockerfile
└── docker-compose.yml
```

## 架构设计

```
视频输入 (RTSP摄像头 / GB28181设备)
    │
    ▼
┌──────────────────────────────────┐
│       MediaHub (核心路由)          │
│  - 流注册/注销                    │
│  - 按需拉流 + 指数退避重连        │
│  - 1:N 多路分发 (零拷贝)          │
│  - GOP 缓存秒开                  │
└───────────────┬──────────────────┘
                │
     ┌──────────┼──────────┐
     ▼          ▼          ▼
   RTSP      WebRTC     GB28181
   Server    (WHEP)      推流
```

## API 文档

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | /api/v1/streams | 列出所有流 |
| POST | /api/v1/streams | 添加 RTSP 摄像头 |
| GET | /api/v1/streams/{id} | 获取流详情 |
| DELETE | /api/v1/streams/{id} | 删除流 |
| POST | /whep/{stream_id} | WebRTC WHEP 播放 |
| GET | /api/v1/gb28181/devices | 列出已注册 GB28181 设备 |
| POST | /api/v1/gb28181/invite | 邀请设备推流 (body: `{"device_id":"...", "stream_id":"..."}`) |
| POST | /api/v1/gb28181/bye/{stream_id} | 停止 GB28181 推流 |
| GET | /api/v1/version | 版本信息 |
| GET | /metrics | Prometheus 指标 |

## 配置文件

```json
{
    "server": { "http_port": 8080, "rtsp_port": 554 },
    "camera": { "reconnect_base_ms": 1000, "reconnect_max_ms": 30000, "timeout_ms": 10000 },
    "gb28181": {
        "enabled": true,
        "sip_port": 5060,
        "server_id": "34020000002000000001",
        "domain": "3402000000",
        "password": "12345678",
        "heartbeat_timeout_s": 180
    },
    "webrtc": { "enabled": true, "udp_port": 8555, "stun_server": "stun:stun.l.google.com:19302" },
    "log": { "level": "info", "file": "" }
}
```

## 构建选项

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_ASAN=OFF \
  -DENABLE_TSAN=OFF \
  -DBUILD_TESTS=ON \
  -DBUILD_EXAMPLES=ON
```

| 选项 | 默认 | 说明 |
|------|------|------|
| CMAKE_BUILD_TYPE | Release | 构建类型 (Debug/Release/RelWithDebInfo) |
| ENABLE_ASAN | OFF | 地址消毒器 (检测内存越界) |
| ENABLE_TSAN | OFF | 线程消毒器 (检测数据竞争) |
| BUILD_TESTS | ON | 编译单元测试 |
| BUILD_EXAMPLES | ON | 编译示例程序 |

## 技术栈

| 技术 | 用途 |
|------|------|
| C++17 | 项目语言 |
| Asio (standalone) | 异步网络 I/O |
| spdlog | 高性能日志 |
| nlohmann/json | JSON 解析 |
| Catch2 | 单元测试 |

## License

MIT
