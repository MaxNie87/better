#include <filesystem>
#include <iostream>
#include <string>

#include <odin/odin.h>

int main()
{
    odin::init_logger();
    ODIN_LOG_INFO("odin demo starting");

    odin::ThreadPool pool(2);
    auto future = pool.submit([](int lhs, int rhs) { return lhs + rhs; }, 20, 22);

    odin::Config config;
    config.set("project", "odin");
    config.set("profile", "demo");

    const auto temp_file = std::filesystem::temp_directory_path() / "odin_demo.txt";
    {
        auto file = odin::FileHandle::open(temp_file, "w");
        if (file.valid()) {
            const std::string message = "hello from odin\n";
            (void)file.write(message.data(), message.size());
            (void)file.flush();
        }
    }

    std::cout << "version: " << odin::version() << '\n';
    std::cout << "sum: " << future.get() << '\n';
    std::cout << "profile: " << config.get("profile") << '\n';
    std::cout << "temp file: " << temp_file << '\n';

    return 0;
}
