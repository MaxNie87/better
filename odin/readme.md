# Odin

Odin is a small C++17 utility library that follows the early-stage plan in `better_e.md`:

- a clean CMake-based project layout
- RAII wrappers for file handles and POSIX sockets
- a lightweight thread pool
- a simple config helper
- a spdlog-backed logging facade
- Catch2-based unit tests

## Layout

```text
odin/
├── CMakeLists.txt
├── cmake/
│   └── dependencies.cmake
├── include/odin/
├── src/core/
├── tests/
├── examples/
├── .clang-format
└── .github/workflows/ci.yml
```

## Build

```bash
cmake -B build -S .
cmake --build build
(cd build && ctest --output-on-failure)
```

## Demo

```bash
./build/examples/odin_demo
```

## What is already implemented

- `odin::version()`
- `odin::ThreadPool`
- `odin::FileHandle`
- `odin::Socket`
- `odin::Config`
- `odin::init_logger()` and log helpers
# CMake 常用指令查询表

| 指令 | 作用 | 常见写法 | 你可以把它理解成 |
|---|---|---|---|
| `cmake_minimum_required()` | 指定最低 CMake 版本 | `cmake_minimum_required(VERSION 3.16)` | 这个项目至少要哪个 CMake 才能跑 |
| `project()` | 定义项目名、版本、语言 | `project(odin VERSION 0.1.0 LANGUAGES CXX)` | 这是个什么项目 |
| `set()` | 设置变量 | `set(CMAKE_CXX_STANDARD 17)` | 给某个配置项赋值 |
| `add_subdirectory()` | 进入子目录继续构建 | `add_subdirectory(src)` | 去别的目录继续读 CMake 文件 |
| `add_library()` | 定义库目标 | `add_library(odin core/dump.cpp)` | 把这些 `.cpp` 编成一个库 |
| `add_executable()` | 定义可执行程序 | `add_executable(app main.cpp)` | 把这些源文件编成一个程序 |
| `target_include_directories()` | 设置头文件搜索路径 | `target_include_directories(odin PUBLIC include)` | 编译时去哪里找 `.h` |
| `target_compile_features()` | 指定编译特性 | `target_compile_features(odin PUBLIC cxx_std_17)` | 这个目标需要什么 C++ 能力 |
| `target_link_libraries()` | 链接库 | `target_link_libraries(odin PUBLIC spdlog::spdlog)` | 这个目标要依赖哪些库 |
| `include()` | 引入另一个 CMake 文件 | `include(cmake/dependencies.cmake)` | 把别的 CMake 脚本拿进来用 |
| `enable_testing()` | 开启测试支持 | `enable_testing()` | 让 `ctest` 能工作 |
| `add_test()` | 注册测试 | `add_test(NAME odin_tests COMMAND odin_tests)` | 告诉 CTest 这个测试怎么跑 |

## 最小骨架里最重要的 5 个

| 指令 | 作用 |
|---|---|
| `project()` | 定义项目 |
| `set()` | 设 C++ 标准 |
| `add_subdirectory()` | 进入 `src/` |
| `add_library()` | 定义 `odin` 库 |
| `target_include_directories()` | 暴露 `include/` |

## 快速记忆版

- **项目级**：`project`, `set`
- **目录级**：`add_subdirectory`
- **目标级**：`add_library`, `add_executable`, `target_include_directories`, `target_link_libraries`, `target_compile_features`
- **测试级**：`enable_testing`, `add_test`



# 这两个 CMake 指令的作用

## 1. `cmake -B build -S .`

作用：**配置项目，生成构建文件**。

- `-S .` 表示源码目录是当前目录
- `-B build` 表示把生成的构建文件放到 `build/` 目录

它会做的事包括：
- 读取当前目录下的 `CMakeLists.txt`
- 分析项目结构
- 检查编译器
- 生成后续真正编译要用的构建文件，比如 Makefile 或 Ninja 文件

你可以把它理解成：

> “先把项目说明书读一遍，准备好编译计划。”

---

## 2. `cmake --build build`

作用：**开始真正编译**。

- `build` 是上一步生成出来的构建目录
- CMake 会调用底层构建工具去执行编译
  - 可能是 `make`
  - 也可能是 `ninja`
  - 或其他平台对应工具

它会做的事包括：
- 编译 `.cpp` 文件
- 生成 `.o` / `.obj`
- 链接成库或可执行文件

你可以把它理解成：

> “按照刚才生成的计划，真正开始干活。”

---

## 两者关系

这两个命令通常是配套使用的：

```bash
cmake -B build -S .
cmake --build build





# `unique_ptr` / `shared_ptr` / `weak_ptr` 对比图

| 项目 | `unique_ptr` | `shared_ptr` | `weak_ptr` |
|---|---|---|---|
| 所有权 | 独占 | 共享 | 不拥有 |
| 能否复制 | 不能 | 可以 | 可以 |
| 能否移动 | 可以 | 可以 | 可以 |
| 引用计数 | 没有 | 有 | 不增加引用计数 |
| 自动释放 | 可以 | 可以 | 不负责释放 |
| 是否需要 `lock()` | 不需要 | 不需要 | 需要 |
| 适合场景 | 单一所有者资源 | 多处共享对象 | 观察对象、打破循环引用 |
| 开销 | 最低 | 较高 | 较低 |

## 生命周期示意

```text
unique_ptr:
对象 <--- 只有一个拥有者
结束时自动释放

shared_ptr:
对象 <--- owner1, owner2, owner3 ...
最后一个 owner 消失时释放

weak_ptr:
对象 <--- shared_ptr 持有
weak_ptr 只是观察，不算拥有者