---
name: C++ streaming media library
overview: A 16-week plan to build a lightweight, modern C++17 camera streaming proxy library (CamStreamKit) supporting RTSP/GB28181/WebRTC with transcoding -- addressing the gap between overly complex frameworks like ZLMediaKit and toy-level RTSP servers.
todos:
  - id: phase1
    content: "Phase 1 (Week 1-2): Modern C++ system programming -- async I/O, threading, RTP/RTSP protocol parsing"
    status: pending
  - id: phase2
    content: "Phase 2 (Week 3-5): RTSP Server + MediaHub core routing -- camera proxy with on-demand pull, reconnection, REST API"
    status: pending
  - id: phase3
    content: "Phase 3 (Week 6-7): FFmpeg transcoding integration -- H.264/H.265 transcode, hardware acceleration"
    status: pending
  - id: phase4
    content: "Phase 4 (Week 8-10): GB28181 protocol -- SIP signaling, PS stream demux, PTZ control, protocol interconversion"
    status: pending
  - id: phase5
    content: "Phase 5 (Week 11-12): WebRTC integration via libdatachannel -- WHEP, browser playback, multi-viewer"
    status: pending
  - id: phase6
    content: "Phase 6 (Week 13-16): Stability, performance, documentation, open-source release v1.0"
    status: pending
isProject: false
---


# C++ 流媒体代理库从零到发布 -- 16 周实战计划

---

## 你的起点与目标

**起点:** 能读懂和修改 IRtekNetSDK 代码，熟悉 C/C++ 基本语法，但没独立从零构建过项目
**目标:** 独立完成一个轻量级摄像头流媒体代理库，精通系统级 C++ 编程

## 最终项目: CamStreamKit -- 轻量级摄像头流媒体代理库

### 为什么做这个

**ZLMediaKit 的痛点（被诟病的地方）:**
- **太重**: 10W+ 行代码，功能全但学习曲线陡峭，很多场景只需要摄像头代理功能
- **高并发 UDP 丢包**: 国标 RTP 超过 120 路时出现发送端丢包 (GitHub issue #4549)
- **WebRTC 客户端模式不成熟**: 不支持 TCP 模式，稳定性待验证
- **GB28181 不内置**: 需要配合 WVP 等外部信令服务器
- **C++11 风格**: 部分代码模式在 2026 年看已经过时
- **商业化趋势**: 核心功能逐步转向收费的 Pro 版本

**CamStreamKit 的定位 -- 做 ZLMediaKit 的 "轻量替代":**

```
┌──────────────────────────────────────────────────┐
│              CamStreamKit 架构                    │
│                                                  │
│  ┌─────────┐  ┌─────────┐  ┌─────────────────┐  │
│  │ RTSP    │  │ GB28181 │  │ WebRTC (WHEP)   │  │
│  │ Client  │  │ SIP+RTP │  │ via libdatach.  │  │
│  └────┬────┘  └────┬────┘  └───────┬─────────┘  │
│       │            │               │             │
│  ┌────v────────────v───────────────v──────────┐  │
│  │          MediaHub (核心路由)                 │  │
│  │  - 流注册/注销                              │  │
│  │  - 按需拉流 (on-demand pull)               │  │
│  │  - 多路分发 (1:N fanout)                   │  │
│  │  - 无人观看自动断开                         │  │
│  └──────────────────┬────────────────────────┘  │
│                     │                            │
│  ┌──────────────────v────────────────────────┐  │
│  │          Transcoder (可选)                  │  │
│  │  - FFmpeg/libav 集成                       │  │
│  │  - H.264 <-> H.265 转码                   │  │
│  │  - 分辨率/帧率调整                          │  │
│  └──────────────────┬────────────────────────┘  │
│                     │                            │
│  ┌──────────────────v────────────────────────┐  │
│  │          输出协议层                          │  │
│  │  RTSP Server | WebRTC(WHEP) | HTTP-FLV    │  │
│  └───────────────────────────────────────────┘  │
│                                                  │
│  ┌───────────────────────────────────────────┐  │
│  │  基础设施: Async I/O (asio) | 线程池       │  │
│  │  日志 | 配置 | RESTful API | 指标监控      │  │
│  └───────────────────────────────────────────┘  │
└──────────────────────────────────────────────────┘
```

**核心差异化:**
- 纯 C++17，代码量控制在 1.5-2W 行以内
- 专注摄像头代理场景，不做通用流媒体服务器
- 摄像头生命周期管理（自动重连、健康监测、按需拉流）
- 内置 GB28181 信令（不依赖外部 WVP）
- 清晰的分层架构 + 完整的文档和测试

**覆盖的市场热点技术:**
- 现代 C++17 系统编程
- 异步网络 I/O (Boost.Asio / standalone asio)
- 流媒体协议栈 (RTSP/RTP/RTCP/SDP)
- 视频编码 (H.264/H.265 NAL 解析)
- FFmpeg/libav 集成
- GB28181 国标 (SIP + PS 封装)
- WebRTC (通过 libdatachannel)
- RESTful API 设计

---

## 阶段一: 现代 C++ 系统编程强化 (第 1-2 周 / Day 1-14)

> 目标: 补齐独立写项目所需的 C++ 工程能力 -- 不重复学基础语法，聚焦你欠缺的部分

### 第 1 周: 项目骨架 + 网络编程基础 (Day 1-7)

#### Day 1 -- 项目初始化 + CMake 工程搭建 (2.5h)

**理论 (30min)**
- CMake 现代用法: `target_xxx` 命令族、FetchContent 自动下载依赖
- 项目结构最佳实践: `include/` (公共头) / `src/` (实现) / `tests/` / `examples/`

**实践 (2h)**
1. 创建 GitHub 仓库 `CamStreamKit`
2. 搭建项目骨架:
   ```
   CamStreamKit/
   ├── CMakeLists.txt
   ├── cmake/
   │   └── dependencies.cmake     # FetchContent 管理依赖
   ├── include/camstreamkit/
   │   └── version.h
   ├── src/
   │   ├── CMakeLists.txt
   │   └── core/
   ├── tests/
   │   └── CMakeLists.txt
   ├── examples/
   ├── .github/workflows/ci.yml
   ├── .clang-format              # 代码格式化配置
   └── README.md
   ```
3. 引入第一批依赖: spdlog (日志)、Catch2 (测试)
4. 确保 `cmake -B build && cmake --build build && ctest --test-dir build` 跑通

**验收:** CI 绿色，能编译空项目并运行一个 hello-world 测试

---

#### Day 2 -- RAII 资源管理 + 智能指针实战 (2.5h)

**理论 (40min)**
- 复习: `unique_ptr` (独占) / `shared_ptr` (共享) / `weak_ptr` (弱引用打破循环)
- 关键模式: RAII 管理 socket fd / FILE* / FFmpeg 资源 (AVCodecContext 等)
- `std::shared_ptr` 的自定义删除器: `shared_ptr<AVCodecContext>(ctx, [](auto *p) { avcodec_free_context(&p); })`

**实践 (90min)**
1. 实现 `Socket` RAII 封装类 (封装 socket 创建/关闭)
2. 实现 `FileHandle` (复用你的知识，快速过)
3. 为将来的 FFmpeg 资源写 RAII wrapper 框架:
   ```cpp
   template<typename T, auto Deleter>
   struct AvPtr {
       struct D { void operator()(T *p) const { if (p) Deleter(&p); } };
       using type = std::unique_ptr<T, D>;
   };
   using AvFormatCtx = AvPtr<AVFormatContext, avformat_close_input>::type;
   ```
4. 编写测试验证资源正确释放

**验收:** Socket 析构时自动 close，FFmpeg RAII wrapper 编译通过

---

#### Day 3 -- 多线程 + 线程池 (2.5h)

**理论 (30min)**
- `std::thread` / `std::mutex` / `std::condition_variable` / `std::atomic`
- 线程池概念: 固定数量线程 + 任务队列 + 条件变量通知

**实践 (2h)**
1. 实现一个轻量级线程池 `ThreadPool`:
   ```cpp
   class ThreadPool {
   public:
       explicit ThreadPool(size_t threads = std::thread::hardware_concurrency());
       ~ThreadPool();  // 等待所有任务完成
       template<typename F, typename... Args>
       auto submit(F &&f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>>;
   private:
       std::vector<std::thread> workers_;
       std::queue<std::function<void()>> tasks_;
       std::mutex mtx_;
       std::condition_variable cv_;
       std::atomic<bool> stop_{false};
   };
   ```
2. 编写测试: 提交 100 个任务，验证全部完成且无竞态

**验收:** 线程池能正确并发执行任务，析构时优雅等待所有任务完成

---

#### Day 4 -- Boost.Asio 异步 I/O 入门 (2.5h)

**理论 (45min)**
- 为什么用 Asio: 跨平台异步网络框架，Linux 底层用 epoll，Windows 用 IOCP
- 核心概念: `io_context` (事件循环)、`strand` (串行化)、`async_read/write`
- 协程: C++20 coroutine + Asio 使异步代码像同步一样写 (`co_await`)
- 用 standalone Asio (不需要整个 Boost): `asio::ip::tcp::socket`

**实践 (90min)**
1. 用 FetchContent 引入 standalone Asio
2. 写一个 TCP echo server:
   ```cpp
   asio::co_spawn(io, [](asio::ip::tcp::socket sock) -> asio::awaitable<void> {
       char buf[1024];
       for (;;) {
           auto n = co_await sock.async_read_some(asio::buffer(buf), asio::use_awaitable);
           co_await asio::async_write(sock, asio::buffer(buf, n), asio::use_awaitable);
       }
   }, asio::detached);
   ```
3. 用 `telnet localhost 9000` 测试 echo
4. 改为多连接并发处理

**验收:** Echo server 能处理多个并发 TCP 连接，每个连接独立收发

---

#### Day 5 -- TCP 服务器框架 (2.5h)

**实践 (2.5h)**
1. 抽象出 `TcpServer` 类:
   ```cpp
   class TcpServer {
   public:
       using SessionFactory = std::function<std::shared_ptr<Session>(asio::ip::tcp::socket)>;
       TcpServer(asio::io_context &io, uint16_t port, SessionFactory factory);
       void start();
       void stop();
   };
   class Session : public std::enable_shared_from_this<Session> {
   public:
       virtual void start() = 0;
       virtual void on_data(std::span<const uint8_t> data) = 0;
   };
   ```
2. 实现连接管理: 新连接创建 Session，断开时自动清理 (weak_ptr 跟踪)
3. 实现 `HttpSession` -- 解析 HTTP 请求行 + 返回简单响应 (为后续 REST API 铺路)
4. 测试: `curl http://localhost:8080/api/version` 返回 JSON

**验收:** HTTP 服务器能响应 GET 请求，返回 JSON

---

#### Day 6 -- UDP 收发 + 定时器 (2.5h)

**实践 (2.5h)**
1. 实现 `UdpSocket` 封装:
   ```cpp
   class UdpSocket {
   public:
       UdpSocket(asio::io_context &io, uint16_t port);
       void async_receive(std::function<void(std::span<const uint8_t>, asio::ip::udp::endpoint)> cb);
       void send_to(std::span<const uint8_t> data, const asio::ip::udp::endpoint &ep);
   };
   ```
2. Asio 定时器: `asio::steady_timer` -- 实现心跳/超时机制
3. 写一个 "UDP ping-pong" 测试
4. 实现 `Timer` 工具类: 支持一次性定时和周期性定时

**验收:** UDP 收发正常，定时器精度在 10ms 以内

---

#### Day 7 -- 周末: 日志 + 配置 + 核心工具库 (4h)

**实践 (4h)**
1. 集成 spdlog -- 封装项目日志宏:
   ```cpp
   #define CSK_LOG_D(...) SPDLOG_DEBUG(__VA_ARGS__)
   #define CSK_LOG_I(...) SPDLOG_INFO(__VA_ARGS__)
   #define CSK_LOG_W(...) SPDLOG_WARN(__VA_ARGS__)
   #define CSK_LOG_E(...) SPDLOG_ERROR(__VA_ARGS__)
   ```
2. 实现 JSON 配置文件解析 (用 nlohmann/json):
   ```json
   {
     "server": { "http_port": 8080, "rtsp_port": 554 },
     "camera": { "reconnect_interval_ms": 5000, "timeout_ms": 10000 }
   }
   ```
3. 实现工具类:
   - `Buffer`: 引用计数的字节缓冲区（用于零拷贝转发）
   - `Url`: RTSP URL 解析 (`rtsp://user:pass@host:port/path`)
   - `TimeUtil`: 毫秒时间戳、NTP 时间戳
4. 编写单元测试

**验收:** 所有工具类测试通过，日志输出格式美观

---

### 第 2 周: 字节流处理 + RTP 基础 (Day 8-14)

#### Day 8 -- 二进制协议解析框架 (2.5h)

**实践 (2.5h)**
1. 实现 `BufferReader` / `BufferWriter`:
   ```cpp
   class BufferReader {
   public:
       explicit BufferReader(std::span<const uint8_t> data);
       uint8_t read_u8();
       uint16_t read_u16_be();   // 网络字节序
       uint32_t read_u32_be();
       std::span<const uint8_t> read_bytes(size_t n);
       size_t remaining() const;
   };
   ```
2. 测试: 构造一个 RTP 头部字节，用 BufferReader 解析各字段

---

#### Day 9 -- RTP/RTCP 协议解析 (2.5h)

**理论 (40min)**
- RTP 头部格式 (RFC 3550): V(2) P(1) X(1) CC(4) M(1) PT(7) SeqNum(16) Timestamp(32) SSRC(32)
- RTCP: SR (发送者报告) / RR (接收者报告) -- 用于统计丢包率和延迟

**实践 (90min)**
1. 实现 `RtpPacket` 解析和构建:
   ```cpp
   struct RtpHeader {
       uint8_t version;    // 2
       bool padding;
       bool extension;
       uint8_t csrc_count;
       bool marker;
       uint8_t payload_type;
       uint16_t sequence;
       uint32_t timestamp;
       uint32_t ssrc;
   };
   std::optional<RtpHeader> parse_rtp_header(std::span<const uint8_t> data);
   ```
2. 实现 RTCP SR 包的解析
3. 测试: 用 Wireshark 抓取的真实 RTP 数据包验证解析

---

#### Day 10 -- H.264 码流基础 (2.5h)

**理论 (45min)**
- H.264 NAL 单元: `[StartCode 00 00 00 01] [NAL Header] [RBSP]`
- NAL 类型: SPS (7), PPS (8), IDR (5), Non-IDR (1)
- RTP 封装 H.264 (RFC 6184): Single NAL / STAP-A / FU-A 三种模式
- SPS/PPS 是解码的前提，必须先于 IDR 帧发送

**实践 (90min)**
1. 实现 H.264 NAL 解析器:
   ```cpp
   struct NalUnit {
       uint8_t type;       // 低 5 位
       uint8_t ref_idc;    // bit 5-6
       std::span<const uint8_t> data;  // 包含 NAL header
   };
   class H264Parser {
   public:
       // 从字节流中提取 NAL 单元 (Annex-B 格式，00 00 00 01 分隔)
       std::vector<NalUnit> parse(std::span<const uint8_t> data);
       // 从 RTP 包中提取 NAL 单元 (处理 FU-A 分片重组)
       std::optional<NalUnit> depacketize_rtp(const RtpPacket &pkt);
   };
   ```
2. 测试: 解析一个 H.264 测试文件，统计 SPS/PPS/IDR/P 帧数量

---

#### Day 11 -- H.265 码流 + SDP 解析 (2.5h)

**实践 (2.5h)**
1. H.265 NAL 类型: VPS (32), SPS (33), PPS (34), IDR_W_RADL (19), TRAIL_R (1)
2. 实现 `H265Parser`（与 H264 类似，NAL header 是 2 字节）
3. 实现 SDP (Session Description Protocol) 解析器:
   ```cpp
   struct SdpMedia {
       std::string type;        // "video" / "audio"
       uint16_t port;
       std::string protocol;    // "RTP/AVP"
       uint8_t payload_type;
       std::string codec;       // "H264" / "H265"
       uint32_t clock_rate;     // 90000
       std::string sprop_params; // base64 SPS/PPS
   };
   class SdpParser {
   public:
       std::vector<SdpMedia> parse(const std::string &sdp);
       std::string build(const std::vector<SdpMedia> &media);
   };
   ```
4. 测试: 解析真实摄像头的 SDP 响应

---

#### Day 12-13 -- RTSP 协议解析器 (5h)

**理论 (1h)**
- RTSP (RFC 2326): 类似 HTTP 的文本协议，控制媒体流的播放
- 核心方法: OPTIONS / DESCRIBE / SETUP / PLAY / TEARDOWN
- 认证: Basic / Digest (RFC 2617)
- RTP 传输模式: UDP (独立端口) / TCP interleaved (复用 RTSP 连接)

**实践 (4h)**
1. 实现 RTSP 消息解析器:
   ```cpp
   struct RtspRequest {
       std::string method;     // "DESCRIBE"
       std::string url;        // "rtsp://..."
       uint32_t cseq;
       std::map<std::string, std::string> headers;
       std::string body;       // SDP for DESCRIBE response
   };
   struct RtspResponse {
       uint16_t status_code;   // 200
       std::string reason;     // "OK"
       uint32_t cseq;
       std::map<std::string, std::string> headers;
       std::string body;
   };
   ```
2. 实现 Digest 认证计算: `HA1 = MD5(user:realm:pass)`, `HA2 = MD5(method:uri)`, `response = MD5(HA1:nonce:HA2)`
3. 实现 RTP interleaved 帧解析 (`$` + channel + length + data)
4. 测试: 手工构造 RTSP 交互序列，验证解析正确

---

#### Day 14 -- 周末: RTSP 客户端 v0.1 (4-5h)

**实践: 实现能拉取摄像头 RTSP 流的客户端**
1. `RtspClient` 类:
   ```cpp
   class RtspClient {
   public:
       struct Config {
           std::string url;        // rtsp://user:pass@192.168.1.100:554/stream1
           TransportMode transport; // UDP / TCP
           int timeout_ms = 10000;
       };
       using FrameCallback = std::function<void(const MediaFrame &frame)>;

       RtspClient(asio::io_context &io, Config config);
       void set_frame_callback(FrameCallback cb);
       asio::awaitable<bool> connect();
       asio::awaitable<void> play();
       void stop();
   };
   ```
2. 实现完整的 RTSP 握手流程: OPTIONS -> DESCRIBE -> SETUP -> PLAY
3. 接收 RTP 数据，解析 H.264 NAL 单元
4. 测试: 对一个真实摄像头（或 FFmpeg 模拟的 RTSP 源）拉流，保存为 H.264 文件

**验收:** 能从真实 RTSP 摄像头拉流并保存为可播放的 `.h264` 文件

---

## 阶段二: RTSP 服务器 + MediaHub 核心 (第 3-5 周 / Day 15-35)

### 第 3 周: RTSP Server (Day 15-21)

---

#### Day 15 -- RTSP Server 框架: 会话管理 (2.5h)

**理论 (30min)**
- RTSP Server 与 HTTP Server 的区别: RTSP 是有状态的（SETUP 建立会话后 PLAY 复用）
- 会话 (Session): 由 `Session` 头部标识，SETUP 时创建，TEARDOWN 时销毁
- 生命周期: 客户端连接 -> OPTIONS -> DESCRIBE -> SETUP -> PLAY -> (数据传输) -> TEARDOWN

**实践 (2h)**
1. 基于 Day 5 的 `TcpServer` 创建 `RtspServer`:
   ```cpp
   class RtspServer {
   public:
       RtspServer(asio::io_context &io, uint16_t port = 554);
       void start();
       void stop();
       // 注册流源: 当客户端请求 /stream/cam1 时，从哪里获取数据
       void add_source(const std::string &path, std::shared_ptr<MediaSource> source);
   private:
       TcpServer tcp_server_;
       std::map<std::string, std::shared_ptr<MediaSource>> sources_;
   };
   ```
2. 实现 `RtspSession` -- 继承 `Session`，解析 RTSP 请求并路由:
   ```cpp
   class RtspSession : public Session {
   public:
       void on_data(std::span<const uint8_t> data) override;
   private:
       void handle_options(const RtspRequest &req);
       void handle_describe(const RtspRequest &req);
       void handle_setup(const RtspRequest &req);
       void handle_play(const RtspRequest &req);
       void handle_teardown(const RtspRequest &req);
       void send_response(const RtspResponse &resp);
       std::string session_id_;  // 随机生成
       uint32_t cseq_{0};
   };
   ```
3. 先实现 `handle_options` -- 返回支持的方法列表:
   ```
   RTSP/1.0 200 OK
   CSeq: 1
   Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN
   ```
4. 测试: `ffplay rtsp://localhost:554/test` 发送 OPTIONS，服务端正确响应

**验收:** VLC/ffplay 连接后能收到 OPTIONS 200 响应

---

#### Day 16 -- RTSP Server: 请求解析管线 (2.5h)

**实践 (2.5h)**
1. 完善 RTSP 请求解析器 -- 处理不完整的 TCP 数据:
   ```cpp
   class RtspParser {
   public:
       // 喂入原始 TCP 数据，可能包含不完整的请求
       // 返回已解析完成的请求列表
       std::vector<RtspRequest> feed(std::span<const uint8_t> data);
   private:
       std::string buffer_;  // 缓存未完成的请求
       // 按 \r\n\r\n 切分完整请求
       std::optional<RtspRequest> try_parse();
   };
   ```
2. 处理 Content-Length: DESCRIBE 响应带 SDP body 时需要按长度读取
3. 处理 interleaved RTP: TCP 模式下 RTSP 连接中混合 `$` 开头的 RTP 数据
   ```
   $<1字节 channel><2字节 长度><RTP 数据>
   ```
4. 测试: 发送拼接的多个请求（一个 TCP 包包含两个 RTSP 请求），验证正确分割

**验收:** 解析器能处理 TCP 粘包/拆包情况

---

#### Day 17 -- DESCRIBE 处理: 动态生成 SDP (2.5h)

**理论 (20min)**
- DESCRIBE 请求: 客户端询问"这个 URL 有什么媒体流?"
- 响应: 200 OK + SDP body，描述视频/音频的编码格式、传输参数
- SDP 关键字段: `m=video`, `a=rtpmap:96 H264/90000`, `a=fmtp:96 sprop-parameter-sets=...`

**实践 (2h)**
1. 实现 `handle_describe`:
   ```cpp
   void RtspSession::handle_describe(const RtspRequest &req) {
       auto source = server_.find_source(req.url);
       if (!source) { send_response(404, "Not Found"); return; }

       auto sdp = source->generate_sdp(req.url);
       RtspResponse resp;
       resp.status_code = 200;
       resp.headers["Content-Type"] = "application/sdp";
       resp.headers["Content-Base"] = req.url;
       resp.body = sdp;
       send_response(resp);
   }
   ```
2. `MediaSource::generate_sdp` -- 根据流的实际编码生成 SDP:
   ```cpp
   std::string RtspMediaSource::generate_sdp(const std::string &url) {
       std::ostringstream ss;
       ss << "v=0\r\n"
          << "o=- " << session_id_ << " 1 IN IP4 0.0.0.0\r\n"
          << "s=CamStreamKit\r\n"
          << "t=0 0\r\n"
          << "m=video 0 RTP/AVP 96\r\n"
          << "a=rtpmap:96 H264/90000\r\n"
          << "a=fmtp:96 packetization-mode=1"
          << ";sprop-parameter-sets=" << base64_encode(sps_) << "," << base64_encode(pps_)
          << "\r\n"
          << "a=control:trackID=0\r\n";
       return ss.str();
   }
   ```
3. 测试: VLC 发 DESCRIBE，服务端返回正确 SDP，VLC 日志中显示解析到 H264 编码

**验收:** VLC 能解析 DESCRIBE 响应中的 SDP 并识别出 H.264 视频流

---

#### Day 18 -- SETUP + PLAY: 建立 RTP 会话 (2.5h)

**理论 (20min)**
- SETUP: 客户端告诉服务端用什么传输方式
  - UDP: `Transport: RTP/AVP;unicast;client_port=5000-5001` -> 服务端回 `server_port=6000-6001`
  - TCP: `Transport: RTP/AVP/TCP;interleaved=0-1` -> 复用 RTSP 连接
- PLAY: 客户端告诉服务端"开始发数据"
  - `Range: npt=0.000-` 表示从头开始

**实践 (2h)**
1. 实现 `handle_setup`:
   ```cpp
   void RtspSession::handle_setup(const RtspRequest &req) {
       auto transport = req.headers.at("Transport");
       if (transport.find("RTP/AVP/TCP") != std::string::npos) {
           // TCP interleaved 模式
           auto [ch_rtp, ch_rtcp] = parse_interleaved(transport); // 解析 interleaved=0-1
           rtp_channel_ = ch_rtp;
           rtcp_channel_ = ch_rtcp;
           transport_mode_ = TransportMode::TCP;
       } else {
           // UDP 模式
           auto [cp1, cp2] = parse_client_ports(transport);
           rtp_socket_ = std::make_unique<UdpSocket>(io_, 0);  // 随机端口
           rtcp_socket_ = std::make_unique<UdpSocket>(io_, 0);
           transport_mode_ = TransportMode::UDP;
       }
       session_id_ = generate_session_id();

       RtspResponse resp;
       resp.status_code = 200;
       resp.headers["Session"] = session_id_ + ";timeout=60";
       resp.headers["Transport"] = build_transport_response();
       send_response(resp);
   }
   ```
2. 实现 `handle_play`:
   ```cpp
   void RtspSession::handle_play(const RtspRequest &req) {
       auto source = server_.find_source(req.url);
       // 向 MediaSource 订阅数据
       source->add_subscriber(shared_from_this());
       send_response(200, "OK");
       // 此后 MediaSource 会通过 on_media_frame() 回调推送数据
   }
   ```
3. 实现 `on_media_frame` -- 将 MediaFrame 通过 RTP 发送给客户端

**验收:** VLC 完成 SETUP+PLAY 握手，进入等待数据状态

---

#### Day 19 -- RTP 打包器: NAL -> RTP (2.5h)

**理论 (30min)**
- H.264 RTP 打包规则 (RFC 6184):
  - NAL 小于 MTU (~1400 字节): 直接作为 Single NAL Unit 包
  - NAL 大于 MTU: 拆分为 FU-A (Fragmentation Unit) 包
    - 第一个 FU-A: Start bit = 1
    - 中间的 FU-A: Start = 0, End = 0
    - 最后一个 FU-A: End bit = 1, Marker bit = 1
  - SPS + PPS: 打包为 STAP-A (Single Time Aggregation Packet)

**实践 (2h)**
1. 实现 `RtpPacketizer`:
   ```cpp
   class RtpPacketizer {
   public:
       RtpPacketizer(uint8_t payload_type = 96, uint32_t ssrc = 0);

       // 输入一个 NAL 单元，输出一个或多个 RTP 包
       std::vector<RtpPacket> packetize(const NalUnit &nal, uint32_t timestamp);

   private:
       std::vector<RtpPacket> pack_single(const NalUnit &nal, uint32_t ts);
       std::vector<RtpPacket> pack_fu_a(const NalUnit &nal, uint32_t ts);  // 分片
       std::vector<RtpPacket> pack_stap_a(const std::vector<NalUnit> &nals, uint32_t ts);

       uint16_t seq_{0};         // 每次递增
       uint32_t ssrc_;
       uint8_t pt_;
       static constexpr size_t MAX_RTP_PAYLOAD = 1400;
   };
   ```
2. 实现 TCP interleaved 发送:
   ```cpp
   void RtspSession::send_rtp_tcp(const RtpPacket &pkt) {
       // $ + channel(1B) + length(2B, big-endian) + RTP data
       uint8_t header[4] = { '$', rtp_channel_, 0, 0 };
       uint16_t len = htons(pkt.size());
       memcpy(header + 2, &len, 2);
       async_write(header, pkt.data());
   }
   ```
3. 测试: 一个 50KB 的 IDR 帧应该被拆分成约 36 个 FU-A 包

**验收:** 生成的 RTP 包序列号连续递增，FU-A 分片 Start/End 标志正确

---

#### Day 20 -- 认证 + 超时管理 (2.5h)

**实践 (2.5h)**
1. 实现 Digest 认证:
   ```cpp
   class DigestAuth {
   public:
       DigestAuth(const std::string &realm, const std::string &nonce);
       // 验证客户端的 Authorization 头
       bool verify(const std::string &auth_header,
                   const std::string &method,
                   const std::string &uri,
                   const std::string &username,
                   const std::string &password);
       // 生成 401 响应中的 WWW-Authenticate 头
       std::string challenge() const;
   private:
       std::string md5(const std::string &input);
   };
   ```
2. 认证流程: 客户端首次请求 -> 返回 401 + nonce -> 客户端带 Authorization 重试 -> 验证通过
3. 超时管理:
   ```cpp
   // RtspSession 中添加:
   asio::steady_timer timeout_timer_;
   void reset_timeout() {
       timeout_timer_.expires_after(std::chrono::seconds(60));
       timeout_timer_.async_wait([this, self = shared_from_this()](auto ec) {
           if (!ec) { CSK_LOG_W("Session {} timed out", session_id_); close(); }
       });
   }
   ```
4. 测试: 使用带认证的 RTSP URL (`rtsp://admin:123456@localhost/stream1`)

**验收:** VLC 用错误密码连接返回 401，正确密码通过

---

#### Day 21 -- 周末: RTSP 代理端到端测试 (4-5h)

**实践: 打通完整链路**

目标: 摄像头(RTSP) -> CamStreamKit(拉流+转发) -> VLC(播放)

1. 启动 FFmpeg 模拟一个 RTSP 摄像头源:
   ```bash
   ffmpeg -re -stream_loop -1 -i test_video.mp4 \
       -c:v copy -f rtsp rtsp://localhost:8554/camera1
   ```
2. 编写 `examples/rtsp_proxy.cpp`:
   ```cpp
   int main() {
       asio::io_context io;
       Config config = Config::from_file("config.json");

       // 创建 RTSP 服务器 (对外提供流)
       RtspServer server(io, 554);

       // 创建 RTSP 客户端 (拉取摄像头流)
       RtspClient client(io, {.url = "rtsp://localhost:8554/camera1"});

       // 桥接: 客户端收到的帧 -> 服务器转发
       auto source = std::make_shared<RtspMediaSource>("cam1");
       client.set_frame_callback([&](const MediaFrame &frame) {
           source->on_frame(frame);  // 推送给所有订阅者
       });
       server.add_source("/stream/cam1", source);

       client.connect();
       server.start();
       io.run();
   }
   ```
3. VLC 打开 `rtsp://localhost:554/stream/cam1`，应看到视频画面
4. 验证:
   - [ ] 画面流畅，无花屏
   - [ ] 延迟在 500ms 以内（对比 VLC 直接拉 8554 的延迟）
   - [ ] 断开 VLC 后，客户端仍在拉流
   - [ ] 停止 FFmpeg 摄像头源后，日志显示连接断开

**验收:** VLC 能通过 CamStreamKit 代理正常观看视频

---

### 第 4 周: MediaHub 核心路由 (Day 22-28)

---

#### Day 22 -- MediaSource 抽象层 (2.5h)

**理论 (20min)**
- 流的来源可能是: RTSP 摄像头、GB28181 设备、本地文件、RTSP 推流
- 需要统一接口让 MediaHub 不关心来源类型
- 观察者模式: MediaSource 产生数据，多个 Subscriber 消费数据

**实践 (2h)**
1. 定义统一接口:
   ```cpp
   // 媒体帧 -- 从源到消费者的数据单元
   struct MediaFrame {
       enum Type { VIDEO_KEY, VIDEO_P, AUDIO };
       Type type;
       uint32_t timestamp;      // 90kHz 时基
       uint64_t dts, pts;       // 解码/显示时间戳
       std::shared_ptr<Buffer> data;  // 引用计数，零拷贝分发
       CodecType codec;         // H264 / H265 / AAC / G711
   };

   // 所有流源的基类
   class MediaSource : public std::enable_shared_from_this<MediaSource> {
   public:
       virtual ~MediaSource() = default;
       virtual std::string id() const = 0;          // 如 "cam1"
       virtual CodecType video_codec() const = 0;
       virtual std::string generate_sdp(const std::string &url) = 0;

       // 订阅/取消订阅
       void add_subscriber(std::weak_ptr<IMediaSink> sink);
       void remove_subscriber(IMediaSink *sink);
       size_t subscriber_count() const;

       // 子类调用: 广播帧给所有订阅者
       void dispatch_frame(const MediaFrame &frame);

   private:
       mutable std::mutex mtx_;
       std::vector<std::weak_ptr<IMediaSink>> subscribers_;
   };

   // 消费者接口
   class IMediaSink {
   public:
       virtual ~IMediaSink() = default;
       virtual void on_media_frame(const MediaFrame &frame) = 0;
   };
   ```
2. 实现 `RtspMediaSource` -- 继承 `MediaSource`，封装 `RtspClient`:
   ```cpp
   class RtspMediaSource : public MediaSource {
   public:
       RtspMediaSource(const std::string &id, const RtspClient::Config &config);
       asio::awaitable<void> start(asio::io_context &io);
       void stop();
   private:
       RtspClient client_;
       // 缓存 SPS/PPS，用于生成 SDP 和 GOP 缓存
       std::vector<uint8_t> sps_, pps_;
   };
   ```
3. 测试: 创建 RtspMediaSource + 一个 mock subscriber，验证帧分发

**验收:** MediaFrame 能从 Source 正确分发到 Subscriber

---

#### Day 23 -- MediaSource: 编码信息提取 (2.5h)

**实践 (2.5h)**
1. 从 H.264 码流中自动提取 SPS/PPS:
   ```cpp
   void RtspMediaSource::on_rtsp_frame(const MediaFrame &frame) {
       if (frame.codec == CodecType::H264) {
           auto nals = h264_parser_.parse(frame.data->span());
           for (auto &nal : nals) {
               if (nal.type == 7) sps_.assign(nal.data.begin(), nal.data.end());
               if (nal.type == 8) pps_.assign(nal.data.begin(), nal.data.end());
           }
       }
       dispatch_frame(frame);
   }
   ```
2. SDP 动态生成: 只有收到第一个关键帧（包含 SPS/PPS）后，DESCRIBE 请求才返回 200
   - 没收到 SPS 时返回 404 或让 DESCRIBE 等待
3. 从 SPS 中解析分辨率 (宽/高) -- 用于日志和 API 展示:
   ```cpp
   struct H264SpsInfo {
       uint16_t width;
       uint16_t height;
       uint8_t profile_idc;
       uint8_t level_idc;
   };
   std::optional<H264SpsInfo> parse_sps(std::span<const uint8_t> sps_data);
   ```
4. 测试: 解析真实摄像头的 SPS，验证宽高与摄像头设置一致

**验收:** 能从码流中自动获取 SPS/PPS 和分辨率信息

---

#### Day 24 -- MediaHub: 流注册与查找 (2.5h)

**实践 (2.5h)**
1. 实现 `MediaHub` -- 全局流管理中心:
   ```cpp
   class MediaHub {
   public:
       static MediaHub &instance();

       // 注册/注销流源
       void add_source(std::shared_ptr<MediaSource> source);
       void remove_source(const std::string &id);

       // 按 ID 或路径查找
       std::shared_ptr<MediaSource> find(const std::string &id) const;
       std::vector<StreamInfo> list_all() const;  // 返回所有流的摘要

       struct StreamInfo {
           std::string id;
           std::string source_url;
           CodecType codec;
           uint16_t width, height;
           size_t subscriber_count;
           std::string status;  // "online" / "connecting" / "offline"
       };

   private:
       mutable std::shared_mutex mtx_;  // 读写锁，读多写少
       std::unordered_map<std::string, std::shared_ptr<MediaSource>> sources_;
   };
   ```
2. 读写锁 `std::shared_mutex`: 多个线程同时查找不阻塞，只有增删时独占
3. 修改 `RtspServer` 使其通过 `MediaHub::find()` 查找流源
4. 测试: 注册 3 个 source，并发查找，验证线程安全

**验收:** MediaHub 线程安全地管理多个流源

---

#### Day 25 -- MediaHub: 1:N 分发 (2.5h)

**实践 (2.5h)**
1. `dispatch_frame` 实现 -- 一路输入分发给 N 个消费者:
   ```cpp
   void MediaSource::dispatch_frame(const MediaFrame &frame) {
       std::lock_guard lock(mtx_);
       // 清理已失效的 weak_ptr
       subscribers_.erase(
           std::remove_if(subscribers_.begin(), subscribers_.end(),
               [](auto &wp) { return wp.expired(); }),
           subscribers_.end());
       // 分发 (shared_ptr<Buffer> 零拷贝)
       for (auto &wp : subscribers_) {
           if (auto sp = wp.lock()) {
               sp->on_media_frame(frame);
           }
       }
   }
   ```
2. `RtspSession` 实现 `IMediaSink` 接口:
   ```cpp
   class RtspSession : public Session, public IMediaSink {
       void on_media_frame(const MediaFrame &frame) override {
           auto packets = packetizer_.packetize(frame);
           for (auto &pkt : packets) {
               if (transport_mode_ == TCP) send_rtp_tcp(pkt);
               else send_rtp_udp(pkt);
           }
       }
   };
   ```
3. 测试: 一路摄像头，3 个 VLC 同时观看，验证画面一致

**验收:** 多个 VLC 客户端同时观看同一路流，画面同步

---

#### Day 26 -- 按需拉流 (2.5h)

**理论 (15min)**
- 按需拉流: 摄像头不需要一直拉流，只有有人观看时才连接
- 第一个客户端订阅 -> 触发连接摄像头
- 最后一个客户端取消订阅 -> 延迟 N 秒后断开摄像头（避免频繁连断）

**实践 (2h)**
1. 在 `MediaSource` 中添加按需逻辑:
   ```cpp
   void MediaSource::add_subscriber(std::weak_ptr<IMediaSink> sink) {
       std::lock_guard lock(mtx_);
       subscribers_.push_back(sink);
       if (subscribers_.size() == 1 && !is_pulling_) {
           CSK_LOG_I("First subscriber, start pulling: {}", id());
           start_pull();  // 启动拉流
       }
       cancel_idle_timer();  // 取消空闲断开定时器
   }

   void MediaSource::remove_subscriber(IMediaSink *sink) {
       std::lock_guard lock(mtx_);
       // ... 移除 subscriber
       if (subscribers_.empty()) {
           CSK_LOG_I("No subscribers, will stop in 10s: {}", id());
           start_idle_timer(std::chrono::seconds(10));
       }
   }
   ```
2. `start_idle_timer`: 10 秒内无新订阅者则断开摄像头
3. 测试:
   - VLC 打开流 -> 日志显示 "start pulling"
   - VLC 关闭 -> 10 秒后日志显示 "stop pulling"
   - 在 10 秒内重新打开 VLC -> 不断开

**验收:** 按需拉流逻辑正确，idle timeout 生效

---

#### Day 27 -- 摄像头断线重连 (2.5h)

**实践 (2.5h)**
1. 实现指数退避重连:
   ```cpp
   class ReconnectPolicy {
   public:
       ReconnectPolicy(int base_ms = 1000, int max_ms = 30000);
       int next_delay_ms();  // 1s -> 2s -> 4s -> 8s -> ... -> 30s (封顶)
       void reset();         // 连接成功后重置
   private:
       int base_ms_, max_ms_, current_ms_;
   };
   ```
2. 在 `RtspMediaSource` 中添加重连逻辑:
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
           CSK_LOG_W("Camera {} disconnected, retry in {}ms", id(), delay);
           asio::steady_timer timer(io_);
           timer.expires_after(std::chrono::milliseconds(delay));
           co_await timer.async_wait(asio::use_awaitable);
       }
   }
   ```
3. 健康状态上报: `source->status()` 返回 "online" / "reconnecting" / "offline"
4. 测试: 中途关闭 FFmpeg RTSP 源 -> 观察重连日志 -> 重启 FFmpeg -> 自动恢复播放

**验收:** 摄像头断开后自动重连，重连间隔指数增长，恢复后自动推送数据

---

#### Day 28 -- 周末: REST API (4-5h)

**实践 (4-5h)**
1. 扩展 Day 5 的 HTTP 服务器，添加 JSON 路由:
   ```cpp
   class ApiServer {
   public:
       ApiServer(asio::io_context &io, uint16_t port, MediaHub &hub);
       void start();
   private:
       void handle_list_streams(HttpRequest &req, HttpResponse &resp);
       void handle_add_stream(HttpRequest &req, HttpResponse &resp);
       void handle_delete_stream(HttpRequest &req, HttpResponse &resp);
       void handle_get_stream(HttpRequest &req, HttpResponse &resp);
   };
   ```
2. API 设计:
   ```
   GET  /api/v1/streams              -> [{"id":"cam1","status":"online","codec":"H264","width":1920,...}]
   POST /api/v1/streams              <- {"id":"cam1","url":"rtsp://admin:123@192.168.1.100/stream1"}
                                     -> {"id":"cam1","status":"connecting"}
   GET  /api/v1/streams/{id}         -> {"id":"cam1",...,"subscribers":3,"bitrate_kbps":4096}
   DELETE /api/v1/streams/{id}       -> 204 No Content
   GET  /api/v1/streams/{id}/snapshot -> JPEG 截图 (加分项)
   ```
3. 用 `nlohmann/json` 序列化/反序列化
4. 测试:
   ```bash
   # 添加摄像头
   curl -X POST http://localhost:8080/api/v1/streams \
     -H "Content-Type: application/json" \
     -d '{"id":"cam1","url":"rtsp://localhost:8554/camera1"}'

   # 列出所有流
   curl http://localhost:8080/api/v1/streams | jq

   # 删除
   curl -X DELETE http://localhost:8080/api/v1/streams/cam1
   ```

**验收:** 通过 curl/Postman 能完整管理摄像头的增删查

---

### 第 5 周: 完善 + 首次可用版本 (Day 29-35)

---

#### Day 29 -- GOP 缓存: 实现"秒开" (2.5h)

**理论 (20min)**
- GOP (Group of Pictures): 从一个 I 帧到下一个 I 帧之间的所有帧
- 问题: 新客户端连接后要等到下一个 I 帧才能开始解码（可能等 2-5 秒）
- 解决: 缓存最近一个 GOP，新客户端连接时立即发送缓存的 I 帧 + 后续 P 帧

**实践 (2h)**
1. 实现 `GopCache`:
   ```cpp
   class GopCache {
   public:
       void on_frame(const MediaFrame &frame);
       // 获取缓存的完整 GOP (从最近的 I 帧开始)
       std::vector<MediaFrame> get_cached_gop() const;
       void clear();
   private:
       mutable std::mutex mtx_;
       std::vector<MediaFrame> frames_;  // 当前 GOP 的帧
   };
   ```
2. 在 `MediaSource::add_subscriber` 时发送缓存的 GOP:
   ```cpp
   void MediaSource::add_subscriber(std::weak_ptr<IMediaSink> sink) {
       // ... 添加订阅者
       // 发送缓存的 GOP 实现秒开
       if (auto sp = sink.lock()) {
           for (auto &frame : gop_cache_.get_cached_gop()) {
               sp->on_media_frame(frame);
           }
       }
   }
   ```
3. 测试: VLC 打开流后是否立即出画面（对比无 GOP 缓存的延迟）

**验收:** 新客户端连接后 < 500ms 出画面

---

#### Day 30 -- 多线程架构优化 (2.5h)

**实践 (2.5h)**
1. 线程模型设计:
   ```
   [网络 I/O 线程组] --帧数据--> [媒体处理线程] --RTP包--> [网络 I/O 线程组]
         (Asio)                  (dispatch)                    (Asio)
   ```
2. 使用 `asio::strand` 保证每个 Session 的操作串行化:
   ```cpp
   class RtspSession : public Session, public IMediaSink {
       asio::strand<asio::io_context::executor_type> strand_;
       void on_media_frame(const MediaFrame &frame) override {
           // 确保在 strand 上执行，避免与其他操作竞争
           asio::post(strand_, [this, frame] {
               send_rtp(frame);
           });
       }
   };
   ```
3. io_context 多线程运行:
   ```cpp
   asio::io_context io;
   std::vector<std::thread> threads;
   for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
       threads.emplace_back([&io] { io.run(); });
   }
   ```
4. 测试: 10 路摄像头 x 5 个客户端，检查 CPU 多核利用率

**验收:** 多核 CPU 被充分利用，无数据竞争

---

#### Day 31-32 -- 全面测试 (5h)

**Day 31 -- 功能测试 (2.5h)**
1. 测试矩阵:
   - [ ] 1 路摄像头，1 个 VLC -- 基本播放
   - [ ] 1 路摄像头，5 个 VLC -- 多客户端
   - [ ] 3 路摄像头，各 2 个 VLC -- 多路多客户端
   - [ ] TCP interleaved 传输模式
   - [ ] UDP 传输模式
   - [ ] Digest 认证
   - [ ] 按需拉流 (全部 VLC 关闭后摄像头断开)
   - [ ] 断线重连 (中途关闭摄像头源)
   - [ ] REST API 增删查

**Day 32 -- 异常测试 (2.5h)**
1. 异常场景:
   - [ ] 客户端突然断开 (kill VLC 进程)
   - [ ] 摄像头 URL 错误 (返回 401/404)
   - [ ] 网络抖动模拟 (`tc netem` 添加延迟/丢包)
   - [ ] 超大帧 (分辨率 4K，单帧 > 500KB)
   - [ ] 长时间运行 (至少 4 小时不崩溃)

---

#### Day 33 -- 性能指标监控 (2.5h)

**实践 (2.5h)**
1. 实现 `StreamStats`:
   ```cpp
   struct StreamStats {
       std::atomic<uint64_t> bytes_received{0};
       std::atomic<uint64_t> frames_received{0};
       std::atomic<uint32_t> current_bitrate_kbps{0};
       std::atomic<uint32_t> current_fps{0};
       std::atomic<uint64_t> rtp_packets_sent{0};
       std::atomic<uint64_t> rtp_packets_lost{0};
       std::chrono::steady_clock::time_point start_time;
   };
   ```
2. 每秒计算一次码率和帧率:
   ```cpp
   void StreamStats::update_periodic() {
       auto elapsed = now() - last_update_;
       current_bitrate_kbps = (bytes_since_last * 8) / elapsed_ms;
       current_fps = frames_since_last / elapsed_s;
       // 重置计数器
   }
   ```
3. 在 REST API 中暴露: `GET /api/v1/streams/{id}/stats`
4. 测试: 验证码率统计与 Wireshark 抓包结果一致 (误差 < 5%)

**验收:** API 返回实时码率、帧率、丢包率等指标

---

#### Day 34-35 -- 周末: v0.1 发布 (4-5h)

**实践**
1. 编写 `examples/simple_proxy.cpp` -- 最小可用示例 (< 50 行)
2. 编写 README Quick Start:
   ```bash
   # 构建
   cmake -B build && cmake --build build
   # 启动
   ./build/bin/camstreamkit -c config.json
   # 添加摄像头
   curl -X POST http://localhost:8080/api/v1/streams \
     -d '{"id":"cam1","url":"rtsp://admin:123456@192.168.1.100/stream1"}'
   # VLC 播放
   vlc rtsp://localhost:554/stream/cam1
   ```
3. 创建 GitHub Release v0.1.0
4. 标签: `git tag v0.1.0 && git push --tags`

**验收:** v0.1 可用 -- RTSP 代理 + 按需拉流 + 断线重连 + REST API + 秒开

---

## 阶段三: FFmpeg 转码集成 (第 6-7 周 / Day 36-49)

### 第 6 周: FFmpeg 基础集成 (Day 36-42)

---

#### Day 36 -- FFmpeg 环境搭建 + 解码 API (2.5h)

**理论 (40min)**
- FFmpeg 库家族: `libavformat`(容器) / `libavcodec`(编解码) / `libswscale`(缩放) / `libavutil`(工具)
- 解码流程: `AVPacket`(压缩帧) -> `avcodec_send_packet` -> `avcodec_receive_frame` -> `AVFrame`(原始YUV)
- 关键: 解码器是有状态的，必须先发 SPS/PPS 初始化，再发 IDR 帧

**实践 (90min)**
1. CMake 中找 FFmpeg: `find_package(PkgConfig REQUIRED)` + `pkg_check_modules(FFMPEG libavcodec libavformat libswscale libavutil)`
2. 使用 Day 2 的 RAII wrapper:
   ```cpp
   using AvCodecCtx = std::unique_ptr<AVCodecContext, decltype([](AVCodecContext *p) {
       avcodec_free_context(&p);
   })>;
   ```
3. 实现 `H264Decoder`:
   ```cpp
   class H264Decoder {
   public:
       H264Decoder();
       // 输入 NAL 数据 (Annex-B 格式)，输出解码后的 YUV 帧
       struct DecodedFrame {
           int width, height;
           AVPixelFormat format;  // AV_PIX_FMT_YUV420P
           std::array<uint8_t*, 3> planes;
           std::array<int, 3> linesize;
       };
       std::optional<DecodedFrame> decode(std::span<const uint8_t> nal_data);
   private:
       AvCodecCtx codec_ctx_;
       AVPacket *pkt_;
       AVFrame *frame_;
   };
   ```
4. 测试: 解码一个 H.264 文件，打印每帧的宽高和类型

**验收:** 能成功解码 H.264 并输出 YUV 帧信息

---

#### Day 37 -- FFmpeg 解码实战: 从 RTP 流解码 (2.5h)

**实践 (2.5h)**
1. 将 `RtspClient` 收到的 RTP 数据喂给解码器:
   ```cpp
   void on_media_frame(const MediaFrame &frame) {
       // MediaFrame.data 已经是 Annex-B 格式的 NAL
       auto decoded = decoder_.decode(frame.data->span());
       if (decoded) {
           CSK_LOG_D("Decoded frame: {}x{} pts={}", decoded->width, decoded->height, frame.pts);
           // 后续: 送入编码器
       }
   }
   ```
2. 处理解码延迟: `avcodec_receive_frame` 可能返回 `EAGAIN`，需要继续送数据
3. 实现保存解码帧为 JPEG (调试用):
   ```cpp
   void save_jpeg(const DecodedFrame &frame, const std::string &path);
   // 使用 libswscale 将 YUV420P 转 RGB，用 FFmpeg MJPEG 编码器压缩
   ```
4. 测试: 每收到一个关键帧保存为 JPEG，人工验证画面正确

**验收:** 从 RTSP 流解码出的 JPEG 截图画面正确

---

#### Day 38 -- FFmpeg 编码器: H.265 编码 (2.5h)

**实践 (2.5h)**
1. 实现 `H265Encoder`:
   ```cpp
   class H265Encoder {
   public:
       struct Config {
           int width, height;
           int fps = 25;
           int bitrate_kbps = 2000;
           int gop_size = 50;        // 关键帧间隔
           std::string preset = "medium";  // ultrafast/fast/medium/slow
       };
       H265Encoder(const Config &config);
       // 输入 YUV 帧，输出编码后的 NAL 数据
       std::vector<std::vector<uint8_t>> encode(const DecodedFrame &frame);
       // 刷新编码器缓冲 (结束时调用)
       std::vector<std::vector<uint8_t>> flush();
   private:
       AvCodecCtx codec_ctx_;
       AVPacket *pkt_;
       AVFrame *frame_;
       int64_t pts_{0};
   };
   ```
2. 编码参数设置:
   ```cpp
   codec_ctx_->bit_rate = config.bitrate_kbps * 1000;
   codec_ctx_->gop_size = config.gop_size;
   av_opt_set(codec_ctx_->priv_data, "preset", config.preset.c_str(), 0);
   av_opt_set(codec_ctx_->priv_data, "tune", "zerolatency", 0);  // 低延迟
   ```
3. 测试: 解码 H.264 -> 编码为 H.265 -> 保存为文件 -> ffplay 验证可播放

**验收:** 输出的 H.265 文件能被 ffplay 正常播放

---

#### Day 39 -- 缩放器: 分辨率调整 (2.5h)

**实践 (2.5h)**
1. 实现 `VideoScaler` -- 封装 `libswscale`:
   ```cpp
   class VideoScaler {
   public:
       VideoScaler(int src_w, int src_h, AVPixelFormat src_fmt,
                   int dst_w, int dst_h, AVPixelFormat dst_fmt);
       ~VideoScaler();
       void scale(const AVFrame *src, AVFrame *dst);
   private:
       SwsContext *sws_ctx_;
   };
   ```
2. 支持常见缩放: 1080p -> 720p, 4K -> 1080p
3. 测试: 1920x1080 输入 -> 1280x720 输出，验证画面比例正确

---

#### Day 40-41 -- Transcoder 模块: 异步转码管线 (5h)

**实践 (5h)**
1. 设计异步转码管线:
   ```
   MediaSource --帧--> [解码队列] --YUV--> [缩放] --YUV--> [编码队列] --NAL--> TranscodedSource
   ```
2. 实现 `Transcoder`:
   ```cpp
   class Transcoder {
   public:
       struct Config {
           CodecType output_codec = CodecType::H265;
           int output_width = 0;   // 0 = 保持原始
           int output_height = 0;
           int bitrate_kbps = 2000;
       };
       Transcoder(const Config &config);
       void start();
       void stop();
       // 输入: 从摄像头收到的帧
       void feed(const MediaFrame &frame);
       // 输出: 转码后的帧
       void set_output_callback(std::function<void(const MediaFrame&)> cb);
   private:
       std::thread decode_thread_;
       std::thread encode_thread_;
       ThreadSafeQueue<MediaFrame> decode_queue_;
       ThreadSafeQueue<DecodedFrame> encode_queue_;
   };
   ```
3. 实现 `TranscodedMediaSource` -- 将原始 Source 包装，输出转码后的流:
   ```cpp
   class TranscodedMediaSource : public MediaSource, public IMediaSink {
       // 订阅原始 source，收到帧后送入 Transcoder，输出新的 MediaFrame
   };
   ```
4. 测试: RTSP H.264 1080p 输入 -> 转码为 H.265 720p -> VLC 播放

**验收:** VLC 能播放转码后的 H.265 720p 流

---

#### Day 42 -- 周末: 转码端到端测试 (4h)

**实践 (4h)**
1. 通过 REST API 创建转码流:
   ```bash
   curl -X POST http://localhost:8080/api/v1/streams \
     -d '{"id":"cam1_720p","source":"cam1","transcode":{"codec":"h265","width":1280,"height":720,"bitrate":2000}}'
   ```
2. 测试场景:
   - [ ] 原始流 `cam1` (H264 1080p) + 转码流 `cam1_720p` (H265 720p) 同时可播放
   - [ ] 只有 `cam1_720p` 有观看者时才启动转码器
   - [ ] 转码延迟 < 200ms (zerolatency preset)
   - [ ] 连续运行 2 小时无内存泄漏

**验收:** 原始流和转码流同时可用，按需启动转码

---

### 第 7 周: 转码优化 + 硬件加速 (Day 43-49)

---

#### Day 43-44 -- 硬件加速编码 (5h)

**理论 (1h)**
- 硬件加速编码器: NVIDIA NVENC (`h264_nvenc`/`hevc_nvenc`), Intel QSV (`h264_qsv`), VAAPI (`h264_vaapi`)
- FFmpeg 硬件加速统一接口: `AVHWDeviceType`, `av_hwdevice_ctx_create`
- 策略: 运行时探测可用的硬件编码器，优先硬件，fallback 软编

**实践 (4h)**
1. 实现硬件探测:
   ```cpp
   struct HwAccelInfo {
       AVHWDeviceType type;
       std::string encoder_name;  // "hevc_nvenc", "hevc_qsv", "hevc_vaapi"
   };
   std::optional<HwAccelInfo> detect_hw_encoder(CodecType codec);
   // 尝试创建硬件编码上下文，成功返回 info，失败返回 nullopt
   ```
2. 修改 `H265Encoder` 支持硬件:
   - 有 NVENC -> 用 `hevc_nvenc`
   - 无 NVENC 有 QSV -> 用 `hevc_qsv`
   - 都没有 -> 用 `libx265` 软编
3. 测试: 打印检测到的硬件编码器，对比软编/硬编的 CPU 占用

**验收:** 有 GPU 时使用硬件编码，CPU 占用降低 > 50%

---

#### Day 45-46 -- 转码参数配置化 (5h)

**实践 (5h)**
1. 配置文件支持:
   ```json
   {
     "transcode_profiles": {
       "720p": {"codec": "h265", "width": 1280, "height": 720, "bitrate": 2000, "fps": 25},
       "480p": {"codec": "h264", "width": 854, "height": 480, "bitrate": 1000, "fps": 15},
       "snapshot": {"codec": "mjpeg", "width": 640, "height": 480, "fps": 1}
     }
   }
   ```
2. 码率控制模式:
   - CBR: 恒定码率 -- `codec_ctx_->rc_max_rate = codec_ctx_->bit_rate`
   - VBR: 可变码率 -- `av_opt_set(priv, "rc", "vbr", 0)` + 设置 maxrate/bufsize
3. 截图 API: `GET /api/v1/streams/{id}/snapshot` 返回 JPEG:
   ```cpp
   void handle_snapshot(const std::string &stream_id, HttpResponse &resp) {
       auto source = hub_.find(stream_id);
       auto frame = source->last_key_frame();
       auto jpeg = decode_and_encode_jpeg(frame);
       resp.set_content_type("image/jpeg");
       resp.set_body(jpeg);
   }
   ```
4. 测试: 通过浏览器 `http://localhost:8080/api/v1/streams/cam1/snapshot` 查看截图

---

#### Day 47-48 -- 按需转码 + 转码资源管理 (5h)

**实践 (5h)**
1. 转码实例池:
   ```cpp
   class TranscoderPool {
   public:
       std::shared_ptr<Transcoder> acquire(const std::string &source_id,
                                           const Transcoder::Config &config);
       void release(const std::string &transcoded_id);
       size_t active_count() const;
       // 限制最大同时转码数 (防止 CPU/GPU 过载)
       void set_max_concurrent(size_t max);
   };
   ```
2. 流命名规则: 原始流 `cam1` + profile `720p` = 转码流 `cam1@720p`
   - VLC 请求 `rtsp://localhost/stream/cam1@720p` -> 触发按需转码
3. 资源保护: 超过最大转码数时返回 503 Service Unavailable
4. 测试: 设置最大 3 路转码，尝试创建第 4 路

---

#### Day 49 -- 周末: v0.2 发布 (4h)

**实践 (4h)**
1. 更新 README -- 添加转码功能说明
2. 创建 `examples/transcode_proxy.cpp`
3. 更新 config.json 模板
4. GitHub Release v0.2.0
5. 运行完整测试套件

**验收:** v0.2 可用 -- RTSP 代理 + 按需转码 + 截图 API

---

## 阶段四: GB28181 国标协议 (第 8-10 周 / Day 50-70)

### 第 8 周: SIP 信令基础 (Day 50-56)

---

#### Day 50 -- SIP 协议学习 (2.5h)

**理论 (2.5h -- 纯理论日)**
- SIP (Session Initiation Protocol, RFC 3261): 建立/修改/终止多媒体会话
- GB28181 使用 SIP 作为信令协议控制摄像头
- 核心消息:
  - `REGISTER`: 设备注册到平台 (每 60s 续期)
  - `INVITE`: 平台邀请设备开始发送视频流
  - `ACK`: 确认 INVITE
  - `BYE`: 结束会话
  - `MESSAGE`: 传递控制命令 (目录查询、PTZ 等)
- SIP 消息格式 (与 HTTP 类似):
  ```
  REGISTER sip:34020000002000000001@192.168.1.10 SIP/2.0
  Via: SIP/2.0/UDP 192.168.1.100:5060;branch=z9hG4bK776asdhds
  From: <sip:34020000001320000001@192.168.1.100>;tag=1928301774
  To: <sip:34020000001320000001@192.168.1.100>
  Call-ID: a84b4c76e66710@192.168.1.100
  CSeq: 1 REGISTER
  Contact: <sip:34020000001320000001@192.168.1.100:5060>
  Expires: 3600
  Content-Length: 0
  ```
- GB28181 编码规则: 20 位设备编号，前 8 位是行政区划，9-10 位是行业，11-13 位是设备类型
- 阅读: GB/T 28181-2022 第 5-7 章

---

#### Day 51 -- SIP 消息解析器 (2.5h)

**实践 (2.5h)**
1. 实现 SIP 消息解析:
   ```cpp
   struct SipMessage {
       bool is_request;    // true=请求, false=响应
       // 请求行
       std::string method; // "REGISTER", "INVITE", ...
       std::string uri;    // "sip:34020000002000000001@192.168.1.10"
       // 响应行
       int status_code;    // 200
       std::string reason; // "OK"
       // 通用头部
       std::string via;
       std::string from;
       std::string to;
       std::string call_id;
       int cseq;
       std::string cseq_method;
       std::string contact;
       int expires = -1;
       int content_length = 0;
       std::map<std::string, std::string> headers;
       std::string body;   // XML for MESSAGE, SDP for INVITE
   };

   class SipParser {
   public:
       std::optional<SipMessage> parse(const std::string &data);
       std::string build(const SipMessage &msg);
   private:
       void parse_request_line(const std::string &line, SipMessage &msg);
       void parse_header(const std::string &line, SipMessage &msg);
   };
   ```
2. 实现 SIP URI 解析: `sip:user@host:port` -> `{user, host, port}`
3. 测试: 用真实的 GB28181 注册消息验证解析

**验收:** 能正确解析 REGISTER / INVITE / BYE / MESSAGE 消息

---

#### Day 52-53 -- SIP 传输层 + 注册服务 (5h)

**实践 (5h)**
1. 实现 SIP UDP 传输 (GB28181 主要用 UDP):
   ```cpp
   class SipTransport {
   public:
       SipTransport(asio::io_context &io, uint16_t port = 5060);
       void set_message_handler(std::function<void(SipMessage, asio::ip::udp::endpoint)> handler);
       void send(const SipMessage &msg, const asio::ip::udp::endpoint &ep);
   private:
       UdpSocket socket_;
       SipParser parser_;
   };
   ```
2. 实现 `SipServer` -- GB28181 信令核心:
   ```cpp
   class SipServer {
   public:
       SipServer(asio::io_context &io, const Gb28181Config &config);
       void start();
       void stop();
       // 设备管理
       std::vector<DeviceInfo> list_devices() const;
       std::optional<DeviceInfo> find_device(const std::string &device_id) const;
   private:
       void handle_register(const SipMessage &msg, const asio::ip::udp::endpoint &from);
       void handle_message(const SipMessage &msg, const asio::ip::udp::endpoint &from);

       struct DeviceInfo {
           std::string device_id;      // 20位国标编号
           asio::ip::udp::endpoint addr;
           std::chrono::steady_clock::time_point last_heartbeat;
           std::string status;         // "online" / "offline"
           std::vector<ChannelInfo> channels;
       };
       std::map<std::string, DeviceInfo> devices_;
   };
   ```
3. REGISTER 处理:
   ```cpp
   void SipServer::handle_register(const SipMessage &msg, const auto &from) {
       auto device_id = extract_user(msg.from);  // "34020000001320000001"
       SipMessage resp = build_response(msg, 200, "OK");
       resp.headers["Date"] = current_sip_date();
       transport_.send(resp, from);

       devices_[device_id] = {device_id, from, now(), "online", {}};
       CSK_LOG_I("Device registered: {} from {}", device_id, from);
   }
   ```
4. 认证: GB28181 使用 SIP Digest 认证 (与 RTSP 类似，复用 Day 20 的代码)

**验收:** 摄像头能注册到 CamStreamKit，后台日志显示设备上线

---

#### Day 54-55 -- 心跳 + 设备目录查询 (5h)

**实践 (5h)**
1. 心跳处理 -- 设备通过 MESSAGE 发送心跳 (XML body):
   ```xml
   <?xml version="1.0"?>
   <Notify>
       <CmdType>Keepalive</CmdType>
       <SN>1</SN>
       <DeviceID>34020000001320000001</DeviceID>
       <Status>OK</Status>
   </Notify>
   ```
   ```cpp
   void SipServer::handle_keepalive(const SipMessage &msg) {
       auto xml = parse_xml(msg.body);
       auto device_id = xml.child("DeviceID").text();
       if (devices_.count(device_id)) {
           devices_[device_id].last_heartbeat = now();
           CSK_LOG_D("Heartbeat from {}", device_id);
       }
   }
   ```
2. 超时检测: 定时器每 30s 扫描，超过 3 次心跳周期未响应则标记 offline
3. 设备目录查询 -- 平台主动向设备询问通道列表:
   ```cpp
   asio::awaitable<std::vector<ChannelInfo>> SipServer::query_catalog(const std::string &device_id) {
       SipMessage msg;
       msg.method = "MESSAGE";
       msg.body = R"(<?xml version="1.0"?>
           <Query><CmdType>Catalog</CmdType><SN>2</SN>
           <DeviceID>)" + device_id + R"(</DeviceID></Query>)";
       transport_.send(msg, devices_[device_id].addr);
       // 等待设备回复 (也是 MESSAGE + XML)
       auto resp = co_await wait_for_catalog_response(device_id);
       co_return parse_catalog_xml(resp.body);
   }
   ```
4. 设备回复的目录 XML 解析:
   ```xml
   <Response>
       <CmdType>Catalog</CmdType>
       <DeviceList Num="2">
           <Item>
               <DeviceID>34020000001310000001</DeviceID>
               <Name>Camera-1</Name>
               <Status>ON</Status>
           </Item>
       </DeviceList>
   </Response>
   ```
5. REST API 集成: `GET /api/v1/gb28181/devices` 列出所有已注册设备

**验收:** 能查询到设备通道列表，API 返回设备信息

---

#### Day 56 -- 周末: 真实设备注册测试 (4h)

**实践 (4h)**
1. 配置海康/大华摄像头的 GB28181 参数:
   - SIP 服务器地址: CamStreamKit 的 IP
   - SIP 服务器端口: 5060
   - SIP 服务器 ID: 34020000002000000001
   - 设备 ID: 摄像头自带的 20 位编号
2. 测试场景:
   - [ ] 摄像头注册成功 -> API 显示 online
   - [ ] 心跳正常 -> 每 60s 收到 Keepalive
   - [ ] 目录查询 -> 返回通道列表
   - [ ] 断开摄像头网线 -> 3 分钟后自动标记 offline
3. 如果没有真实 GB28181 设备，用开源模拟器:
   ```bash
   # 使用 gb28181-simulation 工具模拟 IPC
   git clone https://github.com/swwheihei/wvp-GB28181-pro
   # 或用 Python SIP 库模拟注册
   ```

**验收:** 真实或模拟的 GB28181 设备能注册并维持心跳

---

### 第 9 周: GB28181 媒体流 (Day 57-63)

---

#### Day 57-58 -- PS 流解封装 (5h)

**理论 (1h)**
- GB28181 的 RTP 负载不是裸 H.264，而是 PS (Program Stream MPEG-2) 封装
- PS 结构: `Pack Header (00 00 01 BA)` + `System Header (00 00 01 BB)` + `PES Packet (00 00 01 E0)`
- 解封装流程: RTP -> PS Pack -> PES -> H.264 NAL

**实践 (4h)**
1. 实现 PS 解封装器:
   ```cpp
   class PsDepacketizer {
   public:
       // 输入 RTP payload (PS 数据)，输出 H.264 NAL 单元
       struct PesPacket {
           uint8_t stream_id;  // 0xE0 = video, 0xC0 = audio
           uint64_t pts, dts;
           std::vector<uint8_t> data;
       };
       std::vector<PesPacket> depacketize(std::span<const uint8_t> ps_data);
   private:
       bool parse_pack_header(BufferReader &reader);
       std::optional<PesPacket> parse_pes(BufferReader &reader);
   };
   ```
2. PS 头解析关键字节:
   ```cpp
   bool PsDepacketizer::parse_pack_header(BufferReader &reader) {
       auto start_code = reader.read_u32_be();
       if (start_code != 0x000001BA) return false;
       // MPEG-2 PS: 第5字节 bit[7:6] == 01
       auto byte4 = reader.read_u8();
       bool is_mpeg2 = (byte4 >> 6) == 1;
       // 解析 SCR (System Clock Reference)
       // 跳过填充字节
       return true;
   }
   ```
3. 从 PES 中提取 H.264: `pes.data` 已经是 Annex-B 格式的 NAL 数据
4. 测试: 用 Wireshark 抓取的 GB28181 RTP 包，验证 PS 解封装

**验收:** 能从 PS 流中正确提取 H.264 NAL 单元

---

#### Day 59-60 -- 实时视频点播: INVITE 流程 (5h)

**理论 (30min)**
- GB28181 视频点播流程:
  1. 平台 -> 设备: `INVITE` (SDP 中指定接收地址和端口)
  2. 设备 -> 平台: `200 OK` (SDP 中回复发送端口)
  3. 平台 -> 设备: `ACK`
  4. 设备开始发送 RTP(PS) 流到平台指定的端口

**实践 (4.5h)**
1. 实现 INVITE 发送:
   ```cpp
   asio::awaitable<bool> SipServer::invite_stream(const std::string &device_id,
                                                   const std::string &channel_id) {
       auto &dev = devices_[device_id];
       uint16_t rtp_port = allocate_rtp_port();  // 分配一个 UDP 端口接收 RTP

       SipMessage invite;
       invite.method = "INVITE";
       invite.uri = fmt::format("sip:{}@{}:{}", channel_id, dev.addr.address(), dev.addr.port());
       invite.body = build_invite_sdp(rtp_port, channel_id);
       transport_.send(invite, dev.addr);

       // 等待 200 OK
       auto resp = co_await wait_for_response(invite.call_id, std::chrono::seconds(10));
       if (resp.status_code != 200) co_return false;

       // 发送 ACK
       send_ack(resp);

       // 开始在 rtp_port 上接收数据
       start_rtp_receiver(rtp_port, channel_id);
       co_return true;
   }
   ```
2. INVITE SDP 格式:
   ```
   v=0
   o=34020000002000000001 0 0 IN IP4 192.168.1.10
   s=Play
   c=IN IP4 192.168.1.10
   t=0 0
   m=video 6000 RTP/AVP 96
   a=recvonly
   a=rtpmap:96 PS/90000
   y=0100000001
   ```
3. 创建 `Gb28181MediaSource`: 继承 `MediaSource`，封装 PS 解封装:
   ```cpp
   class Gb28181MediaSource : public MediaSource {
   public:
       Gb28181MediaSource(const std::string &id, SipServer &sip,
                          const std::string &device_id, const std::string &channel_id);
   private:
       void on_rtp_packet(const RtpPacket &pkt) {
           auto pes_list = ps_depacketizer_.depacketize(pkt.payload());
           for (auto &pes : pes_list) {
               if (pes.stream_id == 0xE0) {  // video
                   MediaFrame frame;
                   frame.data = std::make_shared<Buffer>(pes.data);
                   frame.pts = pes.pts;
                   dispatch_frame(frame);
               }
           }
       }
       PsDepacketizer ps_depacketizer_;
   };
   ```
4. 测试: 向 GB28181 设备发送 INVITE，在 rtp_port 上收到数据，解封装后保存为 .h264

**验收:** 能向 GB28181 设备发起点播并接收解码视频流

---

#### Day 61-62 -- BYE + 目录查询 XML 完善 (5h)

**实践 (5h)**
1. 实现 BYE -- 停止视频流:
   ```cpp
   void SipServer::stop_stream(const std::string &call_id) {
       auto &session = active_sessions_[call_id];
       SipMessage bye;
       bye.method = "BYE";
       bye.call_id = call_id;
       transport_.send(bye, session.remote_addr);
       stop_rtp_receiver(session.rtp_port);
       active_sessions_.erase(call_id);
   }
   ```
2. 完善目录查询 XML 解析 (使用 pugixml 或 tinyxml2):
   ```cpp
   std::vector<ChannelInfo> parse_catalog_xml(const std::string &xml) {
       pugi::xml_document doc;
       doc.load_string(xml.c_str());
       std::vector<ChannelInfo> channels;
       for (auto item : doc.select_nodes("//DeviceList/Item")) {
           ChannelInfo ch;
           ch.device_id = item.node().child_value("DeviceID");
           ch.name = item.node().child_value("Name");
           ch.status = item.node().child_value("Status");
           ch.ptz_type = std::stoi(item.node().child_value("PTZType"));
           channels.push_back(ch);
       }
       return channels;
   }
   ```
3. REST API 扩展:
   ```
   GET  /api/v1/gb28181/devices                -> [{device_id, channels, status}]
   POST /api/v1/gb28181/devices/{id}/invite     <- {channel_id}  -> 开始点播
   POST /api/v1/gb28181/devices/{id}/bye        <- {channel_id}  -> 停止点播
   ```
4. 测试: 完整流程 -- 注册 -> 查询目录 -> 点播 -> 播放 -> 停止

---

#### Day 63 -- 周末: GB28181 端到端测试 (4-5h)

**实践 (4-5h)**
1. 完整 GB28181 流程测试:
   ```
   [GB28181摄像头] --REGISTER--> [CamStreamKit SipServer] 
                   <--200 OK---
                   --Keepalive-> (每60s)
   [用户] --curl POST invite--> [CamStreamKit]
   [CamStreamKit] --INVITE--> [摄像头]
                  <--200 OK--
                  --ACK-->
   [摄像头] --RTP(PS)--> [CamStreamKit] --解封装--> [MediaHub] --RTSP--> [VLC]
   ```
2. 国标设备流通过 RTSP 播放:
   - GB28181 设备注册 -> 自动创建 `Gb28181MediaSource`
   - VLC 访问 `rtsp://localhost/stream/gb_{channel_id}` 即可播放
3. 测试清单:
   - [ ] 设备注册 + 心跳
   - [ ] 目录查询返回通道列表
   - [ ] INVITE 点播 -> VLC 看到画面
   - [ ] BYE 停止 -> VLC 停止
   - [ ] 设备离线检测

**验收:** GB28181 摄像头的视频能通过 RTSP 转发给 VLC 播放

---

### 第 10 周: GB28181 完善 (Day 64-70)

---

#### Day 64-65 -- 云台控制 (PTZ) (5h)

**理论 (30min)**
- GB28181 PTZ 控制通过 MESSAGE + XML 发送，body 中包含 8 字节十六进制指令
- 指令格式: `A5 0F 01 [方向] [速度水平] [速度垂直] [放大缩小] [校验]`
- 方向: 上(0x08), 下(0x04), 左(0x02), 右(0x01), 左上(0x0A)...

**实践 (4.5h)**
1. 实现 PTZ 指令编码:
   ```cpp
   class PtzCommand {
   public:
       enum Direction { UP=0x08, DOWN=0x04, LEFT=0x02, RIGHT=0x01,
                        LEFT_UP=0x0A, RIGHT_UP=0x09, LEFT_DOWN=0x06, RIGHT_DOWN=0x05,
                        ZOOM_IN=0x10, ZOOM_OUT=0x20, STOP=0x00 };
       // 生成 8 字节十六进制指令字符串
       static std::string encode(Direction dir, uint8_t speed = 128,
                                 uint8_t zoom_speed = 0, uint8_t device_addr = 1);
       static std::string stop(uint8_t device_addr = 1);
   };
   ```
2. 发送 PTZ 控制:
   ```cpp
   void SipServer::send_ptz(const std::string &device_id, const std::string &channel_id,
                            PtzCommand::Direction dir, uint8_t speed) {
       auto cmd = PtzCommand::encode(dir, speed);
       std::string xml = fmt::format(R"(<?xml version="1.0"?>
           <Control><CmdType>DeviceControl</CmdType><SN>{}</SN>
           <DeviceID>{}</DeviceID><PTZCmd>{}</PTZCmd></Control>)",
           next_sn(), channel_id, cmd);
       SipMessage msg;
       msg.method = "MESSAGE";
       msg.body = xml;
       transport_.send(msg, devices_[device_id].addr);
   }
   ```
3. REST API: `POST /api/v1/gb28181/devices/{id}/ptz`
   ```json
   {"channel": "34020000001310000001", "direction": "left", "speed": 128}
   ```
4. 测试: 通过 API 控制云台转动，观察 VLC 画面变化

**验收:** 能通过 REST API 控制 GB28181 摄像头的云台

---

#### Day 66-67 -- GB28181 <-> RTSP 协议互转 (5h)

**实践 (5h)**
1. GB28181 -> RTSP:
   - 已在 Day 59-60 实现: `Gb28181MediaSource` 注册到 MediaHub
   - VLC 通过 RTSP 拉流即可
2. RTSP -> GB28181 (作为下级平台向上级推流):
   ```cpp
   class Gb28181Pusher {
   public:
       // 将一路 RTSP 流通过 GB28181 协议推给上级平台
       Gb28181Pusher(MediaSource &source, const Gb28181PushConfig &config);
       void start();  // 注册到上级平台，等待 INVITE
   private:
       void on_invite(const SipMessage &invite);
       void pack_to_ps(const MediaFrame &frame);  // H.264 -> PS -> RTP
   };
   ```
3. PS 封装器 (与 Day 57 的解封装反向):
   ```cpp
   class PsPacketizer {
   public:
       // 输入 H.264 NAL，输出 PS 数据
       std::vector<uint8_t> packetize(std::span<const uint8_t> h264_nal,
                                       uint64_t pts, uint64_t dts,
                                       bool is_key_frame);
   private:
       void write_pack_header(BufferWriter &w, uint64_t scr);
       void write_system_header(BufferWriter &w);  // 只在 I 帧前写
       void write_pes(BufferWriter &w, std::span<const uint8_t> data, uint64_t pts, uint64_t dts);
   };
   ```
4. 测试: CamStreamKit 作为下级平台向 WVP-Pro 注册并推流

---

#### Day 68-69 -- 异常处理 + 会话管理 (5h)

**实践 (5h)**
1. 设备离线检测优化:
   ```cpp
   void SipServer::check_device_heartbeats() {
       auto now = std::chrono::steady_clock::now();
       for (auto it = devices_.begin(); it != devices_.end();) {
           auto elapsed = now - it->second.last_heartbeat;
           if (elapsed > std::chrono::seconds(180)) {  // 3 个心跳周期
               CSK_LOG_W("Device {} offline, cleaning up", it->first);
               cleanup_device_sessions(it->first);  // 停止所有关联的媒体流
               it->second.status = "offline";
           }
           ++it;
       }
   }
   ```
2. INVITE 超时: 10 秒内没有 200 OK 则放弃
3. RTP 接收超时: 30 秒没有 RTP 数据则认为流断开，发送 BYE
4. 端口复用: RTP 端口池管理，用完释放
   ```cpp
   class RtpPortPool {
   public:
       RtpPortPool(uint16_t start = 30000, uint16_t end = 31000);
       std::optional<uint16_t> allocate();  // 返回可用端口
       void release(uint16_t port);
   };
   ```
5. 测试: 模拟各种异常场景 -- 设备断线、INVITE 无响应、流中断

**验收:** 所有异常场景都能优雅处理，不泄漏资源

---

#### Day 70 -- 周末: v0.3 发布 (4h)

**实践 (4h)**
1. 更新 README -- GB28181 功能说明 + 配置示例
2. GB28181 配置:
   ```json
   {
     "gb28181": {
       "enabled": true,
       "sip_port": 5060,
       "server_id": "34020000002000000001",
       "domain": "3402000000",
       "password": "12345678",
       "heartbeat_timeout_s": 180
     }
   }
   ```
3. 创建 `examples/gb28181_proxy.cpp`
4. GitHub Release v0.3.0

**验收:** v0.3 可用 -- RTSP 代理 + 转码 + GB28181 信令/媒体/PTZ

---

## 阶段五: WebRTC 集成 (第 11-12 周 / Day 71-84)

### 第 11 周: WebRTC 基础 (Day 71-77)

---

#### Day 71 -- WebRTC 协议栈学习 (2.5h)

**理论 (2.5h -- 纯理论日)**
- WebRTC 协议栈层次:
  ```
  [应用层: 视频/音频/数据通道]
          |
  [SRTP / SCTP]           -- 加密的 RTP / 数据通道
  [DTLS]                  -- TLS 的 UDP 版本
  [ICE / STUN / TURN]     -- NAT 穿越
  [UDP]
  ```
- ICE (Interactive Connectivity Establishment):
  - Candidate 类型: host (本地IP) / srflx (STUN反射) / relay (TURN中继)
  - 连通性检查: 逐对尝试 candidate pair，选择最优
- SDP Offer/Answer 模型:
  - 浏览器发送 Offer SDP (我支持什么编码、我的 ICE candidate)
  - 服务端返回 Answer SDP (我选择什么编码、我的 ICE candidate)
- WHEP (WebRTC-HTTP Egress Protocol):
  - 简化版: 浏览器 POST SDP Offer 到 HTTP endpoint，服务端返回 SDP Answer
  - 不需要额外的信令服务器 (WebSocket 等)
- 阅读: WHEP draft spec, libdatachannel wiki

---

#### Day 72 -- WebRTC SDP 与 ICE 细节 (2.5h)

**理论 + 实验 (2.5h)**
1. WebRTC SDP 与普通 SDP 的区别:
   ```
   a=ice-ufrag:xxxx
   a=ice-pwd:yyyyyyyy
   a=fingerprint:sha-256 AA:BB:CC:...   <- DTLS 证书指纹
   a=setup:actpass                        <- DTLS 角色
   a=mid:0
   a=rtcp-mux                            <- RTP 和 RTCP 复用同一端口
   a=rtpmap:96 H264/90000
   a=fmtp:96 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f
   a=rtcp-fb:96 nack                     <- 丢包重传
   ```
2. 用浏览器开发者工具抓取真实 WebRTC SDP:
   ```javascript
   // 在浏览器控制台中:
   const pc = new RTCPeerConnection();
   pc.addTransceiver('video', {direction: 'recvonly'});
   const offer = await pc.createOffer();
   console.log(offer.sdp);  // 观察 SDP 结构
   ```
3. 理解 `profile-level-id`: 42e01f = Baseline Profile, Level 3.1
   - WebRTC 主流支持: Baseline (42xx) 和 Constrained Baseline (42e0)
   - H.265 在 WebRTC 中的支持状况 (尚不普遍)

---

#### Day 73-74 -- 集成 libdatachannel (5h)

**实践 (5h)**
1. 通过 FetchContent 引入 libdatachannel:
   ```cmake
   FetchContent_Declare(libdatachannel
       GIT_REPOSITORY https://github.com/paullouisageneau/libdatachannel.git
       GIT_TAG v0.21.2
   )
   set(NO_WEBSOCKET ON)   # 不需要 WebSocket
   set(NO_MEDIA OFF)      # 需要媒体传输
   FetchContent_MakeAvailable(libdatachannel)
   ```
2. 实现 `WebRtcSession`:
   ```cpp
   class WebRtcSession {
   public:
       WebRtcSession();
       // 接收浏览器的 Offer SDP，生成 Answer SDP
       std::string handle_offer(const std::string &offer_sdp);
       // 开始向浏览器推送 RTP 数据
       void send_rtp(const RtpPacket &pkt);
       void close();
   private:
       std::shared_ptr<rtc::PeerConnection> pc_;
       std::shared_ptr<rtc::Track> video_track_;
   };
   ```
3. PeerConnection 配置:
   ```cpp
   WebRtcSession::WebRtcSession() {
       rtc::Configuration config;
       config.iceServers.emplace_back("stun:stun.l.google.com:19302");
       pc_ = std::make_shared<rtc::PeerConnection>(config);
       pc_->onStateChange([](rtc::PeerConnection::State state) {
           CSK_LOG_I("WebRTC state: {}", static_cast<int>(state));
       });
   }
   ```
4. 创建 Video Track 并设置 Answer:
   ```cpp
   std::string WebRtcSession::handle_offer(const std::string &offer_sdp) {
       pc_->setRemoteDescription(rtc::Description(offer_sdp, rtc::Description::Type::Offer));

       rtc::Description::Video video("video", rtc::Description::Direction::SendOnly);
       video.addH264Codec(96);  // payload type 96
       video.addSSRC(ssrc_, "video-stream");
       video_track_ = pc_->addTrack(video);

       pc_->setLocalDescription();
       // 等待 ICE gathering 完成
       auto answer = pc_->localDescription()->generateSdp();
       return answer;
   }
   ```
5. 测试: 硬编码一个 Offer，验证能生成合法的 Answer

**验收:** libdatachannel 编译通过，能创建 PeerConnection 并生成 Answer SDP

---

#### Day 75-76 -- WHEP 端点实现 (5h)

**实践 (5h)**
1. 在 HTTP API 中添加 WHEP 端点:
   ```cpp
   // POST /api/v1/whep/{stream_id}
   // Request Body: SDP Offer (text/plain 或 application/sdp)
   // Response Body: SDP Answer
   void ApiServer::handle_whep(HttpRequest &req, HttpResponse &resp) {
       auto stream_id = req.path_param("stream_id");
       auto source = hub_.find(stream_id);
       if (!source) { resp.set_status(404); return; }

       auto session = std::make_shared<WebRtcSession>();
       auto answer = session->handle_offer(req.body());

       // 订阅媒体流
       source->add_subscriber(session);  // WebRtcSession 实现 IMediaSink

       resp.set_status(201);
       resp.set_header("Content-Type", "application/sdp");
       resp.set_header("Location", "/api/v1/whep/" + stream_id + "/sessions/" + session->id());
       resp.set_body(answer);

       webrtc_sessions_[session->id()] = session;
   }
   ```
2. WHEP DELETE (关闭会话):
   ```
   DELETE /api/v1/whep/{stream_id}/sessions/{session_id}
   ```
3. 最小浏览器测试页面 `examples/player.html`:
   ```html
   <video id="video" autoplay muted></video>
   <script>
   async function play(streamId) {
       const pc = new RTCPeerConnection({
           iceServers: [{urls: 'stun:stun.l.google.com:19302'}]
       });
       pc.addTransceiver('video', {direction: 'recvonly'});
       pc.ontrack = (e) => { document.getElementById('video').srcObject = e.streams[0]; };

       const offer = await pc.createOffer();
       await pc.setLocalDescription(offer);
       // 等待 ICE gathering 完成
       await new Promise(r => { pc.onicegatheringstatechange = () => {
           if (pc.iceGatheringState === 'complete') r();
       }});

       const resp = await fetch(`/api/v1/whep/${streamId}`, {
           method: 'POST',
           headers: {'Content-Type': 'application/sdp'},
           body: pc.localDescription.sdp
       });
       const answer = await resp.text();
       await pc.setRemoteDescription({type: 'answer', sdp: answer});
   }
   play('cam1');
   </script>
   ```
4. 测试: 浏览器打开 player.html，选择一路流，验证出画面

**验收:** 浏览器通过 WHEP 协议能播放摄像头画面

---

#### Day 77 -- 周末: WebRTC 播放调试 (4h)

**实践 (4h)**
1. 解决常见问题:
   - ICE 失败: 检查防火墙，确保 STUN 可达
   - 黑屏: 检查 SPS/PPS 是否正确传递 (WebRTC 需要在 SDP fmtp 中)
   - 花屏: 确保关键帧优先发送 (GOP 缓存)
2. Chrome 调试: `chrome://webrtc-internals/`
   - 检查 ICE candidate pair 状态
   - 检查 inbound-rtp 统计 (packets received, frames decoded)
3. 延迟测量: 从摄像头到浏览器的端到端延迟 (目标 < 500ms)
4. 测试: 同一路流同时用 VLC(RTSP) 和浏览器(WebRTC) 观看

**验收:** RTSP + WebRTC 双协议同时播放同一路摄像头

---

### 第 12 周: WebRTC 完善 (Day 78-84)

---

#### Day 78-79 -- RTP 转封装 + NACK (5h)

**实践 (5h)**
1. RTSP RTP -> WebRTC RTP 转换:
   ```cpp
   class RtpRepackager {
   public:
       // 将从 RTSP 收到的 RTP 包转换为 WebRTC 兼容格式
       RtpPacket repackage(const RtpPacket &src_pkt) {
           RtpPacket dst = src_pkt;
           dst.ssrc = webrtc_ssrc_;        // 使用 WebRTC 会话的 SSRC
           dst.sequence = next_seq_++;     // 重新编号
           dst.timestamp = map_timestamp(src_pkt.timestamp);  // 时间戳映射
           return dst;
       }
   private:
       uint32_t webrtc_ssrc_;
       uint16_t next_seq_{0};
       // 时间戳映射: 从源时间基准转换到 WebRTC 时间基准
       uint32_t map_timestamp(uint32_t src_ts);
   };
   ```
2. NACK 重传支持 (WebRTC 丢包恢复):
   ```cpp
   class NackHandler {
   public:
       // 缓存最近 N 个 RTP 包，用于丢包重传
       void cache_packet(const RtpPacket &pkt);
       // 收到 NACK 请求后查找并重传
       std::optional<RtpPacket> find(uint16_t seq_num) const;
   private:
       static constexpr size_t CACHE_SIZE = 512;
       std::array<RtpPacket, CACHE_SIZE> cache_;
   };
   ```
3. 在 libdatachannel 中处理 NACK:
   ```cpp
   video_track_->onMessage([this](rtc::message_variant msg) {
       // libdatachannel 的 RTCP NACK 处理
       // 查找缓存的包并重发
   });
   ```
4. 测试: 模拟 5% 丢包率，验证 NACK 重传后画面无花屏

---

#### Day 80-81 -- 多观看者 + 资源管理 (5h)

**实践 (5h)**
1. 多个浏览器同时观看同一路流:
   - 每个浏览器创建独立的 `WebRtcSession`
   - 所有 session 订阅同一个 `MediaSource`
   - 数据通过 `shared_ptr<Buffer>` 零拷贝分发
2. WebRTC 会话生命周期管理:
   ```cpp
   class WebRtcManager {
   public:
       std::string create_session(const std::string &stream_id, const std::string &offer);
       void close_session(const std::string &session_id);
       size_t session_count() const;
       // 定期清理: ICE 断开/超时的会话自动回收
       void cleanup_stale_sessions();
   private:
       std::map<std::string, std::shared_ptr<WebRtcSession>> sessions_;
       asio::steady_timer cleanup_timer_;
   };
   ```
3. ICE 断开检测:
   ```cpp
   pc_->onStateChange([this](rtc::PeerConnection::State state) {
       if (state == rtc::PeerConnection::State::Disconnected ||
           state == rtc::PeerConnection::State::Failed) {
           CSK_LOG_I("WebRTC session {} disconnected", id_);
           close();
       }
   });
   ```
4. 测试: 5 个浏览器标签页同时打开同一路流，验证全部播放正常

**验收:** 多个浏览器同时观看，独立连接/断开不影响其他观看者

---

#### Day 82-83 -- 前端播放页面 (5h)

**实践 (5h)**
1. 实现简单的 Web UI (`examples/web/index.html`):
   ```html
   <!-- 设备列表 + 视频播放器 -->
   <div id="app">
       <div class="sidebar">
           <h3>摄像头列表</h3>
           <ul id="stream-list"></ul>
       </div>
       <div class="main">
           <video id="player" autoplay muted controls></video>
           <div id="stats">码率: --  帧率: --  延迟: --</div>
       </div>
   </div>
   ```
2. JavaScript 逻辑:
   ```javascript
   // 从 API 获取流列表
   async function loadStreams() {
       const resp = await fetch('/api/v1/streams');
       const streams = await resp.json();
       streams.forEach(s => {
           const li = document.createElement('li');
           li.textContent = `${s.id} (${s.status})`;
           li.onclick = () => play(s.id);
           document.getElementById('stream-list').appendChild(li);
       });
   }
   // WHEP 播放 (复用 Day 75 的代码)
   async function play(streamId) { /* ... */ }
   // 统计信息更新
   setInterval(async () => {
       const stats = await pc.getStats();
       stats.forEach(s => {
           if (s.type === 'inbound-rtp' && s.kind === 'video') {
               document.getElementById('stats').textContent =
                   `帧率: ${s.framesPerSecond} 丢包: ${s.packetsLost}`;
           }
       });
   }, 1000);
   ```
3. 静态文件服务: HTTP 服务器添加 `/web/` 路径返回 HTML/JS/CSS
4. 测试: 浏览器打开 `http://localhost:8080/web/`，看到设备列表，点击播放

**验收:** Web UI 能列出摄像头并点击播放

---

#### Day 84 -- 周末: v0.4 发布 (4-5h)

**实践 (4-5h)**
1. 完整功能验证:
   - [ ] RTSP 代理: VLC 播放 OK
   - [ ] 转码: H.264 -> H.265, 1080p -> 720p
   - [ ] GB28181: 设备注册 + 点播 + PTZ
   - [ ] WebRTC: 浏览器 WHEP 播放
   - [ ] Web UI: 设备列表 + 点击播放
   - [ ] REST API: 增删查 + 统计
2. 更新 README
3. GitHub Release v0.4.0

**验收:** v0.4 全功能可用 -- RTSP + 转码 + GB28181 + WebRTC + Web UI

---

## 阶段六: 系统集成 + 优化 + 发布 (第 13-16 周 / Day 85-112)

### 第 13 周: 稳定性与性能 (Day 85-91)

---

#### Day 85 -- 内存泄漏检测: ASan (2.5h)

**理论 (20min)**
- AddressSanitizer (ASan): GCC/Clang 内置，检测内存越界、use-after-free、内存泄漏
- 编译选项: `-fsanitize=address -fno-omit-frame-pointer`
- ASan 会拦截 `malloc/free`，在每次内存操作时检查合法性

**实践 (2h)**
1. CMake 添加 ASan 构建模式:
   ```cmake
   if(ENABLE_ASAN)
       target_compile_options(camstreamkit PRIVATE -fsanitize=address -fno-omit-frame-pointer)
       target_link_options(camstreamkit PRIVATE -fsanitize=address)
   endif()
   ```
2. 在 ASan 模式下运行所有单元测试: `cmake -B build-asan -DENABLE_ASAN=ON && ctest --test-dir build-asan`
3. 在 ASan 模式下运行代理: 拉 3 路流，各 2 个客户端，运行 10 分钟
4. 修复 ASan 报告的所有问题 (常见: RTP 包缓存未释放、Session 循环引用)

**验收:** ASan 模式下运行 10 分钟无任何报告

---

#### Day 86 -- ThreadSanitizer + UBSan (2.5h)

**实践 (2.5h)**
1. ThreadSanitizer (TSan) -- 检测数据竞争:
   ```cmake
   if(ENABLE_TSAN)
       target_compile_options(camstreamkit PRIVATE -fsanitize=thread)
       target_link_options(camstreamkit PRIVATE -fsanitize=thread)
   endif()
   ```
2. 常见竞争点:
   - `MediaSource::subscribers_` 读写竞争
   - `StreamStats` 原子操作正确性
   - `MediaHub::sources_` 读写锁使用
3. UndefinedBehaviorSanitizer (UBSan) -- 检测未定义行为:
   - 整数溢出 (RTP sequence 回绕)
   - 空指针解引用
   - 数组越界
4. 修复所有报告的问题

**验收:** TSan + UBSan 均无报告

---

#### Day 87-88 -- 长时间运行稳定性测试 (5h)

**实践 (5h)**
1. 编写自动化压测脚本:
   ```python
   # test/stress/run_stress.py
   import subprocess, time, requests

   # 启动 FFmpeg 模拟 10 路摄像头
   cameras = []
   for i in range(10):
       p = subprocess.Popen(['ffmpeg', '-re', '-stream_loop', '-1',
           '-i', 'test.mp4', '-c:v', 'copy', '-f', 'rtsp',
           f'rtsp://localhost:8554/cam{i}'])
       cameras.append(p)

   # 通过 API 添加到 CamStreamKit
   for i in range(10):
       requests.post('http://localhost:8080/api/v1/streams',
           json={'id': f'cam{i}', 'url': f'rtsp://localhost:8554/cam{i}'})

   # 启动 ffplay 客户端
   players = []
   for i in range(10):
       for j in range(5):
           p = subprocess.Popen(['ffplay', '-nodisp', '-autoexit',
               f'rtsp://localhost:554/stream/cam{i}'])
           players.append(p)

   # 运行 24 小时，每小时检查一次
   for hour in range(24):
       time.sleep(3600)
       stats = requests.get('http://localhost:8080/api/v1/stats').json()
       print(f"Hour {hour}: memory={stats['memory_mb']}MB, streams={stats['active_streams']}")
   ```
2. 监控指标: 内存增长趋势、CPU 占用、丢包率
3. 期望结果: 内存稳定不持续增长 (允许波动 < 10%)

**验收:** 连续运行 24h，内存稳定，无崩溃

---

#### Day 89-90 -- 性能优化 (5h)

**实践 (5h)**
1. CPU profiling (使用 perf / gperftools):
   ```bash
   # Linux perf
   perf record -g ./build/bin/camstreamkit -c config.json
   perf report
   # 或 gperftools
   CPUPROFILE=prof.out ./build/bin/camstreamkit
   google-pprof --text ./build/bin/camstreamkit prof.out
   ```
2. 常见优化点:
   - RTP 打包: 避免频繁小内存分配 -> 使用内存池
   ```cpp
   class RtpPacketPool {
   public:
       std::shared_ptr<RtpPacket> acquire();
       void release(RtpPacket *pkt);
   private:
       std::vector<std::unique_ptr<RtpPacket>> pool_;
       std::mutex mtx_;
   };
   ```
   - 零拷贝分发: `shared_ptr<Buffer>` 确保多个订阅者共享同一份数据
   - 批量发送: 使用 `sendmmsg` (Linux) 一次发送多个 UDP 包
3. 内存优化: 检查 `Buffer` 的引用计数是否及时释放

---

#### Day 91 -- 周末: 性能基准报告 (4h)

**实践 (4h)**
1. 建立性能基线 (benchmark):
   ```
   环境: Ubuntu 22.04, 4 核 8GB, 1Gbps 网络
   ┌─────────────────────┬──────────────┬────────────┬────────────┐
   │ 场景                │ CPU 占用     │ 内存占用   │ 端到端延迟 │
   ├─────────────────────┼──────────────┼────────────┼────────────┤
   │ 1路 H264 1080p      │ < 5%         │ < 30MB     │ < 200ms    │
   │ 10路 各5客户端      │ < 30%        │ < 100MB    │ < 300ms    │
   │ 1路转码 H265 720p   │ < 15% (硬编) │ < 80MB     │ < 500ms    │
   │ 50路 (压测极限)     │ < 80%        │ < 500MB    │ < 1000ms   │
   └─────────────────────┴──────────────┴────────────┴────────────┘
   ```
2. 与 ZLMediaKit 对比 (同等硬件):
   - 代理场景内存占用预期低 30-50%
   - 代码量对比: CamStreamKit < 2W 行 vs ZLMediaKit > 10W 行
3. 将基准数据写入 README

**验收:** 完成性能基线报告，识别并修复主要性能瓶颈

---

### 第 14 周: 运维功能 (Day 92-98)

---

#### Day 92-93 -- Prometheus 指标导出 (5h)

**实践 (5h)**
1. 实现 Prometheus 文本格式导出 (`/metrics` 端点):
   ```cpp
   void ApiServer::handle_metrics(HttpRequest &req, HttpResponse &resp) {
       std::ostringstream ss;
       // 系统指标
       ss << "# HELP csk_uptime_seconds Server uptime\n"
          << "# TYPE csk_uptime_seconds gauge\n"
          << "csk_uptime_seconds " << uptime_seconds() << "\n";
       // 流指标
       for (auto &info : hub_.list_all()) {
           ss << "csk_stream_bitrate_kbps{id=\"" << info.id << "\"} " << info.bitrate << "\n"
              << "csk_stream_fps{id=\"" << info.id << "\"} " << info.fps << "\n"
              << "csk_stream_subscribers{id=\"" << info.id << "\"} " << info.subscriber_count << "\n";
       }
       // 连接指标
       ss << "csk_rtsp_sessions_total " << rtsp_server_.session_count() << "\n"
          << "csk_webrtc_sessions_total " << webrtc_mgr_.session_count() << "\n"
          << "csk_gb28181_devices_online " << sip_server_.online_count() << "\n";
       resp.set_content_type("text/plain; version=0.0.4");
       resp.set_body(ss.str());
   }
   ```
2. Prometheus 配置:
   ```yaml
   # prometheus.yml
   scrape_configs:
     - job_name: 'camstreamkit'
       static_configs:
         - targets: ['localhost:8080']
       metrics_path: '/metrics'
       scrape_interval: 5s
   ```
3. Grafana 仪表板: 导入预配置 dashboard JSON
4. 测试: Prometheus 抓取 OK，Grafana 显示码率/帧率曲线

---

#### Day 94-95 -- 日志 + 信号处理 + 优雅关闭 (5h)

**实践 (5h)**
1. 日志分级 + 文件轮转:
   ```cpp
   void setup_logging(const Config &config) {
       auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
       auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
           config.log_file, 10 * 1024 * 1024, 5);  // 10MB x 5 个文件

       auto logger = std::make_shared<spdlog::logger>("csk",
           spdlog::sinks_init_list{console_sink, file_sink});
       logger->set_level(spdlog::level::from_str(config.log_level));  // "debug"/"info"/"warn"
       logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
       spdlog::set_default_logger(logger);
   }
   ```
2. 优雅关闭 (SIGTERM/SIGINT):
   ```cpp
   asio::signal_set signals(io, SIGINT, SIGTERM);
   signals.async_wait([&](auto, int sig) {
       CSK_LOG_I("Received signal {}, shutting down...", sig);
       // 有序关闭:
       // 1. 停止接受新连接
       rtsp_server_.stop();
       api_server_.stop();
       // 2. 断开所有客户端 (发送 TEARDOWN/BYE)
       hub_.disconnect_all_subscribers();
       // 3. 断开所有摄像头连接
       hub_.stop_all_sources();
       // 4. 停止 io_context
       io.stop();
   });
   ```
3. 命令行参数:
   ```cpp
   int main(int argc, char *argv[]) {
       // -c config.json -d (daemon) --log-level debug
       auto config = parse_args(argc, argv);
   }
   ```
4. 测试: `kill -TERM <pid>` 后所有连接优雅关闭，日志文件完整

---

#### Day 96-97 -- Docker 容器化 (5h)

**实践 (5h)**
1. 多阶段 Dockerfile:
   ```dockerfile
   # 构建阶段
   FROM ubuntu:22.04 AS builder
   RUN apt-get update && apt-get install -y cmake g++ pkg-config \
       libavcodec-dev libavformat-dev libswscale-dev libavutil-dev
   COPY . /src
   RUN cmake -B /src/build -DCMAKE_BUILD_TYPE=Release /src && \
       cmake --build /src/build -j$(nproc)

   # 运行阶段
   FROM ubuntu:22.04
   RUN apt-get update && apt-get install -y \
       libavcodec59 libavformat59 libswscale6 libavutil57
   COPY --from=builder /src/build/bin/camstreamkit /usr/local/bin/
   COPY config.docker.json /etc/camstreamkit/config.json
   EXPOSE 554 5060/udp 8080
   ENTRYPOINT ["camstreamkit", "-c", "/etc/camstreamkit/config.json"]
   ```
2. docker-compose.yml:
   ```yaml
   version: '3'
   services:
     camstreamkit:
       build: .
       ports:
         - "554:554"       # RTSP
         - "5060:5060/udp" # SIP
         - "8080:8080"     # HTTP API
         - "30000-30100:30000-30100/udp"  # RTP 端口范围
       volumes:
         - ./config.json:/etc/camstreamkit/config.json
       restart: unless-stopped
     prometheus:
       image: prom/prometheus
       volumes:
         - ./prometheus.yml:/etc/prometheus/prometheus.yml
       ports:
         - "9090:9090"
   ```
3. 测试: `docker-compose up` 后完整功能可用

---

#### Day 98 -- 周末: 运维功能集成测试 (4h)

**实践 (4h)**
1. Docker 镜像构建 + 功能验证
2. 日志轮转验证: 生成 > 10MB 日志后检查文件轮转
3. 优雅关闭验证: docker stop 后检查日志中的关闭顺序
4. Prometheus + Grafana 监控验证

**验收:** Docker 一键部署，监控完整，优雅关闭正确

---

### 第 15 周: 文档与测试 (Day 99-105)

---

#### Day 99-100 -- 完整 README + 架构文档 (5h)

**实践 (5h)**
1. README.md 结构:
   ```markdown
   # CamStreamKit
   > 轻量级摄像头流媒体代理库

   ## 功能特性
   - RTSP 代理 (按需拉流、断线重连、秒开)
   - H.264/H.265 转码 (软编/硬编)
   - GB28181 国标 (信令/媒体/PTZ)
   - WebRTC 浏览器播放 (WHEP)
   - RESTful API + Web UI

   ## 快速开始
   ### Docker (推荐)
   ### 源码编译

   ## 配置说明
   ## API 文档
   ## 架构设计
   ## 性能指标
   ## 与 ZLMediaKit 的对比
   ## 构建状态 / CI Badge
   ```
2. 架构设计图 (Mermaid):
   ```mermaid
   graph TD
       A[RTSP Camera] -->|RTSP Pull| B[RtspClient]
       C[GB28181 IPC] -->|SIP+RTP| D[SipServer]
       B --> E[MediaHub]
       D --> E
       E -->|RTSP| F[RtspServer -> VLC]
       E -->|WebRTC| G[WHEP -> Browser]
       E -->|Transcode| H[Transcoder]
       H --> E
   ```
3. API 文档: 每个端点的请求/响应示例

---

#### Day 101-102 -- Doxygen + 代码注释 (5h)

**实践 (5h)**
1. 添加 Doxyfile:
   ```
   PROJECT_NAME = CamStreamKit
   INPUT = include/ src/
   RECURSIVE = YES
   GENERATE_HTML = YES
   EXTRACT_ALL = YES
   ```
2. 为所有公共接口添加 Doxygen 注释:
   ```cpp
   /// @brief 媒体流源的基类
   /// @details 所有输入源 (RTSP/GB28181/文件) 都继承此类，
   ///          通过 dispatch_frame() 向订阅者分发媒体帧
   class MediaSource {
       /// @brief 添加媒体帧订阅者
       /// @param sink 订阅者的弱引用 (避免循环引用)
       /// @note 第一个订阅者触发按需拉流
       void add_subscriber(std::weak_ptr<IMediaSink> sink);
   };
   ```
3. 生成 HTML 文档: `doxygen Doxyfile` -> 上传到 GitHub Pages

---

#### Day 103-104 -- 集成测试套件 (5h)

**实践 (5h)**
1. 用 FFmpeg 模拟摄像头的自动化测试:
   ```cpp
   // tests/integration/test_rtsp_proxy.cpp
   TEST_CASE("RTSP proxy end-to-end") {
       // 1. 启动 FFmpeg RTSP 源
       auto ffmpeg = start_ffmpeg_rtsp_source("test.mp4", 8554);
       // 2. 启动 CamStreamKit
       auto server = start_server(config);
       // 3. 添加摄像头
       auto resp = http_post("/api/v1/streams", {{"id","test"},{"url","rtsp://localhost:8554/cam"}});
       REQUIRE(resp.status == 200);
       // 4. 等待流就绪
       wait_until([&] { return http_get("/api/v1/streams/test").json()["status"] == "online"; });
       // 5. 用 FFprobe 验证输出流
       auto probe = ffprobe("rtsp://localhost:554/stream/test");
       REQUIRE(probe.codec == "h264");
       REQUIRE(probe.width == 1920);
   }
   ```
2. GB28181 模拟设备测试
3. CI 中运行集成测试

---

#### Day 105 -- 周末: 演示视频 (4h)

**实践 (4h)**
1. 录制演示视频 (3-5 分钟):
   - 场景1: 通过 REST API 添加 RTSP 摄像头，VLC 播放
   - 场景2: 浏览器 WebRTC 实时播放
   - 场景3: GB28181 设备注册 + 点播
   - 场景4: 转码 H.264 -> H.265
   - 场景5: Grafana 监控面板
2. 上传到 YouTube 或 Bilibili，链接放入 README

**验收:** 完整的文档、API 文档、测试套件、演示视频

---

### 第 16 周: 开源发布 + 面试准备 (Day 106-112)

---

#### Day 106-107 -- GitHub Release v1.0.0 (5h)

**实践 (5h)**
1. 代码清理:
   - 删除调试代码和 TODO 注释
   - 统一代码风格: `clang-format -i src/**/*.cpp src/**/*.h`
   - License 文件 (推荐 MIT)
2. CHANGELOG.md:
   ```markdown
   ## v1.0.0 (2026-xx-xx)
   - RTSP proxy with on-demand pull and reconnection
   - H.264/H.265 transcoding with hardware acceleration
   - GB28181 SIP signaling, PS demux, PTZ control
   - WebRTC browser playback via WHEP
   - RESTful API + Web UI
   - Docker deployment
   - Prometheus metrics
   ```
3. GitHub Release:
   ```bash
   git tag -a v1.0.0 -m "CamStreamKit v1.0.0 - First stable release"
   git push origin v1.0.0
   gh release create v1.0.0 --title "v1.0.0" --notes-file CHANGELOG.md
   ```
4. Docker Hub 推送: `docker push camstreamkit/camstreamkit:1.0.0`

---

#### Day 108-109 -- 技术博客 (5h)

**实践 (5h)**
1. 撰写 2-3 篇技术博客:
   - **博客1: 为什么要自研流媒体代理** (1500字)
     - ZLMediaKit 的痛点分析
     - CamStreamKit 的设计理念
     - 架构选择和权衡
   - **博客2: GB28181 PS 流解封装踩坑记录** (2000字)
     - PS 封装格式详解
     - 常见坑: PS 头长度可变、PTS/DTS 解析、时间戳回绕
     - 与各品牌摄像头的兼容性问题
   - **博客3: C++17 实现轻量 WebRTC 服务端** (2000字)
     - WHEP 协议介绍
     - libdatachannel 集成经验
     - RTP 转封装细节
2. 发布平台: 知乎 / 掘金 / CSDN / 个人博客

---

#### Day 110-111 -- 简历 + 面试准备 (5h)

**实践 (5h)**
1. 简历项目描述:
   ```
   CamStreamKit -- 轻量级摄像头流媒体代理库 (C++17, GitHub star xxx)
   - 独立设计并实现支持 RTSP/GB28181/WebRTC 的摄像头代理库
   - 基于 Asio 实现异步网络框架，支持 50+ 路并发代理
   - 集成 FFmpeg 实现 H.264/H.265 转码，支持 NVENC 硬件加速
   - 自研 GB28181 SIP 信令栈，实现设备注册/点播/PTZ 控制
   - 通过 WHEP 协议实现 WebRTC 浏览器无插件播放
   - 代码量 ~1.5W 行，单元测试覆盖率 > 70%
   ```
2. 面试深挖问题准备:
   - **Q: 为什么不直接用 ZLMediaKit?**
     - A: 场景聚焦 + 代码量差 5 倍 + 内置 GB28181 + 现代 C++17
   - **Q: 如何处理摄像头断线?**
     - A: 指数退避重连 + 健康检测 + 按需拉流避免无效连接
   - **Q: GB28181 PS 解封装有什么坑?**
     - A: PS 头长度不固定 / PTS 解析字节位不连续 / 某些摄像头不发 System Header
   - **Q: 多路转发如何做到零拷贝?**
     - A: shared_ptr<Buffer> 引用计数 + 每个客户端独立 RTP 打包
   - **Q: WebRTC 延迟如何优化?**
     - A: GOP 缓存秒开 + zerolatency 编码 + NACK 丢包重传
   - **Q: 最大支持多少路并发?**
     - A: 50 路代理 (4 核 8G)，瓶颈在带宽而非 CPU

---

#### Day 112 -- 最终日: 项目收尾 (4h)

**实践 (4h)**
1. GitHub 仓库最终检查:
   - [ ] README 完整 (架构图 + Quick Start + API)
   - [ ] CI 全绿 (编译 + 测试 + ASan)
   - [ ] Docker 镜像可用
   - [ ] License 文件存在
   - [ ] CHANGELOG 更新
   - [ ] 演示视频链接有效
2. 在 Reddit r/cpp 或 V2EX 分享项目
3. 回顾 16 周学到的核心能力:
   - 现代 C++17 系统编程
   - 异步网络框架设计
   - 流媒体协议栈 (RTSP/RTP/SDP/PS/SIP)
   - FFmpeg 编解码集成
   - WebRTC 服务端实现
   - 项目工程化 (CI/CD/Docker/监控/文档)

**最终验收:** CamStreamKit v1.0.0 开源发布，简历和面试准备完成

---

## 每日时间建议

- **工作日**: 2.5-3h (理论 + 编码)
- **周末**: 4-5h (综合项目 + 测试)
- **总投入**: ~350h

## 关键依赖库

| 库 | 用途 | 许可证 |
|---|---|---|
| standalone Asio | 异步网络 I/O | Boost |
| spdlog | 日志 | MIT |
| nlohmann/json | JSON 解析 | MIT |
| Catch2 | 单元测试 | BSL-1.0 |
| FFmpeg (libav*) | 转码 | LGPL/GPL |
| libdatachannel | WebRTC | MPL-2.0 |
| PJSIP 或自研 | SIP (GB28181) | GPL/自研 |

## 推荐学习资源

- RFC 2326 (RTSP) / RFC 3550 (RTP) / RFC 6184 (H.264 RTP)
- GB/T 28181-2022 国标文档
- "Erta's Blog" RTSP 系列 (中文，搜索"从零实现 RTSP")
- FFmpeg 官方文档: https://ffmpeg.org/doxygen/trunk/
- ZLMediaKit 源码 (作为参考，不是照抄)
- libdatachannel 文档: https://github.com/paullouisageneau/libdatachannel
