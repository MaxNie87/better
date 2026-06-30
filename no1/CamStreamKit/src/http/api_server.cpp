#include "http/api_server.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

#include "core/logger.h"
#include "rtsp/rtsp_client.h"

using json = nlohmann::json;

namespace {
const char *kIndexHtml = R"HTMLPAGE(
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CamStreamKit - 流媒体管理</title>
    <style>
        :root{--bg:#0f172a;--card:#1e293b;--card-hover:#334155;--primary:#3b82f6;--primary-hover:#2563eb;--danger:#ef4444;--danger-hover:#dc2626;--success:#22c55e;--warning:#f59e0b;--text:#f1f5f9;--text-muted:#94a3b8;--border:#334155;--radius:12px}
        *{margin:0;padding:0;box-sizing:border-box}
        body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;background:var(--bg);color:var(--text);min-height:100vh}
        .header{display:flex;align-items:center;justify-content:space-between;padding:20px 32px;border-bottom:1px solid var(--border);background:var(--card)}
        .header h1{font-size:22px;font-weight:700;background:linear-gradient(135deg,#3b82f6,#8b5cf6);-webkit-background-clip:text;-webkit-text-fill-color:transparent}
        .header-stats{display:flex;gap:24px;font-size:14px;color:var(--text-muted)}
        .header-stats span{display:flex;align-items:center;gap:6px}
        .dot{width:8px;height:8px;border-radius:50%}.dot-green{background:var(--success)}.dot-yellow{background:var(--warning)}
        .main{max-width:1200px;margin:0 auto;padding:32px}
        .add-section{background:var(--card);border-radius:var(--radius);padding:24px;margin-bottom:32px;border:1px solid var(--border)}
        .add-section h2{font-size:16px;margin-bottom:16px;color:var(--text-muted);font-weight:500}
        .form-row{display:flex;gap:12px;flex-wrap:wrap}
        .form-row input{flex:1;min-width:200px;padding:12px 16px;background:var(--bg);border:1px solid var(--border);border-radius:8px;color:var(--text);font-size:14px;outline:none;transition:border-color .2s}
        .form-row input:focus{border-color:var(--primary)}
        .form-row input::placeholder{color:var(--text-muted)}
        .btn{padding:12px 24px;border:none;border-radius:8px;font-size:14px;font-weight:600;cursor:pointer;transition:all .2s;display:inline-flex;align-items:center;gap:6px}
        .btn-primary{background:var(--primary);color:#fff}.btn-primary:hover{background:var(--primary-hover);transform:translateY(-1px)}
        .btn-danger{background:var(--danger);color:#fff}.btn-danger:hover{background:var(--danger-hover)}
        .btn-ghost{background:transparent;color:var(--text-muted);border:1px solid var(--border)}.btn-ghost:hover{background:var(--card-hover);color:var(--text)}
        .streams-header{display:flex;align-items:center;justify-content:space-between;margin-bottom:16px}
        .streams-header h2{font-size:18px;font-weight:600}
        .stream-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(340px,1fr));gap:16px}
        .stream-card{background:var(--card);border:1px solid var(--border);border-radius:var(--radius);padding:20px;transition:all .2s;position:relative}
        .stream-card:hover{border-color:var(--primary);transform:translateY(-2px);box-shadow:0 8px 24px rgba(0,0,0,.3)}
        .stream-card-header{display:flex;align-items:center;justify-content:space-between;margin-bottom:12px}
        .stream-id{font-size:16px;font-weight:700;display:flex;align-items:center;gap:8px}
        .status-badge{display:inline-flex;align-items:center;gap:4px;padding:3px 10px;border-radius:20px;font-size:12px;font-weight:500}
        .status-online{background:rgba(34,197,94,.15);color:var(--success)}
        .status-offline{background:rgba(239,68,68,.15);color:var(--danger)}
        .status-connecting{background:rgba(245,158,11,.15);color:var(--warning)}
        .stream-info{display:flex;flex-direction:column;gap:8px;margin-bottom:16px}
        .info-row{display:flex;align-items:center;gap:8px;font-size:13px;color:var(--text-muted)}
        .info-row .label{min-width:60px}
        .info-row .value{flex:1;color:var(--text);word-break:break-all;font-family:'SF Mono','Menlo',monospace;font-size:12px;background:var(--bg);padding:4px 8px;border-radius:4px}
        .stream-footer{display:flex;align-items:center;justify-content:space-between;padding-top:12px;border-top:1px solid var(--border)}
        .viewer-count{font-size:13px;color:var(--text-muted);display:flex;align-items:center;gap:4px}
        .btn-sm{padding:6px 12px;font-size:12px;border-radius:6px}
        .empty-state{text-align:center;padding:60px 20px;color:var(--text-muted)}
        .empty-state svg{width:64px;height:64px;opacity:.5;margin-bottom:16px}
        .empty-state p{font-size:15px}
        .toast{position:fixed;bottom:24px;right:24px;padding:14px 20px;border-radius:8px;font-size:14px;font-weight:500;z-index:1000;transform:translateY(100px);opacity:0;transition:all .3s}
        .toast.show{transform:translateY(0);opacity:1}
        .toast-success{background:var(--success);color:#fff}
        .toast-error{background:var(--danger);color:#fff}
        .modal-overlay{display:none;position:fixed;top:0;left:0;right:0;bottom:0;background:rgba(0,0,0,.8);z-index:900;align-items:center;justify-content:center}
        .modal-overlay.active{display:flex}
        .modal-content{background:var(--card);border-radius:var(--radius);padding:24px;max-width:800px;width:90%;position:relative}
        .modal-content h3{margin-bottom:16px;font-size:18px}
        .modal-close{position:absolute;top:16px;right:16px;background:none;border:none;color:var(--text-muted);font-size:24px;cursor:pointer}
        .modal-close:hover{color:var(--text)}
        .btn-play{background:var(--success);color:#fff}
        .btn-play:hover{opacity:.9}
    </style>
</head>
<body>
    <div class="header">
        <h1>CamStreamKit</h1>
        <div class="header-stats">
            <span><span class="dot dot-green"></span><span id="online-count">0</span> 在线</span>
            <span><span class="dot dot-yellow"></span><span id="total-count">0</span> 总计</span>
        </div>
    </div>
    <div class="main">
        <div class="add-section">
            <h2>添加摄像头流</h2>
            <div class="form-row">
                <input type="text" id="input-id" placeholder="流 ID (如: cam1)">
                <input type="text" id="input-url" placeholder="RTSP 地址 (如: rtsp://admin:123456@192.168.1.100/stream1)">
                <button class="btn btn-primary" onclick="addStream()">+ 添加</button>
            </div>
        </div>
        <div class="streams-header">
            <h2>流列表</h2>
            <button class="btn btn-ghost btn-sm" onclick="loadStreams()">刷新</button>
        </div>
        <div id="stream-list" class="stream-grid"></div>
    </div>
    <div class="modal-overlay" id="player-modal"><div class="modal-content"><button class="modal-close" onclick="closePlayer()">&times;</button><h3 id="player-title">播放</h3><video id="player-video" autoplay playsinline muted style="width:100%;background:#000;border-radius:8px;min-height:300px"></video><div id="player-status" style="margin-top:12px;font-size:13px;color:var(--text-muted)">等待连接...</div></div></div>
    <div class="toast" id="toast"></div>
    <script>
    const API_BASE='';
    let streams=[];
    async function loadStreams(){try{const r=await fetch(API_BASE+'/api/v1/streams');if(!r.ok)throw new Error('HTTP '+r.status);streams=await r.json();renderStreams();updateStats()}catch(e){document.getElementById('stream-list').innerHTML='<div class="empty-state" style="grid-column:1/-1"><p>无法连接服务器</p></div>'}}
    function renderStreams(){const c=document.getElementById('stream-list');if(!streams.length){c.innerHTML='<div class="empty-state" style="grid-column:1/-1"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><rect x="2" y="3" width="20" height="14" rx="2"/><line x1="8" y1="21" x2="16" y2="21"/><line x1="12" y1="17" x2="12" y2="21"/></svg><p>暂无摄像头流</p><p style="margin-top:8px;font-size:13px">在上方添加第一个 RTSP 摄像头</p></div>';return}
    c.innerHTML=streams.map(s=>{const sc=s.status==='online'?'status-online':s.status==='connecting'?'status-connecting':'status-offline';const st=s.status==='online'?'在线':s.status==='connecting'?'连接中':'离线';const rtsp='rtsp://'+location.hostname+':554/stream/'+s.id;return '<div class="stream-card"><div class="stream-card-header"><div class="stream-id">'+s.id+'</div><span class="status-badge '+sc+'">'+st+'</span></div><div class="stream-info"><div class="info-row"><span class="label">RTSP</span><span class="value">'+rtsp+'</span></div><div class="info-row"><span class="label">WebRTC</span><span class="value">POST /whep/'+s.id+'</span></div><div class="info-row"><span class="label">编码</span><span class="value">'+(s.codec||'H264')+' | '+(s.bitrate_kbps||0)+' kbps</span></div></div><div class="stream-footer"><div class="viewer-count">'+(s.subscribers||0)+' 观看</div><div style="display:flex;gap:8px"><button class="btn btn-play btn-sm" onclick="playStream(\''+s.id+'\')">播放</button><button class="btn btn-ghost btn-sm" onclick="copyUrl(\''+rtsp+'\')">复制</button><button class="btn btn-danger btn-sm" onclick="deleteStream(\''+s.id+'\')">删除</button></div></div></div>'}).join('')}
    function updateStats(){document.getElementById('online-count').textContent=streams.filter(s=>s.status==='online').length;document.getElementById('total-count').textContent=streams.length}
    async function addStream(){const id=document.getElementById('input-id').value.trim();const url=document.getElementById('input-url').value.trim();if(!id||!url){showToast('请填写 ID 和 URL','error');return}try{const r=await fetch(API_BASE+'/api/v1/streams',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id,url})});if(r.ok){document.getElementById('input-id').value='';document.getElementById('input-url').value='';showToast('已添加 '+id,'success');loadStreams()}else{const e=await r.json().catch(()=>({}));showToast(e.error||'添加失败','error')}}catch(e){showToast('网络错误','error')}}
    async function deleteStream(id){if(!confirm('确认删除 "'+id+'"？'))return;try{const r=await fetch(API_BASE+'/api/v1/streams/'+id,{method:'DELETE'});if(r.ok||r.status===204){showToast('已删除 '+id,'success');loadStreams()}else showToast('删除失败','error')}catch(e){showToast('网络错误','error')}}
    function copyUrl(u){navigator.clipboard?navigator.clipboard.writeText(u).then(()=>showToast('已复制','success')):showToast('复制失败','error')}
    function showToast(m,t){const el=document.getElementById('toast');el.textContent=m;el.className='toast toast-'+t+' show';setTimeout(()=>el.classList.remove('show'),3000)}
    document.getElementById('input-url').addEventListener('keydown',e=>{if(e.key==='Enter')addStream()});
    document.getElementById('input-id').addEventListener('keydown',e=>{if(e.key==='Enter')document.getElementById('input-url').focus()});
    // WebRTC/WHEP Player
    async function playStream(id){closePlayer();document.getElementById('player-modal').classList.add('active');document.getElementById('player-title').textContent='播放: '+id;document.getElementById('player-status').textContent='正在建立 WebRTC 连接...';try{const pc=new RTCPeerConnection({iceServers:[{urls:'stun:stun.l.google.com:19302'}]});window._pc=pc;pc.addTransceiver('video',{direction:'recvonly'});pc.ontrack=ev=>{document.getElementById('player-video').srcObject=ev.streams[0];document.getElementById('player-status').textContent='已连接 - 正在播放'};pc.oniceconnectionstatechange=()=>{const s=pc.iceConnectionState;if(s==='disconnected'||s==='failed')document.getElementById('player-status').textContent='连接断开: '+s};const offer=await pc.createOffer();await pc.setLocalDescription(offer);await new Promise(r=>{if(pc.iceGatheringState==='complete')r();else{pc.onicegatheringstatechange=()=>{if(pc.iceGatheringState==='complete')r()};setTimeout(r,3000)}});const resp=await fetch('/whep/'+id,{method:'POST',headers:{'Content-Type':'application/sdp'},body:pc.localDescription.sdp});if(!resp.ok){document.getElementById('player-status').textContent='连接失败: '+resp.status;return}const answer=await resp.text();await pc.setRemoteDescription({type:'answer',sdp:answer});document.getElementById('player-status').textContent='等待视频流...'}catch(e){document.getElementById('player-status').textContent='错误: '+e.message}}
    function closePlayer(){document.getElementById('player-modal').classList.remove('active');document.getElementById('player-video').srcObject=null;if(window._pc){window._pc.close();window._pc=null}}
    document.getElementById('player-modal').addEventListener('click',e=>{if(e.target===e.currentTarget)closePlayer()});
    loadStreams();setInterval(loadStreams,5000);
    </script>
</body>
</html>
)HTMLPAGE";
}  // anonymous namespace

namespace csk {

// --- HttpRequest ---

std::string HttpRequest::path_param(const std::string &prefix) const {
    if (path.find(prefix) == 0) {
        return path.substr(prefix.size());
    }
    return "";
}

// --- HttpResponse ---

void HttpResponse::set_json(const std::string &json) {
    body = json;
    headers["Content-Type"] = "application/json";
}

void HttpResponse::set_status(int code, const std::string &text) {
    status_code = code;
    if (!text.empty()) {
        status_text = text;
    } else {
        switch (code) {
            case 200: status_text = "OK"; break;
            case 201: status_text = "Created"; break;
            case 204: status_text = "No Content"; break;
            case 400: status_text = "Bad Request"; break;
            case 404: status_text = "Not Found"; break;
            case 500: status_text = "Internal Server Error"; break;
            default: status_text = "Unknown"; break;
        }
    }
}

std::string HttpResponse::serialize() const {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    for (auto &[key, value] : headers) {
        ss << key << ": " << value << "\r\n";
    }
    ss << "Content-Length: " << body.size() << "\r\n";
    ss << "Access-Control-Allow-Origin: *\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n";
    ss << body;
    return ss.str();
}

// --- HttpSession ---

HttpSession::HttpSession(asio::ip::tcp::socket socket, RequestHandler handler)
    : Session(std::move(socket)), handler_(std::move(handler)) {}

void HttpSession::start() { do_read(); }

void HttpSession::on_data(SpanU8 data) {
    buffer_.append(reinterpret_cast<const char *>(data.data()), data.size());

    auto header_end = buffer_.find("\r\n\r\n");
    if (header_end == std::string::npos) return;

    // Check for Content-Length
    size_t content_length = 0;
    auto cl_pos = buffer_.find("Content-Length:");
    if (cl_pos != std::string::npos && cl_pos < header_end) {
        auto start = cl_pos + 15;
        while (start < buffer_.size() && buffer_[start] == ' ') start++;
        auto end = buffer_.find("\r\n", start);
        content_length = std::stoul(buffer_.substr(start, end - start));
    }

    if (buffer_.size() < header_end + 4 + content_length) return;

    process_request();
}

void HttpSession::process_request() {
    HttpRequest req;

    auto first_line_end = buffer_.find("\r\n");
    std::string first_line = buffer_.substr(0, first_line_end);

    std::istringstream line_stream(first_line);
    std::string version;
    line_stream >> req.method >> req.path >> version;

    // Parse headers
    auto header_end = buffer_.find("\r\n\r\n");
    std::string headers_section = buffer_.substr(first_line_end + 2, header_end - first_line_end - 2);
    std::istringstream headers_stream(headers_section);
    std::string header_line;
    while (std::getline(headers_stream, header_line)) {
        if (!header_line.empty() && header_line.back() == '\r') header_line.pop_back();
        auto colon = header_line.find(':');
        if (colon != std::string::npos) {
            std::string key = header_line.substr(0, colon);
            std::string value = header_line.substr(colon + 1);
            while (!value.empty() && value[0] == ' ') value.erase(0, 1);
            req.headers[key] = value;
        }
    }

    // Body
    size_t content_length = 0;
    if (req.headers.count("Content-Length")) {
        content_length = std::stoul(req.headers["Content-Length"]);
    }
    if (content_length > 0) {
        req.body = buffer_.substr(header_end + 4, content_length);
    }

    HttpResponse resp;
    handler_(req, resp);

    send(resp.serialize());
    close();
}

// --- ApiServer ---

ApiServer::ApiServer(asio::io_context &io, uint16_t port, MediaHub &hub,
                     std::shared_ptr<WhepServer> whep)
    : hub_(hub), io_(io), whep_(std::move(whep)) {
    tcp_server_ = std::make_unique<TcpServer>(
        io, port, [this](asio::ip::tcp::socket socket) -> std::shared_ptr<Session> {
            return std::make_shared<HttpSession>(
                std::move(socket),
                [this](const HttpRequest &req, HttpResponse &resp) {
                    handle_request(req, resp);
                });
        });
}

void ApiServer::start() {
    tcp_server_->start();
    CSK_LOG_I("API Server started on HTTP");
}

void ApiServer::stop() { tcp_server_->stop(); }

void ApiServer::handle_request(const HttpRequest &req, HttpResponse &resp) {
    resp.headers["Content-Type"] = "application/json";

    // Handle CORS preflight
    if (req.method == "OPTIONS") {
        resp.set_status(204);
        resp.headers["Access-Control-Allow-Methods"] = "GET, POST, DELETE, OPTIONS";
        resp.headers["Access-Control-Allow-Headers"] = "Content-Type";
        resp.body.clear();
        return;
    }

    if (req.method == "GET" && (req.path == "/" || req.path == "/index.html")) {
        resp.headers["Content-Type"] = "text/html; charset=utf-8";
        resp.body = kIndexHtml;
        return;
    } else if (req.method == "GET" && req.path == "/api/v1/streams") {
        handle_list_streams(req, resp);
    } else if (req.method == "POST" && req.path == "/api/v1/streams") {
        handle_add_stream(req, resp);
    } else if (req.method == "GET" && req.path.find("/api/v1/streams/") == 0) {
        handle_get_stream(req, resp);
    } else if (req.method == "DELETE" && req.path.find("/api/v1/streams/") == 0) {
        handle_delete_stream(req, resp);
    } else if (req.method == "POST" && req.path.find("/whep/") == 0) {
        handle_whep_offer(req, resp);
    } else if (req.method == "DELETE" && req.path.find("/whep/resource/") == 0) {
        handle_whep_delete(req, resp);
    } else if (req.method == "GET" && req.path == "/metrics") {
        handle_metrics(req, resp);
    } else if (req.method == "GET" && req.path == "/api/v1/version") {
        nlohmann::json j;
        j["version"] = "1.0.0";
        j["name"] = "CamStreamKit";
        resp.set_json(j.dump());
    } else {
        resp.set_status(404);
        resp.set_json(R"({"error":"not found"})");
    }
}

void ApiServer::handle_list_streams(const HttpRequest &, HttpResponse &resp) {
    auto streams = hub_.list_all();
    nlohmann::json arr = nlohmann::json::array();
    for (auto &s : streams) {
        nlohmann::json j;
        j["id"] = s.id;
        j["status"] = s.status;
        j["codec"] = s.codec == CodecType::H265 ? "H265" : "H264";
        j["subscribers"] = s.subscriber_count;
        j["bitrate_kbps"] = s.bitrate_kbps;
        j["fps"] = s.fps;
        arr.push_back(j);
    }
    resp.set_json(arr.dump());
}

void ApiServer::handle_add_stream(const HttpRequest &req, HttpResponse &resp) {
    try {
        auto j = nlohmann::json::parse(req.body);
        std::string id = j.value("id", "");
        std::string url = j.value("url", "");

        if (id.empty() || url.empty()) {
            resp.set_status(400);
            resp.set_json(R"({"error":"id and url are required"})");
            return;
        }

        if (hub_.find(id)) {
            resp.set_status(409, "Conflict");
            resp.set_json(R"({"error":"stream already exists"})");
            return;
        }

        RtspClient::Config config;
        config.url = url;
        config.transport = TransportMode::TCP;

        auto source = std::make_shared<RtspMediaSource>(id, config, io_);
        hub_.add_source(source);
        source->start();

        nlohmann::json result;
        result["id"] = id;
        result["status"] = "connecting";
        resp.set_status(201);
        resp.set_json(result.dump());

    } catch (const std::exception &e) {
        resp.set_status(400);
        resp.set_json(std::string(R"({"error":")") + e.what() + R"("})");
    }
}

void ApiServer::handle_get_stream(const HttpRequest &req, HttpResponse &resp) {
    std::string id = req.path.substr(std::string("/api/v1/streams/").size());
    auto source = hub_.find(id);
    if (!source) {
        resp.set_status(404);
        resp.set_json(R"({"error":"stream not found"})");
        return;
    }

    nlohmann::json j;
    j["id"] = source->id();
    j["status"] = source->status();
    j["codec"] = source->video_codec() == CodecType::H265 ? "H265" : "H264";
    j["subscribers"] = source->subscriber_count();
    j["bitrate_kbps"] = source->stats().current_bitrate_kbps.load();
    j["fps"] = source->stats().current_fps.load();
    j["frames_received"] = source->stats().frames_received.load();
    j["bytes_received"] = source->stats().bytes_received.load();
    resp.set_json(j.dump());
}

void ApiServer::handle_delete_stream(const HttpRequest &req, HttpResponse &resp) {
    std::string id = req.path.substr(std::string("/api/v1/streams/").size());
    auto source = hub_.find(id);
    if (!source) {
        resp.set_status(404);
        resp.set_json(R"({"error":"stream not found"})");
        return;
    }

    hub_.remove_source(id);
    resp.set_status(204);
    resp.body.clear();
}

void ApiServer::handle_whep_offer(const HttpRequest &req, HttpResponse &resp) {
    if (!whep_) {
        resp.set_status(503, "Service Unavailable");
        resp.set_json(R"({"error":"WebRTC not enabled"})");
        return;
    }

    std::string stream_id = req.path.substr(std::string("/whep/").size());
    if (stream_id.empty()) {
        resp.set_status(400);
        resp.set_json(R"({"error":"stream_id required"})");
        return;
    }

    auto answer = whep_->handle_offer(stream_id, req.body);
    if (answer.sdp.empty()) {
        resp.set_status(404);
        resp.set_json(R"({"error":"stream not found or offer failed"})");
        return;
    }

    resp.set_status(201);
    resp.headers["Content-Type"] = "application/sdp";
    resp.headers["Location"] = "/whep/resource/" + answer.resource_id;
    resp.body = answer.sdp;
}

void ApiServer::handle_whep_delete(const HttpRequest &req, HttpResponse &resp) {
    if (!whep_) {
        resp.set_status(503, "Service Unavailable");
        resp.set_json(R"({"error":"WebRTC not enabled"})");
        return;
    }

    std::string resource_id = req.path.substr(std::string("/whep/resource/").size());
    if (whep_->handle_delete(resource_id)) {
        resp.set_status(204);
        resp.body.clear();
    } else {
        resp.set_status(404);
        resp.set_json(R"({"error":"session not found"})");
    }
}

void ApiServer::handle_metrics(const HttpRequest &, HttpResponse &resp) {
    std::ostringstream ss;

    auto streams = hub_.list_all();
    ss << "# HELP csk_streams_total Total number of streams\n"
       << "# TYPE csk_streams_total gauge\n"
       << "csk_streams_total " << streams.size() << "\n\n";

    for (auto &s : streams) {
        ss << "csk_stream_bitrate_kbps{id=\"" << s.id << "\"} " << s.bitrate_kbps << "\n"
           << "csk_stream_fps{id=\"" << s.id << "\"} " << s.fps << "\n"
           << "csk_stream_subscribers{id=\"" << s.id << "\"} " << s.subscriber_count << "\n";
    }

    resp.headers["Content-Type"] = "text/plain; version=0.0.4";
    resp.body = ss.str();
}

}  // namespace csk
