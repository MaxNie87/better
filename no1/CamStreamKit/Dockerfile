# Build stage
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    cmake g++ pkg-config git \
    && rm -rf /var/lib/apt/lists/*

COPY . /src
RUN cmake -B /src/build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF /src && \
    cmake --build /src/build -j$(nproc)

# Runtime stage
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/bin/camstreamkit /usr/local/bin/
COPY config.json /etc/camstreamkit/config.json

EXPOSE 554 5060/udp 8080

ENTRYPOINT ["camstreamkit", "-c", "/etc/camstreamkit/config.json"]
