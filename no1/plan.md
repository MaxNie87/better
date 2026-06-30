# CamStreamKit 学习与实现计划

> 从零到发布：如何学习并实现一套类 ZLMediaKit 的轻量级流媒体分发库

---

## 一、项目认知：先理解再动手

### 1.1 项目定位

CamStreamKit 是 ZLMediaKit 的**轻量替代**，专注于摄像头流媒体代理场景：

| 维度 | ZLMediaKit | CamStreamKit |
|------|-----------|--------------|
| 代码量 | 10W+ 行 | 1.5-2W 行 |
| 定位 | 通用流媒体服务器 | 摄像头代理库 |
| C++ 标准 | C++11 | C++17 |
| GB28181 | 依赖外部 WVP | 内置 SIP 信令 |
| 学习曲线 | 陡峭 | 适中 |
| 核心场景 | 全协议全功能 | 摄像头接入 + 多协议分发 |

**核心功能链路：**

```
视频输入 (RTSP/GB28181 摄像头)
    │
    ▼
┌──────────────────────────────┐
│  MediaHub (核心路由引擎)      │
│  - 流注册/注销               │
│  - 按需拉流                  │
│  - 1:N 多路分发              │
│  - GOP 缓存秒开             │
└──────────────┬───────────────┘
               │
    ┌──────────┼──────────┐
    ▼          ▼          ▼
  RTSP      WebRTC     GB28181
  输出      (WHEP)      推流
```

### 1.2 核心技术栈

| 技术 | 用途 | 学习优先级 |
|------|------|-----------|
| C++17 | 项目语言（RAII、智能指针、协程） | 最高 |
| Boost.Asio / standalone Asio | 异步网络 I/O 框架 | 最高 |
| RTP/RTSP/SDP | 流媒体传输协议 | 最高 |
| H.264/H.265 NAL | 视频编码格式 | 高 |
| FFmpeg (libav*) | 转码引擎 | 高 |
| SIP + GB28181 | 国标监控协议 | 中 |
| libdatachannel | WebRTC 实现 | 中 |
| CMake | 构建系统 | 基础 |
| spdlog / nlohmann-json | 日志与配置 | 基础 |

### 1.3 学习前置条件

**必须具备：**
- C/C++ 基本语法（指针、类、模板）
- 了解 TCP/UDP socket 编程概念
- 了解多线程基本概念（mutex、condition_variable）
- 会用 CMake 构建 C++ 项目

**建议提前了解：**
- `std::shared_ptr` / `std::unique_ptr` 使用场景
- 网络字节序（大端序 Big-Endian）
- 视频编码的基本概念（I 帧、P 帧、关键帧间隔）

---

## 二、学习路径：4 个递进阶段

### 阶段一：基础能力 (Week 1-2)

> 目标：掌握独立写项目所需的 C++ 工程能力和网络编程能力

#### Week 1：项目骨架 + 网络编程

| 天 | 主题 | 产出 |
|----|------|------|
| Day 1 | CMake 工程搭建 | 项目目录结构、CI 绿色 |
| Day 2 | RAII + 智能指针 | Socket/File/FFmpeg RAII wrapper |
| Day 3 | 多线程 + 线程池 | ThreadPool 类 + 并发测试 |
| Day 4 | Asio 异步 I/O | TCP echo server (协程版) |
| Day 5 | TCP 服务器框架 | TcpServer + Session + HTTP 响应 |
| Day 6 | UDP 收发 + 定时器 | UdpSocket + Timer 工具类 |
| Day 7 | 工具库集成 | spdlog + JSON 配置 + Buffer/Url 工具 |

**关键学习资源：**
- Asio 官方文档 + 示例代码
- "C++ Concurrency in Action" 第 2-4 章

#### Week 2：字节流处理 + RTP/RTSP 基础

| 天 | 主题 | 产出 |
|----|------|------|
| Day 8 | 二进制协议解析框架 | BufferReader / BufferWriter |
| Day 9 | RTP/RTCP 协议解析 | RtpPacket 解析 + RTCP SR 解析 |
| Day 10 | H.264 码流基础 | H264Parser (NAL 分离 + FU-A 重组) |
| Day 11 | H.265 + SDP 解析 | H265Parser + SdpParser |
| Day 12-13 | RTSP 协议解析 | RtspRequest/Response 解析 + Digest 认证 |
| Day 14 | RTSP Client v0.1 | 能拉取 RTSP 流并保存为 .h264 文件 |

**里程碑验收：** 能从真实 RTSP 摄像头拉流并保存为可播放的 H.264 裸流文件

---

### 阶段二：核心能力 (Week 3-7)

> 目标：实现完整的 RTSP 代理 + MediaHub + 转码

#### Week 3：RTSP Server

```
学习顺序：
Session 管理 → 请求解析管线 → DESCRIBE(SDP) → SETUP+PLAY → RTP 打包 → 认证
```

**核心接口设计：**

```cpp
class RtspServer {
public:
    RtspServer(asio::io_context &io, uint16_t port = 554);
    void start();
    void stop();
    void add_source(const std::string &path, std::shared_ptr<MediaSource> source);
};
```

**Week 3 验收：** VLC 能通过 CamStreamKit 代理正常观看 RTSP 摄像头视频

#### Week 4：MediaHub 核心路由

```
学习顺序：
MediaSource 抽象 → 编码信息提取 → 流注册/查找 → 1:N 分发 → 按需拉流 → 断线重连
```

**核心架构：**

```cpp
// 所有流源的基类
class MediaSource {
public:
    virtual std::string id() const = 0;
    void add_subscriber(std::weak_ptr<IMediaSink> sink);
    void remove_subscriber(IMediaSink *sink);
protected:
    void dispatch_frame(const MediaFrame &frame);  // 1:N 广播
};

// 全局流管理中心
class MediaHub {
public:
    static MediaHub &instance();
    void add_source(std::shared_ptr<MediaSource> source);
    std::shared_ptr<MediaSource> find(const std::string &id) const;
};
```

**Week 4 验收：** 按需拉流 + 自动重连 + REST API 增删查 + 多客户端同时观看

#### Week 5：首次可用版本 v0.1

- GOP 缓存实现"秒开"
- 多线程架构优化（strand 串行化）
- 全面功能/异常测试
- 性能指标监控（码率、帧率、丢包率）
- 发布 v0.1: RTSP 代理 + 按需拉流 + 断线重连 + REST API

#### Week 6-7：FFmpeg 转码集成

```
学习顺序：
FFmpeg 解码 → 从 RTP 流解码 → H.265 编码 → 缩放器 → 异步转码管线 → 硬件加速 → 按需转码
```

**转码管线数据流：**

```
MediaSource ─帧─→ [解码队列] ─YUV─→ [缩放] ─YUV─→ [编码队列] ─NAL─→ TranscodedSource
```

**Week 6-7 验收：** v0.2 发布 -- RTSP 代理 + H.264/H.265 按需转码 + 截图 API

---

### 阶段三：协议扩展 (Week 8-12)

> 目标：实现 GB28181 国标接入 + WebRTC 浏览器播放

#### Week 8-10：GB28181 国标协议

```
学习顺序（分 3 周）：
Week 8: SIP 协议学习 → SIP 消息解析 → 设备注册 → 心跳/目录查询
Week 9: PS 流解封装 → INVITE 视频点播 → BYE → Gb28181MediaSource
Week 10: PTZ 云台控制 → 协议互转 → 异常处理 → v0.3 发布
```

**GB28181 核心交互流程：**

```
[摄像头] ──REGISTER──→ [CamStreamKit SipServer]
         ←──200 OK────
         ──Keepalive──→ (每60s)

[用户] ──curl POST──→ [CamStreamKit] ──INVITE──→ [摄像头]
                                      ←──200 OK──
                                      ──ACK──→
[摄像头] ──RTP(PS)──→ [CamStreamKit] ──解封装──→ [MediaHub] ──RTSP──→ [VLC]
```

**关键难点：**
- PS 流解封装：Pack Header (00 00 01 BA) → System Header → PES → H.264 NAL
- SIP Digest 认证（与 RTSP 认证逻辑复用）
- RTP 端口池管理

#### Week 11-12：WebRTC 集成

```
学习顺序：
WebRTC 协议栈概念 → SDP/ICE 细节 → libdatachannel 集成 → WHEP 端点 →
RTP 转封装 → NACK 重传 → 多观看者管理 → Web UI → v0.4 发布
```

**WHEP (WebRTC-HTTP Egress Protocol) 流程：**

```
浏览器:  POST /api/v1/whep/{stream_id}  Body: SDP Offer
服务端:  201 Created                     Body: SDP Answer
         → 建立 ICE 连接 → 开始推送 RTP(SRTP) 数据
```

**Week 11-12 验收：** v0.4 -- RTSP + 转码 + GB28181 + WebRTC + Web UI 全功能可用

---

### 阶段四：工程完善 (Week 13-16)

> 目标：确保生产可用的稳定性、性能、文档

#### Week 13：稳定性与性能

| 任务 | 工具/方法 | 验收标准 |
|------|----------|---------|
| 内存安全 | ASan (AddressSanitizer) | 运行 10 分钟无报告 |
| 数据竞争 | TSan (ThreadSanitizer) | 无竞争报告 |
| 未定义行为 | UBSan | 无报告 |
| 长时间运行 | 24h 压力测试脚本 | 内存稳定不增长 |
| 性能优化 | perf profiling + 内存池 | 满足基线指标 |

**性能基线目标：**

```
环境: 4 核 8GB, 1Gbps 网络
┌─────────────────────┬──────────┬──────────┬──────────┐
│ 场景                │ CPU      │ 内存     │ 延迟     │
├─────────────────────┼──────────┼──────────┼──────────┤
│ 1路 H264 1080p      │ < 5%     │ < 30MB   │ < 200ms  │
│ 10路 各5客户端      │ < 30%    │ < 100MB  │ < 300ms  │
│ 1路转码 H265 720p   │ < 15%    │ < 80MB   │ < 500ms  │
│ 50路 (压测极限)     │ < 80%    │ < 500MB  │ < 1000ms │
└─────────────────────┴──────────┴──────────┴──────────┘
```

#### Week 14：运维功能

- Prometheus 指标导出 (`/metrics`)
- 日志分级 + 文件轮转 (spdlog rotating_file_sink)
- 信号处理 + 优雅关闭 (SIGTERM/SIGINT)
- Docker 多阶段构建 + docker-compose

#### Week 15：文档与测试

- README (架构图 + Quick Start + API 文档)
- Doxygen 代码文档
- 集成测试套件（自动化端到端测试）
- 演示视频录制

#### Week 16：开源发布

- GitHub Release v1.0.0
- Docker Hub 推送
- 技术博客 2-3 篇
- 简历项目描述 + 面试深挖问题准备

---

## 三、实现指南：分模块详解

### 3.1 模块依赖关系

```
                    ┌────────────────────────────────────────┐
                    │            应用层 (main)                │
                    └────────┬──────────┬──────────┬─────────┘
                             │          │          │
              ┌──────────────┤          │          ├──────────────┐
              ▼              ▼          ▼          ▼              ▼
       ┌────────────┐ ┌───────────┐ ┌────────┐ ┌─────────┐ ┌──────────┐
       │ RtspServer │ │ SipServer │ │ WebRTC │ │ApiServer│ │Transcoder│
       └─────┬──────┘ └─────┬─────┘ └───┬────┘ └────┬────┘ └────┬─────┘
             │               │           │           │            │
             └───────────────┴─────┬─────┴───────────┘            │
                                   ▼                              │
                            ┌────────────┐                        │
                            │  MediaHub  │◄───────────────────────┘
                            └──────┬─────┘
                                   │
                    ┌──────────────┬┴──────────────┐
                    ▼              ▼               ▼
             ┌────────────┐ ┌───────────┐  ┌────────────────┐
             │RtspClient  │ │Gb28181Src │  │TranscodedSource│
             │MediaSource │ │MediaSource│  │                │
             └──────┬─────┘ └─────┬─────┘  └────────────────┘
                    │             │
                    └──────┬──────┘
                           ▼
                  ┌──────────────────┐
                  │  基础设施层       │
                  │  Asio / Buffer   │
                  │  Logger / Config │
                  │  ThreadPool      │
                  └──────────────────┘
```

### 3.2 实现顺序（从底向上）

#### 第一层：基础设施（Week 1）

| 模块 | 文件 | 核心职责 |
|------|------|---------|
| ThreadPool | `src/core/thread_pool.h` | 固定线程数 + 任务队列 |
| Buffer | `src/core/buffer.h` | 引用计数字节缓冲区（零拷贝分发） |
| Logger | `src/core/logger.h` | spdlog 封装，项目日志宏 |
| Config | `src/core/config.h` | JSON 配置文件解析 |
| Timer | `src/core/timer.h` | Asio 定时器封装 |
| TcpServer | `src/net/tcp_server.h` | 通用 TCP 监听 + Session 工厂 |
| UdpSocket | `src/net/udp_socket.h` | 异步 UDP 收发 |

#### 第二层：协议解析（Week 2）

| 模块 | 文件 | 核心职责 |
|------|------|---------|
| BufferReader/Writer | `src/core/buffer_rw.h` | 网络字节序二进制读写 |
| RtpPacket | `src/rtp/rtp_packet.h` | RTP 头解析/构建 |
| H264Parser | `src/codec/h264_parser.h` | NAL 分离、FU-A 重组 |
| H265Parser | `src/codec/h265_parser.h` | H.265 NAL 解析 |
| SdpParser | `src/sdp/sdp_parser.h` | SDP 解析/生成 |
| RtspParser | `src/rtsp/rtsp_parser.h` | RTSP 消息解析 |

#### 第三层：核心业务（Week 3-5）

| 模块 | 文件 | 核心职责 |
|------|------|---------|
| MediaFrame | `src/core/media_frame.h` | 统一媒体帧结构体 |
| MediaSource | `src/core/media_source.h` | 流源基类 + 订阅者管理 |
| IMediaSink | `src/core/media_sink.h` | 消费者接口 |
| MediaHub | `src/core/media_hub.h` | 全局流注册/查找/管理 |
| GopCache | `src/core/gop_cache.h` | GOP 缓存秒开 |
| RtspClient | `src/rtsp/rtsp_client.h` | 拉流客户端 |
| RtspServer | `src/rtsp/rtsp_server.h` | 对外提供 RTSP 流 |
| RtpPacketizer | `src/rtp/rtp_packetizer.h` | NAL → RTP 打包 |
| ApiServer | `src/http/api_server.h` | RESTful API |

#### 第四层：扩展功能（Week 6-12）

| 模块 | 文件 | 核心职责 |
|------|------|---------|
| H264Decoder | `src/transcode/h264_decoder.h` | FFmpeg H.264 解码 |
| H265Encoder | `src/transcode/h265_encoder.h` | FFmpeg H.265 编码 |
| VideoScaler | `src/transcode/video_scaler.h` | 分辨率缩放 |
| Transcoder | `src/transcode/transcoder.h` | 异步转码管线 |
| SipParser | `src/gb28181/sip_parser.h` | SIP 消息解析/构建 |
| SipServer | `src/gb28181/sip_server.h` | GB28181 信令核心 |
| PsDepacketizer | `src/gb28181/ps_depacketizer.h` | PS 流解封装 |
| PtzCommand | `src/gb28181/ptz_command.h` | 云台控制指令 |
| WebRtcSession | `src/webrtc/webrtc_session.h` | PeerConnection 管理 |
| WebRtcManager | `src/webrtc/webrtc_manager.h` | 多会话生命周期 |

### 3.3 项目目录结构

```
CamStreamKit/
├── CMakeLists.txt
├── cmake/
│   └── dependencies.cmake          # FetchContent 管理依赖
├── include/camstreamkit/
│   └── version.h                   # 公共头文件
├── src/
│   ├── CMakeLists.txt
│   ├── core/                       # 基础设施
│   │   ├── buffer.h / buffer.cpp
│   │   ├── media_frame.h
│   │   ├── media_source.h / media_source.cpp
│   │   ├── media_hub.h / media_hub.cpp
│   │   ├── gop_cache.h / gop_cache.cpp
│   │   ├── thread_pool.h / thread_pool.cpp
│   │   ├── config.h / config.cpp
│   │   └── logger.h
│   ├── net/                        # 网络基础
│   │   ├── tcp_server.h / tcp_server.cpp
│   │   └── udp_socket.h / udp_socket.cpp
│   ├── rtp/                        # RTP 协议
│   │   ├── rtp_packet.h / rtp_packet.cpp
│   │   └── rtp_packetizer.h / rtp_packetizer.cpp
│   ├── codec/                      # 编码解析
│   │   ├── h264_parser.h / h264_parser.cpp
│   │   └── h265_parser.h / h265_parser.cpp
│   ├── sdp/                        # SDP 协议
│   │   └── sdp_parser.h / sdp_parser.cpp
│   ├── rtsp/                       # RTSP 协议
│   │   ├── rtsp_parser.h / rtsp_parser.cpp
│   │   ├── rtsp_client.h / rtsp_client.cpp
│   │   └── rtsp_server.h / rtsp_server.cpp
│   ├── transcode/                  # 转码模块
│   │   ├── h264_decoder.h / h264_decoder.cpp
│   │   ├── h265_encoder.h / h265_encoder.cpp
│   │   ├── video_scaler.h / video_scaler.cpp
│   │   └── transcoder.h / transcoder.cpp
│   ├── gb28181/                    # GB28181 国标
│   │   ├── sip_parser.h / sip_parser.cpp
│   │   ├── sip_server.h / sip_server.cpp
│   │   ├── ps_depacketizer.h / ps_depacketizer.cpp
│   │   ├── ps_packetizer.h / ps_packetizer.cpp
│   │   └── ptz_command.h / ptz_command.cpp
│   ├── webrtc/                     # WebRTC
│   │   ├── webrtc_session.h / webrtc_session.cpp
│   │   └── webrtc_manager.h / webrtc_manager.cpp
│   ├── http/                       # HTTP/REST API
│   │   └── api_server.h / api_server.cpp
│   └── main.cpp                    # 入口
├── tests/
│   ├── CMakeLists.txt
│   ├── unit/                       # 单元测试
│   └── integration/                # 集成测试
├── examples/
│   ├── simple_proxy.cpp
│   ├── transcode_proxy.cpp
│   ├── gb28181_proxy.cpp
│   └── web/                        # Web UI
│       └── index.html
├── config.json                     # 配置文件模板
├── Dockerfile
├── docker-compose.yml
├── .github/workflows/ci.yml
├── .clang-format
├── Doxyfile
├── CHANGELOG.md
├── LICENSE
└── README.md
```

---

## 四、流稳定性保障策略

### 4.1 断线自动重连（指数退避）

**问题：** 摄像头可能因网络抖动、设备重启等原因断开连接

**方案：**

```cpp
class ReconnectPolicy {
public:
    ReconnectPolicy(int base_ms = 1000, int max_ms = 30000);
    int next_delay_ms();  // 1s → 2s → 4s → 8s → ... → 30s (封顶)
    void reset();         // 连接成功后重置为 1s
};
```

**重连循环逻辑：**

```cpp
asio::awaitable<void> RtspMediaSource::pull_loop() {
    while (!stopped_) {
        auto ok = co_await client_.connect();
        if (ok) {
            reconnect_.reset();
            status_ = "online";
            co_await client_.play();  // 阻塞直到连接断开
            status_ = "reconnecting";
        }
        auto delay = reconnect_.next_delay_ms();
        asio::steady_timer timer(io_);
        timer.expires_after(std::chrono::milliseconds(delay));
        co_await timer.async_wait(asio::use_awaitable);
    }
}
```

**关键设计：**
- 成功连接后立即 `reset()` 回 1 秒
- 封顶 30 秒避免等待时间无限增长
- 每次重连记录日志便于排查
- 重连状态通过 API 暴露给外部监控

### 4.2 GOP 缓存（新客户端秒开）

**问题：** 新客户端连接后需要等到下一个 I 帧才能解码，延迟 2-5 秒

**方案：**

```cpp
class GopCache {
public:
    void on_frame(const MediaFrame &frame) {
        std::lock_guard lock(mtx_);
        if (frame.type == MediaFrame::VIDEO_KEY) {
            frames_.clear();  // 新 I 帧，清除旧 GOP
        }
        frames_.push_back(frame);
    }

    std::vector<MediaFrame> get_cached_gop() const {
        std::lock_guard lock(mtx_);
        return frames_;
    }
};
```

**新客户端连接时立即发送缓存 GOP：**

```cpp
void MediaSource::add_subscriber(std::weak_ptr<IMediaSink> sink) {
    // ... 添加订阅者
    if (auto sp = sink.lock()) {
        for (auto &frame : gop_cache_.get_cached_gop()) {
            sp->on_media_frame(frame);
        }
    }
}
```

**效果：** 新客户端出画面时间 < 500ms

### 4.3 按需拉流 + 空闲超时

**问题：** 不应该一直拉取所有摄像头的流，浪费带宽和资源

**方案：**

```
第一个观看者连接 → 触发 start_pull() → 开始拉流
最后一个观看者离开 → 启动 10 秒定时器
    ├─ 10 秒内有新观看者 → 取消定时器
    └─ 10 秒后无人 → stop_pull() → 停止拉流
```

**关键参数：**
- 空闲等待时间：10 秒（可配置），避免频繁连断
- 拉流触发：同步触发（第一个 subscriber 会等待连接建立）

### 4.4 健康监测 + 指标导出

**实时监控指标：**

```cpp
struct StreamStats {
    std::atomic<uint64_t> bytes_received{0};
    std::atomic<uint64_t> frames_received{0};
    std::atomic<uint32_t> current_bitrate_kbps{0};
    std::atomic<uint32_t> current_fps{0};
    std::atomic<uint64_t> rtp_packets_sent{0};
    std::atomic<uint64_t> rtp_packets_lost{0};
};
```

**Prometheus 指标端点 (`GET /metrics`)：**

```
csk_stream_bitrate_kbps{id="cam1"} 4096
csk_stream_fps{id="cam1"} 25
csk_stream_subscribers{id="cam1"} 3
csk_rtsp_sessions_total 8
csk_webrtc_sessions_total 2
csk_gb28181_devices_online 5
```

**告警条件：**
- 码率突变 > 50% → 可能存在网络问题
- 帧率降为 0 → 流已断开
- 内存持续增长 → 可能存在泄漏

### 4.5 异常处理策略

| 异常场景 | 处理方式 |
|---------|---------|
| 摄像头断线 | 指数退避重连，状态切为 "reconnecting" |
| 客户端突然断开 | Session 析构时自动 unsubscribe，weak_ptr 自然失效 |
| RTP 丢包 (UDP) | WebRTC: NACK 重传；RTSP UDP: 客户端 RTCP RR 反馈 |
| 内存泄漏 | ASan 检测 + shared_ptr/weak_ptr 打破循环引用 |
| 端口耗尽 | RtpPortPool 统一管理，用完释放 |
| 转码队列堆积 | 队列满时丢弃最老帧（保持实时性） |
| 设备离线 (GB28181) | 心跳超时 3 个周期 → 标记 offline → 清理关联会话 |
| INVITE 无响应 | 10 秒超时 → 放弃 → 返回错误 |

### 4.6 内存安全保障

**编译期检测：**

```cmake
# ASan (内存越界 + use-after-free + 泄漏)
target_compile_options(camstreamkit PRIVATE -fsanitize=address -fno-omit-frame-pointer)

# TSan (数据竞争)
target_compile_options(camstreamkit PRIVATE -fsanitize=thread)

# UBSan (未定义行为)
target_compile_options(camstreamkit PRIVATE -fsanitize=undefined)
```

**压力测试：**

```bash
# 10 路摄像头 × 5 客户端，运行 24 小时
python test/stress/run_stress.py --cameras 10 --clients-per-camera 5 --duration 24h
```

**验收标准：**
- 连续运行 24 小时，内存波动 < 10%
- 无崩溃、无 ASan/TSan 报告
- 所有流正常播放

### 4.7 零拷贝数据分发

**问题：** 1 路摄像头可能有 N 个观看者，如果每次都复制帧数据，内存开销巨大

**方案：** 使用 `shared_ptr<Buffer>` 实现引用计数的零拷贝：

```cpp
struct MediaFrame {
    std::shared_ptr<Buffer> data;  // 多个 subscriber 共享同一份数据
    // ...
};

void MediaSource::dispatch_frame(const MediaFrame &frame) {
    for (auto &subscriber : subscribers_) {
        // frame.data 是 shared_ptr，这里只增加引用计数，不复制数据
        subscriber->on_media_frame(frame);
    }
    // 每个 subscriber 独立进行 RTP 打包（只写 RTP 头部，不复制 payload）
}
```

---

## 五、关键依赖库

| 库 | 版本建议 | 用途 | 引入方式 |
|---|---------|------|---------|
| standalone Asio | latest | 异步网络 I/O | FetchContent |
| spdlog | >= 1.12 | 日志 | FetchContent |
| nlohmann/json | >= 3.11 | JSON 解析 | FetchContent |
| Catch2 | v3 | 单元测试 | FetchContent |
| FFmpeg (libav*) | >= 5.0 | 转码 | 系统 pkg-config |
| libdatachannel | >= 0.21 | WebRTC | FetchContent |
| pugixml | >= 1.13 | XML 解析 (GB28181) | FetchContent |

---

## 六、推荐学习资源

### 协议规范
- RFC 2326 (RTSP) / RFC 3550 (RTP) / RFC 6184 (H.264 RTP Payload)
- GB/T 28181-2022 国标文档（重点第 5-7 章）
- WHEP Draft Spec (WebRTC-HTTP Egress Protocol)

### 代码参考
- ZLMediaKit 源码 -- 作为架构参考（不照搬）
- libdatachannel 文档与示例
- FFmpeg 官方 API 文档: https://ffmpeg.org/doxygen/trunk/

### 书籍
- "C++ Concurrency in Action" -- 多线程必读
- "TCP/IP Illustrated Vol.1" -- 网络基础
- "Video Encoding by the Numbers" -- 视频编码入门

### 工具
- Wireshark -- 抓包分析 RTP/RTSP/SIP
- FFmpeg/FFplay/FFprobe -- 模拟源/播放器/流分析
- VLC -- 通用流媒体播放器
- Chrome WebRTC Internals (`chrome://webrtc-internals/`)

---

## 七、每日时间建议

- **工作日**: 2.5-3h（理论学习 30-45min + 编码实践 90-120min）
- **周末**: 4-5h（综合项目集成 + 端到端测试）
- **总投入**: 约 350 小时 / 16 周

## 八、版本发布里程碑

| 版本 | 周数 | 功能 |
|------|------|------|
| v0.1 | Week 5 | RTSP 代理 + 按需拉流 + 断线重连 + REST API + 秒开 |
| v0.2 | Week 7 | + H.264/H.265 转码 + 硬件加速 + 截图 API |
| v0.3 | Week 10 | + GB28181 信令/媒体/PTZ 控制 |
| v0.4 | Week 12 | + WebRTC WHEP 播放 + Web UI |
| v1.0 | Week 16 | + Docker + 监控 + 文档 + 开源发布 |
