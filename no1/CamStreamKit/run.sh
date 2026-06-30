#!/bin/bash
set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
BINARY="$BUILD_DIR/src/camstreamkit"
CONFIG="$PROJECT_DIR/config.json"

echo "=== CamStreamKit 编译运行脚本 ==="
echo ""

# 编译
echo "[1/2] 编译中..."
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release "$PROJECT_DIR" > /dev/null 2>&1
cmake --build "$BUILD_DIR" -j$(nproc)
echo ""
echo "[2/2] 启动服务..."
echo "  HTTP API:  http://localhost:8080/"
echo "  RTSP:      rtsp://localhost:554/stream/{id}"
echo "  Metrics:   http://localhost:8080/metrics"
echo ""

# 运行
exec "$BINARY" -c "$CONFIG"
