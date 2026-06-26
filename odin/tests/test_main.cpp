#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#ifndef _WIN32
#include <sys/socket.h>
#endif

#include <odin/odin.h>

TEST_CASE("version is exposed", "[version]")
{
    REQUIRE(std::string(odin::version()) == "0.1.0");
}

TEST_CASE("thread pool executes submitted tasks", "[thread_pool]")
{
    odin::ThreadPool pool(4);
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 100; ++i) {
        futures.push_back(pool.submit([](int value) { return value * 2; }, i));
    }

    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += futures[static_cast<std::size_t>(i)].get();
    }

    REQUIRE(sum == 9900);
}

TEST_CASE("file handle can write and read text", "[file_handle]")
{
    const auto path = std::filesystem::temp_directory_path() / "odin_file_handle_test.txt";

    {
        auto file = odin::FileHandle::open(path, "w");
        REQUIRE(file.valid());

        const std::string text = "odin";
        REQUIRE(file.write(text.data(), text.size()) == text.size());
        REQUIRE(file.flush());
    }

    {
        auto file = odin::FileHandle::open(path, "r");
        REQUIRE(file.valid());
        REQUIRE(file.read_all() == "odin");
    }

    std::filesystem::remove(path);
}

TEST_CASE("config roundtrip works", "[config]")
{
    const auto path = std::filesystem::temp_directory_path() / "odin_config_test.cfg";

    odin::Config config;
    config.set("name", "odin");
    config.set("mode", "demo");
    REQUIRE(config.save(path));

    odin::Config loaded;
    REQUIRE(loaded.load(path));
    REQUIRE(loaded.get("name") == "odin");
    REQUIRE(loaded.get("mode") == "demo");

    std::filesystem::remove(path);
}

TEST_CASE("socket can be created", "[socket]")
{
#ifndef _WIN32
    auto socket = odin::Socket::create(AF_INET, SOCK_STREAM, 0);
    REQUIRE(socket.valid());
    REQUIRE(socket.close());
#else
    SUCCEED("Socket wrapper is a POSIX-only helper in this first pass.");
#endif
}
