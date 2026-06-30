#include <catch2/catch_test_macros.hpp>

#include "core/gop_cache.h"
#include "core/media_hub.h"
#include "core/media_source.h"

using namespace csk;

class MockSource : public MediaSource {
public:
    MockSource(const std::string &id) : id_(id) {}
    std::string id() const override { return id_; }
    CodecType video_codec() const override { return CodecType::H264; }
    std::string generate_sdp(const std::string &) override { return "v=0\r\n"; }

    void push_frame(const MediaFrame &frame) { dispatch_frame(frame); }

private:
    std::string id_;
};

class MockSink : public IMediaSink {
public:
    void on_media_frame(const MediaFrame &frame) override {
        frames_received++;
        last_frame = frame;
    }

    int frames_received = 0;
    MediaFrame last_frame;
};

TEST_CASE("MediaHub add and find source", "[media_hub]") {
    auto &hub = MediaHub::instance();
    hub.stop_all();

    auto source = std::make_shared<MockSource>("test1");
    hub.add_source(source);

    REQUIRE(hub.source_count() == 1);
    REQUIRE(hub.find("test1") != nullptr);
    REQUIRE(hub.find("nonexistent") == nullptr);

    hub.remove_source("test1");
    REQUIRE(hub.source_count() == 0);
}

TEST_CASE("MediaHub list all", "[media_hub]") {
    auto &hub = MediaHub::instance();
    hub.stop_all();

    hub.add_source(std::make_shared<MockSource>("cam1"));
    hub.add_source(std::make_shared<MockSource>("cam2"));

    auto list = hub.list_all();
    REQUIRE(list.size() == 2);

    hub.stop_all();
}

TEST_CASE("MediaSource dispatch to subscribers", "[media_source]") {
    auto source = std::make_shared<MockSource>("test");
    auto sink = std::make_shared<MockSink>();

    source->add_subscriber(sink);
    REQUIRE(source->subscriber_count() == 1);

    MediaFrame frame;
    frame.type = MediaFrame::VIDEO_KEY;
    frame.timestamp = 90000;
    frame.data = make_buffer(100);

    source->push_frame(frame);
    REQUIRE(sink->frames_received == 1);
    REQUIRE(sink->last_frame.timestamp == 90000);
}

TEST_CASE("MediaSource remove subscriber", "[media_source]") {
    auto source = std::make_shared<MockSource>("test");
    auto sink = std::make_shared<MockSink>();

    source->add_subscriber(sink);
    REQUIRE(source->subscriber_count() == 1);

    source->remove_subscriber(sink.get());
    REQUIRE(source->subscriber_count() == 0);
}

TEST_CASE("GopCache stores and returns frames", "[gop_cache]") {
    GopCache cache;

    MediaFrame key_frame;
    key_frame.type = MediaFrame::VIDEO_KEY;
    key_frame.timestamp = 0;
    cache.on_frame(key_frame);

    MediaFrame p_frame;
    p_frame.type = MediaFrame::VIDEO_P;
    p_frame.timestamp = 3600;
    cache.on_frame(p_frame);

    auto gop = cache.get_cached_gop();
    REQUIRE(gop.size() == 2);
    REQUIRE(gop[0].is_key_frame());
}

TEST_CASE("GopCache clears on new key frame", "[gop_cache]") {
    GopCache cache;

    MediaFrame key1;
    key1.type = MediaFrame::VIDEO_KEY;
    cache.on_frame(key1);

    MediaFrame p1;
    p1.type = MediaFrame::VIDEO_P;
    cache.on_frame(p1);
    cache.on_frame(p1);

    REQUIRE(cache.frame_count() == 3);

    // New key frame clears previous GOP
    MediaFrame key2;
    key2.type = MediaFrame::VIDEO_KEY;
    cache.on_frame(key2);

    REQUIRE(cache.frame_count() == 1);
}
