---
name: C/C++ embedded learning plan
overview: A 16-week (112-day) structured learning plan from zero C/C++ knowledge to building a production-grade STM32 IoT Gateway with Secure OTA Bootloader -- a project that is universally complained about in the embedded community but lacks a clean, educational open-source implementation.
todos:
  - id: phase1
    content: "Phase 1 (Week 1-4): C Language Core -- pointers, memory, data structures, state machines, build systems"
    status: pending
  - id: phase2
    content: "Phase 2 (Week 5-7): C++ Core -- RAII, templates, modern C++17, embedded-friendly patterns"
    status: pending
  - id: phase3
    content: "Phase 3 (Week 8-10): Embedded Fundamentals -- STM32 GPIO/UART/I2C/SPI, drivers, flash, bootloader basics"
    status: pending
  - id: phase4
    content: "Phase 4 (Week 11-12): FreeRTOS + MQTT -- multi-task architecture, ESP32 gateway, cloud connectivity"
    status: pending
  - id: phase5
    content: "Phase 5 (Week 13-16): Final Project -- Secure OTA Bootloader + IoT Gateway + PC tools + documentation + release"
    status: pending
isProject: false
---


# C/C++ 嵌入式开发从零到项目实战 -- 16 周学习计划

---

## 最终项目: STM32 IoT 传感器网关 + 安全 OTA Bootloader

### 为什么是这个项目

**被诟病的痛点:**
- OTA 固件升级是嵌入式开发中**最令人头疼的问题之一** -- 几乎每个团队都要重复造轮子
- 现有开源方案要么绑定特定厂商（ESP-IDF 只能用于 ESP32）、要么过于复杂（eboot 支持 83 种板子但学习成本极高）、要么是玩具级别（没有断电恢复、没有签名验证）
- 中文社区大量文章标题都是"OTA 升级踩坑记录"、"Bootloader 设计的 N 个坑" -- **人人都在抱怨，但没有一个干净、教育性强、可直接复用的 STM32 通用实现**

**项目覆盖的市场热门技术栈:**

| 技能 | 岗位出现率 | 本项目覆盖 |
|------|-----------|-----------|
| C 语言（指针/内存/位运算） | 96% | Bootloader + 驱动层 |
| C++（RAII/模板/现代标准） | 78% | 应用层架构 |
| FreeRTOS | 76% | 任务调度/信号量/队列 |
| I2C / SPI / UART | 88% | 传感器驱动 + ESP32 通信 |
| MQTT | IoT 必备 | 云端数据上报 |
| OTA / Secure Boot | 28%（高薪加分项） | 双 Bank Bootloader |
| CMake / GCC ARM / GDB | 52% | 构建系统 |
| Git | 14% | 全程版本管理 |

### 项目架构

```
                        +-----------+
                        | MQTT 云端 |
                        +-----+-----+
                              |
                    WiFi/MQTT |
                              |
+---------------------------+ | +---------------------------+
|       STM32F4 主控         | | |       ESP32 网关           |
|                           | | |                           |
|  +-------+  +----------+ | | | +--------+  +-----------+ |
|  |BME280 |  | MPU6050  | | | | | WiFi   |  | MQTT      | |
|  |I2C    |  | SPI      | | | | | Stack  |  | Client    | |
|  +---+---+  +----+-----+ | | | +---+----+  +-----+-----+ |
|      |           |        | | |     |              |       |
|  +---v-----------v-----+  | | | +---v--------------v----+  |
|  |   FreeRTOS 任务调度   |  | | | |   AT 指令解析引擎     |  |
|  |  - 传感器采集任务     |  |<-->| |   OTA 镜像接收       |  |
|  |  - 数据处理任务       |  |UART| +------------------------+  |
|  |  - 通信管理任务       |  | | |                           |
|  |  - CLI 调试任务       |  | | +---------------------------+
|  +----------+----------+  | |
|             |              | |
|  +----------v----------+  | |
|  |  双 Bank Bootloader   |  | |
|  |  - SHA256 校验       |  | |
|  |  - 断电恢复          |  | |
|  |  - 版本管理          |  | |
|  |  - 自动回滚          |  | |
|  +---------------------+  | |
+---------------------------+ |
```

---

## 阶段一: C 语言核心 (第 1-4 周 / Day 1-28)

> 目标: 精通 C 语言基础，能手写链表、环形缓冲区等嵌入式常用数据结构

### 第 1 周: 开发环境 + 基本语法 (Day 1-7)

---

#### Day 1 -- 环境搭建与第一个程序 (总计 2.5h)

**理论学习 (40min)**
- 阅读: "C Programming: A Modern Approach" 第 1-2 章（程序概念 + 编译流程）
- 理解编译四步: 预处理(.i) -> 编译(.s) -> 汇编(.o) -> 链接(可执行文件)
- 了解 GCC 是什么、MinGW 是什么、为什么 Windows 需要 MinGW

**实践操作 (90min)**
1. 安装 VS Code + C/C++ 扩展 + MinGW-w64（或在 Windows 上启用 WSL2 + `sudo apt install gcc gdb`）
2. 终端中验证: `gcc --version`，确保输出版本号
3. 安装 Git: `git --version`，配置 `git config --global user.name/email`
4. 在 GitHub 上创建仓库 `c-cpp-learning`，本地 `git clone`
5. 创建目录结构:
   ```
   c-cpp-learning/
   ├── day01/
   │   └── hello.c
   └── README.md
   ```
6. 编写 `hello.c`:
   ```c
   #include <stdio.h>
   int main(void) {
       printf("Hello, Embedded World!\n");
       return 0;
   }
   ```
7. 分步编译并观察每步产物:
   - `gcc -E hello.c -o hello.i` -- 打开 `hello.i` 看预处理展开了什么
   - `gcc -S hello.c -o hello.s` -- 打开 `hello.s` 看汇编代码，找到 `main` 标签
   - `gcc -c hello.c -o hello.o` -- 用 `file hello.o` 查看文件类型
   - `gcc hello.o -o hello.exe` -- 运行 `./hello.exe`
8. `git add . && git commit -m "day01: hello world + compilation pipeline"`

**验收标准**
- [ ] `gcc --version` 正常输出
- [ ] 能解释 `.i` `.s` `.o` 三个中间文件分别是什么
- [ ] GitHub 仓库有第一个 commit

**常见坑**
- Windows 下 MinGW 安装后需手动添加 `bin` 目录到 PATH 环境变量
- WSL2 中要用 `gcc` 而不是 `x86_64-w64-mingw32-gcc`

---

#### Day 2 -- 数据类型与内存模型 (总计 2.5h)

**理论学习 (45min)**
- 阅读: "C Programming: A Modern Approach" 第 7 章（基本类型）
- 重点掌握:
  - `char` (1字节) / `short` (2字节) / `int` (通常4字节) / `long` (4或8字节)
  - 为什么嵌入式中**必须用固定宽度类型**: `#include <stdint.h>` -> `uint8_t/int8_t/uint16_t/int16_t/uint32_t/int32_t`
  - `sizeof` 运算符: 返回类型或变量占用的字节数
  - 大端(Big-Endian) vs 小端(Little-Endian): 高字节在低地址 vs 低字节在低地址
  - x86/ARM 默认小端，网络字节序是大端

**实践操作 (90min)**
1. 创建 `day02/types.c`:
   ```c
   #include <stdio.h>
   #include <stdint.h>
   int main(void) {
       printf("char:     %zu bytes\n", sizeof(char));
       printf("int:      %zu bytes\n", sizeof(int));
       printf("uint8_t:  %zu bytes\n", sizeof(uint8_t));
       printf("uint16_t: %zu bytes\n", sizeof(uint16_t));
       printf("uint32_t: %zu bytes\n", sizeof(uint32_t));
       printf("float:    %zu bytes\n", sizeof(float));
       printf("double:   %zu bytes\n", sizeof(double));
       printf("void*:    %zu bytes\n", sizeof(void*));
       return 0;
   }
   ```
2. 创建 `day02/endian.c` -- 编写大小端检测与转换程序:
   ```c
   #include <stdio.h>
   #include <stdint.h>
   // 任务: 实现以下函数
   void print_bytes(const void *ptr, size_t len);   // 逐字节打印内存内容 (十六进制)
   int is_little_endian(void);                       // 检测当前系统是否小端
   uint32_t swap_endian_32(uint32_t value);           // 反转32位整数的字节序
   ```
3. `print_bytes` 实现提示: 将 `ptr` 转为 `const uint8_t *`，用 for 循环逐字节打印
4. `is_little_endian` 实现提示: 定义 `uint32_t x = 1;`，检查 `*(uint8_t*)&x` 是否为 1
5. `swap_endian_32` 实现提示: 用移位和掩码 `(v >> 24) | ((v >> 8) & 0xFF00) | ...`
6. 测试: `uint32_t val = 0xDEADBEEF;`，打印原始字节和交换后字节
7. 学习十六进制: 练习手算 `0xFF = 255`、`0x0A = 10`、`0x80 = 128`

**验收标准**
- [ ] 能说出 `uint32_t` 在内存中占 4 字节
- [ ] 运行程序正确检测出系统是小端
- [ ] `swap_endian_32(0xDEADBEEF)` 返回 `0xEFBEADDE`
- [ ] 代码提交到 GitHub

**常见坑**
- `printf` 打印 `sizeof` 结果要用 `%zu`（size_t 类型），不能用 `%d`
- 类型转换 `(uint8_t*)&x` 不要忘记取地址 `&`

---

#### Day 3 -- 控制流与函数 (总计 2.5h)

**理论学习 (40min)**
- 阅读: "C Programming: A Modern Approach" 第 5-6 章（选择语句 + 循环）+ 第 9 章（函数）
- 重点掌握:
  - `switch` 语句必须写 `break`（遗漏是经典 bug）
  - `for` 循环的三段式、`while` 和 `do-while` 的区别
  - 函数的声明(declaration)与定义(definition)的区别
  - 头文件(`.h`)放声明，源文件(`.c`)放实现
  - `#ifndef HEADER_H` / `#define HEADER_H` / `#endif` 头文件守卫的作用

**实践操作 (90min)**
1. 创建 `day03/` 目录，包含以下文件结构:
   ```
   day03/
   ├── main.c          // 主程序，包含 main() 和用户交互
   ├── calc.h          // 计算器函数声明
   ├── calc.c          // 计算器函数实现
   └── Makefile        // 构建脚本
   ```
2. `calc.h` 内容:
   ```c
   #ifndef CALC_H
   #define CALC_H
   double calc_add(double a, double b);
   double calc_sub(double a, double b);
   double calc_mul(double a, double b);
   double calc_div(double a, double b, int *error);  // error: 0=成功, 1=除零
   #endif
   ```
3. `calc.c`: 实现上述 4 个函数，`calc_div` 中检查除零
4. `main.c`: 用 `scanf` 读取两个数和运算符，用 `switch` 分发到对应函数
5. `Makefile` (第一个 Makefile):
   ```makefile
   CC = gcc
   CFLAGS = -Wall -Wextra -std=c11
   calc: main.o calc.o
   	$(CC) -o $@ $^
   main.o: main.c calc.h
   	$(CC) $(CFLAGS) -c main.c
   calc.o: calc.c calc.h
   	$(CC) $(CFLAGS) -c calc.c
   clean:
   	rm -f *.o calc
   ```
6. 运行 `make`，测试所有运算，特别测试除零情况
7. 思考题: 为什么 `calc_div` 的 error 参数用指针而不是返回值?

**验收标准**
- [ ] `make` 一次通过，无 warning
- [ ] 除零时程序不会崩溃，输出错误提示
- [ ] 能解释头文件守卫 `#ifndef` 的作用（防止重复包含）
- [ ] 理解 `.h` 放什么、`.c` 放什么

**常见坑**
- Makefile 中命令行前必须是 **Tab 字符** 而非空格，否则 `make` 会报 `missing separator`
- Windows 下 `rm` 命令可能不存在，Makefile 中 `clean` 可以改用 `del`

---

#### Day 4 -- 数组与字符串 (总计 2.5h)

**理论学习 (40min)**
- 阅读: "C Programming: A Modern Approach" 第 8 章（数组）+ 第 13 章（字符串）
- 重点掌握:
  - 数组名就是首元素的地址: `arr` 等价于 `&arr[0]`
  - 数组作为参数传递时退化为指针（丢失长度信息），所以必须额外传 `len`
  - C 字符串是以 `\0` 结尾的 `char` 数组
  - `strncpy` vs `strcpy`: 为什么必须用 `strncpy`（防止缓冲区溢出）
  - `strncpy` 的坑: 如果源串长度 >= n，不会自动添加 `\0`

**实践操作 (90min)**
1. 创建 `day04/string_utils.h`:
   ```c
   #ifndef STRING_UTILS_H
   #define STRING_UTILS_H
   #include <stddef.h>
   size_t my_strlen(const char *s);
   char *my_strncpy(char *dst, const char *src, size_t n);  // 保证末尾有 \0
   int my_strcmp(const char *s1, const char *s2);
   char *my_strstr(const char *haystack, const char *needle); // 加分项
   #endif
   ```
2. 创建 `day04/string_utils.c`，逐个实现:
   - `my_strlen`: 遍历直到遇到 `\0`，返回计数。注意: 不包含 `\0` 本身
   - `my_strncpy`: 最多复制 `n-1` 个字符，**始终**在末尾写入 `\0`（比标准 `strncpy` 更安全）
   - `my_strcmp`: 逐字符比较，返回差值
3. 创建 `day04/test_strings.c` 测试程序:
   ```c
   // 测试用例:
   // 1. my_strlen("hello") == 5
   // 2. my_strlen("") == 0
   // 3. my_strncpy(buf, "hello world", 6) -> buf 应为 "hello\0"
   // 4. my_strcmp("abc", "abd") < 0
   // 5. my_strcmp("abc", "abc") == 0
   ```
4. 缓冲区溢出演示 -- 创建 `day04/overflow_demo.c`:
   ```c
   char buf[8];
   strcpy(buf, "this is way too long for buf");  // 溢出!
   // 用 gcc -fsanitize=address 编译运行，观察 AddressSanitizer 报错
   ```

**验收标准**
- [ ] 所有 5 个测试用例通过
- [ ] `my_strncpy` 在目标缓冲区不足时不会溢出，且末尾有 `\0`
- [ ] 能用 `-fsanitize=address` 检测出 `overflow_demo.c` 的溢出

**常见坑**
- `my_strlen` 如果传入 `NULL` 会段错误 -- 可以加 NULL 检查
- 标准 `strncpy` 不保证末尾有 `\0`，你的 `my_strncpy` 要修复这个问题

---

#### Day 5 -- 指针基础 (总计 2.5h)

**理论学习 (45min)**
- 阅读: "C Programming: A Modern Approach" 第 11-12 章（指针 + 指针与数组）
- 核心概念:
  - 指针变量存储的是一个**内存地址**
  - `int *p = &x;` -- `p` 存储了 `x` 的地址
  - `*p` 是解引用 -- 通过地址访问那个位置的值
  - `p + 1` 并非地址加 1，而是加 `sizeof(*p)` 个字节（指针算术）
  - 数组名和指针的关系: `arr[i]` 等价于 `*(arr + i)`
- 画内存图: 在纸上画出变量和指针的内存关系（地址、值、箭头）

**实践操作 (90min)**
1. 创建 `day05/pointer_basics.c`:
   ```c
   #include <stdio.h>
   // 练习 1: 通过指针交换两个变量
   void swap(int *a, int *b);
   // 练习 2: 用指针遍历数组
   void print_array(const int *arr, int len);
   // 练习 3: 用指针翻转数组 (原地)
   void reverse_array(int *arr, int len);
   // 练习 4: 在数组中查找值，返回指向该元素的指针（找不到返回 NULL）
   int *find_value(int *arr, int len, int target);
   ```
2. 实现要点:
   - `swap`: `int tmp = *a; *a = *b; *b = tmp;`
   - `reverse_array`: 用两个指针 `left` 和 `right`，从两端向中间移动，逐对交换
   - `find_value`: 遍历数组，`if (arr[i] == target) return &arr[i];`
3. `main` 函数中:
   ```c
   int arr[] = {5, 3, 8, 1, 9, 2, 7};
   int len = sizeof(arr) / sizeof(arr[0]);  // 计算数组长度的标准方法
   printf("原始: "); print_array(arr, len);
   reverse_array(arr, len);
   printf("翻转: "); print_array(arr, len);
   int *found = find_value(arr, len, 8);
   if (found) printf("找到 8，地址偏移: %td\n", found - arr);
   ```
4. 画图练习: 在纸上画出 `arr` 的内存布局，标注每个元素的地址偏移

**验收标准**
- [ ] `swap` 正确交换两个变量（打印前后值验证）
- [ ] `reverse_array` 正确翻转（奇数/偶数长度都测试）
- [ ] 能口头解释 `*p`、`&x`、`p + 1` 的含义
- [ ] 能解释 `sizeof(arr) / sizeof(arr[0])` 为什么能计算数组长度

**常见坑**
- 数组作为函数参数传递后，`sizeof(arr)` 不再返回数组总大小，而是返回指针大小（4或8字节）

---

#### Day 6 -- 指针进阶 (总计 2.5h)

**理论学习 (40min)**
- 阅读: "C Programming: A Modern Approach" 第 17 章（指针的高级应用）
- 核心概念:
  - `int **pp` -- 指向指针的指针，用于在函数中修改调用者的指针变量
  - `void *` -- 通用指针，可以指向任何类型（但不能直接解引用，需要先转型）
  - `const int *p` -- 指向 const int 的指针（不能通过 p 修改值，但 p 本身可以改指向）
  - `int *const p` -- const 指针（p 本身不能改指向，但可以通过 p 修改值）
  - `const int *const p` -- 都不能改
  - 口诀: `const` 在 `*` 左边修饰数据，在 `*` 右边修饰指针

**实践操作 (90min)**
1. 创建 `day06/memutils.h`:
   ```c
   #ifndef MEMUTILS_H
   #define MEMUTILS_H
   #include <stddef.h>
   void *my_memcpy(void *dst, const void *src, size_t n);
   void *my_memset(void *ptr, int value, size_t n);
   int my_memcmp(const void *s1, const void *s2, size_t n);
   #endif
   ```
2. 实现 `day06/memutils.c`:
   - `my_memcpy`: 将 `src` 和 `dst` 转为 `uint8_t *`，逐字节复制
   - `my_memset`: 将 `ptr` 转为 `uint8_t *`，逐字节设置
   - `my_memcmp`: 将两个指针转为 `const uint8_t *`，逐字节比较
3. 创建 `day06/ptr_ptr_demo.c` -- 指针的指针练习:
   ```c
   // 场景: 用函数分配内存并返回给调用者
   // 错误版本: void alloc_buffer(int *buf, int size) { buf = malloc(size * sizeof(int)); }
   // 正确版本: void alloc_buffer(int **buf, int size) { *buf = malloc(size * sizeof(int)); }
   // 任务: 实现正确版本，在 main 中验证
   ```
4. const 指针练习 -- 解释为什么以下代码能/不能编译:
   ```c
   int x = 10;
   const int *p1 = &x;    // *p1 = 20;  能编译吗?
   int *const p2 = &x;    // p2 = NULL;  能编译吗?  *p2 = 20; 能编译吗?
   ```

**验收标准**
- [ ] `my_memcpy` 能正确复制结构体（不仅仅是 int 数组）
- [ ] 能解释 `void *` 为什么需要先转型才能使用
- [ ] 能说出"指向指针的指针"的使用场景（函数内分配内存返回给调用者）
- [ ] 能区分 `const int *p` 和 `int *const p`

---

#### Day 7 -- 周末复习 + 小项目: 学生管理系统 (总计 4-5h)

**上午: 复习 (1.5h)**
- 回顾 Day 1-6 笔记，在纸上默写:
  - 编译四步流程
  - 大端/小端区别
  - `sizeof` 各类型结果
  - 指针相关操作符含义
  - `const` 指针的三种组合
- 整理代码，确保 Day 1-6 全部提交到 GitHub

**下午: 综合项目 (3-3.5h)**

项目: **学生成绩管理系统** -- 文件结构:
```
day07_student_mgr/
├── main.c              // 主程序: 菜单循环
├── student.h           // 数据结构定义 + 函数声明
├── student.c           // 函数实现
└── Makefile
```

`student.h` 要求:
```c
#define MAX_STUDENTS 100
#define NAME_MAX_LEN 32

typedef struct {
    int id;
    char name[NAME_MAX_LEN];
    float scores[3];    // 语文、数学、英语
    float average;       // 平均分
} Student;

typedef struct {
    Student data[MAX_STUDENTS];
    int count;
} StudentDB;

void db_init(StudentDB *db);
int db_add(StudentDB *db, const char *name, float s1, float s2, float s3);
int db_remove(StudentDB *db, int id);
Student *db_find_by_id(StudentDB *db, int id);
Student *db_find_by_name(StudentDB *db, const char *name);
void db_sort_by_average(StudentDB *db);           // 冒泡排序，降序
void db_print_all(const StudentDB *db);
void db_print_one(const Student *s);
```

`main.c` 菜单:
```
===== 学生管理系统 =====
1. 添加学生
2. 删除学生
3. 按ID查找
4. 按姓名查找
5. 按均分排序并显示
6. 显示所有学生
0. 退出
请选择:
```

**验收标准**
- [ ] 添加 5 个学生，排序显示，平均分从高到低
- [ ] 删除一个学生后再显示，确认已删除
- [ ] 按姓名模糊查找（用 `strstr`）能找到学生
- [ ] 代码 `gcc -Wall -Wextra` 编译零 warning
- [ ] 代码提交到 GitHub，commit 信息清晰

---

### 第 2 周: 指针、结构体与内存管理 (Day 8-14)

---

#### Day 8 -- 结构体与 typedef (总计 2.5h)

**理论学习 (45min)**
- 阅读: "C Programming: A Modern Approach" 第 16 章（结构、联合与枚举）
- 核心概念:
  - 结构体内存对齐规则: 每个成员的起始地址必须是其大小的整数倍
  - 例: `struct { char a; int b; }` 占 8 字节而非 5 字节（a 后有 3 字节填充）
  - `#pragma pack(1)` 可以禁用对齐（嵌入式协议解析中常用）
  - `__attribute__((packed))` 是 GCC 特有的等效写法
  - 嵌入式中为什么关心对齐: 1) 节省内存 2) 协议帧要求字段紧凑排列

**实践操作 (90min)**
1. 创建 `day08/alignment.c`:
   ```c
   #include <stdio.h>
   #include <stdint.h>
   #include <stddef.h>  // offsetof

   typedef struct {
       uint8_t  start_byte;    // 1 字节
       uint16_t length;         // 2 字节
       uint32_t timestamp;      // 4 字节
       uint8_t  data[8];        // 8 字节
       uint16_t crc;            // 2 字节
   } Packet_Normal;

   #pragma pack(push, 1)
   typedef struct {
       uint8_t  start_byte;
       uint16_t length;
       uint32_t timestamp;
       uint8_t  data[8];
       uint16_t crc;
   } Packet_Packed;
   #pragma pack(pop)

   // 任务: 打印两个结构体的 sizeof 和每个成员的 offsetof
   // 对比差异，理解填充字节在哪里
   ```
2. 用 `offsetof(Packet_Normal, length)` 打印每个字段的偏移量
3. 思考: 网络协议解析时，为什么必须用 packed 结构体?
4. 创建 `day08/nested.c` -- 嵌套结构体练习:
   ```c
   typedef struct { float x, y, z; } Vec3;
   typedef struct {
       Vec3 accel;
       Vec3 gyro;
       uint32_t timestamp_ms;
   } IMU_Data;
   // 初始化并打印
   ```

**验收标准**
- [ ] 能说出 `Packet_Normal` 和 `Packet_Packed` 的 sizeof 差异及原因
- [ ] 能画出 `Packet_Normal` 的内存布局图（包括填充字节位置）
- [ ] 理解 `offsetof` 宏的用途

---

#### Day 9 -- 结构体指针与函数指针 (总计 2.5h)

**理论学习 (40min)**
- 函数指针语法: `返回类型 (*变量名)(参数列表)` -- 例: `int (*cmp)(int, int)`
- `typedef` 简化函数指针: `typedef int (*Comparator)(int, int);`
- 函数指针的三大用途: 1) 回调函数 2) 命令分发表 3) C 语言模拟多态
- 结构体指针 `->` 运算符: `p->member` 等价于 `(*p).member`

**实践操作 (90min)**
1. 创建 `day09/cmd_parser.h`:
   ```c
   #ifndef CMD_PARSER_H
   #define CMD_PARSER_H

   #define MAX_COMMANDS 16
   #define MAX_ARGS     8

   typedef void (*CmdHandler)(int argc, char **argv);

   typedef struct {
       const char *name;
       const char *help;
       CmdHandler handler;
   } Command;

   typedef struct {
       Command commands[MAX_COMMANDS];
       int count;
   } CmdParser;

   void parser_init(CmdParser *p);
   int parser_register(CmdParser *p, const char *name, const char *help, CmdHandler handler);
   void parser_execute(CmdParser *p, const char *input);  // 解析输入字符串并执行匹配的命令
   void parser_print_help(const CmdParser *p);
   #endif
   ```
2. 实现 `day09/cmd_parser.c`:
   - `parser_execute`: 用 `strtok` 将输入拆分为 `argv[]`，第一个 token 是命令名
   - 遍历注册的命令表，`strcmp` 匹配命令名，调用对应 handler
3. 创建 `day09/main.c`:
   ```c
   void cmd_hello(int argc, char **argv) {
       if (argc > 1) printf("Hello, %s!\n", argv[1]);
       else printf("Hello, World!\n");
   }
   void cmd_add(int argc, char **argv) {
       if (argc != 3) { printf("用法: add <a> <b>\n"); return; }
       printf("%d\n", atoi(argv[1]) + atoi(argv[2]));
   }
   // 注册命令，循环读取用户输入
   ```
4. 测试: 输入 `hello Bob` -> `Hello, Bob!`，输入 `add 3 5` -> `8`

**验收标准**
- [ ] 命令解析器能正确分发至少 3 个命令
- [ ] 输入未知命令时提示 "Unknown command"
- [ ] `help` 命令能列出所有注册的命令及帮助信息
- [ ] 能口头解释函数指针 `void (*handler)(int, char**)` 的类型含义

---

#### Day 10 -- 动态内存管理 (总计 2.5h)

**理论学习 (40min)**
- 阅读: "C Programming: A Modern Approach" 第 17.3-17.4 节
- 核心概念:
  - `malloc(n)`: 分配 n 字节，内容未初始化（可能是垃圾值）
  - `calloc(count, size)`: 分配 count * size 字节，初始化为 0
  - `realloc(ptr, new_size)`: 扩展/缩小已分配的内存
  - `free(ptr)`: 释放内存。`free(NULL)` 是安全的
  - 三大内存错误: **内存泄漏**(忘记 free)、**野指针**(free 后继续使用)、**double free**(free 两次)
  - 防御: free 后立即将指针设为 NULL

**实践操作 (90min)**
1. 创建 `day10/dynarray.h`:
   ```c
   #ifndef DYNARRAY_H
   #define DYNARRAY_H
   #include <stddef.h>

   typedef struct {
       int *data;
       size_t size;      // 当前元素个数
       size_t capacity;  // 已分配容量
   } DynArray;

   int  da_init(DynArray *arr, size_t initial_cap);  // 返回 0 成功，-1 失败
   void da_destroy(DynArray *arr);
   int  da_push(DynArray *arr, int value);            // 容量不足时自动 2 倍扩容
   int  da_pop(DynArray *arr, int *out_value);         // 返回并移除最后一个元素
   int  da_get(const DynArray *arr, size_t index, int *out_value);
   int  da_set(DynArray *arr, size_t index, int value);
   size_t da_size(const DynArray *arr);
   void da_print(const DynArray *arr);
   #endif
   ```
2. 实现关键点:
   - `da_push` 中当 `size == capacity` 时，调用 `realloc(data, capacity * 2 * sizeof(int))`
   - `realloc` 可能失败 -- 必须先用临时变量接收返回值，判断非 NULL 后再赋值
   - `da_destroy` 中 `free(arr->data); arr->data = NULL; arr->size = 0;`
3. 测试: 初始容量 4，连续 push 20 个元素，验证自动扩容
4. 故意制造内存泄漏（注释掉 `da_destroy`），用编译选项检测:
   ```bash
   gcc -fsanitize=leak -g day10/test.c day10/dynarray.c -o test && ./test
   ```

**验收标准**
- [ ] 连续 push 20 个元素后 `da_get` 全部正确
- [ ] `da_pop` 在空数组上不会崩溃，返回错误码
- [ ] 程序退出前调用 `da_destroy`，AddressSanitizer 无泄漏报告
- [ ] 能解释为什么 `realloc` 返回值必须先存到临时变量

---

#### Day 11 -- 链表 (总计 2.5h)

**理论学习 (40min)**
- 链表概念: 每个节点包含数据 + 指向下一个节点的指针
- 链表 vs 数组:
  - 链表: 插入/删除 O(1)（已知位置），查找 O(n)，不需要连续内存
  - 数组: 随机访问 O(1)，插入/删除 O(n)，需要连续内存
- 嵌入式中的选择: 数组优先（缓存友好、无碎片），链表在需要频繁增删时使用
- 画图: 在纸上画出链表的插入和删除过程（指针变化）

**实践操作 (90min)**
1. 创建 `day11/linked_list.h`:
   ```c
   #ifndef LINKED_LIST_H
   #define LINKED_LIST_H

   typedef struct Node {
       int data;
       struct Node *next;
   } Node;

   typedef struct {
       Node *head;
       int count;
   } LinkedList;

   void list_init(LinkedList *list);
   void list_destroy(LinkedList *list);         // 释放所有节点
   int  list_push_front(LinkedList *list, int data);
   int  list_push_back(LinkedList *list, int data);
   int  list_pop_front(LinkedList *list, int *out_data);
   Node *list_find(const LinkedList *list, int data);
   int  list_remove(LinkedList *list, int data);  // 删除第一个匹配的节点
   void list_print(const LinkedList *list);       // 打印: [1] -> [2] -> [3] -> NULL
   int  list_size(const LinkedList *list);
   #endif
   ```
2. 实现要点:
   - `list_push_front`: 新建节点，`new_node->next = list->head; list->head = new_node;`
   - `list_push_back`: 遍历到最后一个节点，`last->next = new_node;`
   - `list_remove`: 需要用 **前驱指针** 或 **指向指针的指针** 技巧
   - `list_destroy`: 用 `while` 循环，保存 `next` 后再 `free` 当前节点
3. 测试用例:
   - push_front 3 个元素，打印验证顺序
   - push_back 3 个元素，打印验证顺序
   - 删除头节点、中间节点、尾节点，分别验证
   - destroy 后验证没有内存泄漏

**验收标准**
- [ ] 所有操作正确（特别是边界: 空链表删除、删除唯一节点）
- [ ] `list_destroy` 后无内存泄漏（用 `-fsanitize=leak` 检测）
- [ ] 能在纸上画出 `list_remove` 的指针变化过程

---

#### Day 12 -- 位运算 (总计 2.5h)

**理论学习 (45min)**
- 六种位运算符及含义:
  - `&` (AND): 清零指定位 -- `reg & ~(1 << n)` 清除第 n 位
  - `|` (OR): 置位指定位 -- `reg | (1 << n)` 设置第 n 位
  - `^` (XOR): 翻转指定位 -- `reg ^ (1 << n)` 翻转第 n 位
  - `~` (NOT): 取反所有位
  - `<< >>` (左移/右移): 左移等于乘 2，右移等于除 2
- 位掩码 (bitmask): 用于从寄存器中提取特定位字段
  - 例: 提取 bit[7:4] -> `(reg >> 4) & 0x0F`
- 嵌入式应用: GPIO 寄存器操作全靠位运算

**实践操作 (90min)**
1. 创建 `day12/bitops.h`:
   ```c
   #ifndef BITOPS_H
   #define BITOPS_H
   #include <stdint.h>

   #define BIT(n)               (1U << (n))
   #define SET_BIT(reg, n)      ((reg) |= BIT(n))
   #define CLR_BIT(reg, n)      ((reg) &= ~BIT(n))
   #define TOGGLE_BIT(reg, n)   ((reg) ^= BIT(n))
   #define READ_BIT(reg, n)     (((reg) >> (n)) & 1U)
   #define WRITE_BITS(reg, mask, val)  ((reg) = ((reg) & ~(mask)) | ((val) & (mask)))
   #endif
   ```
2. 创建 `day12/gpio_sim.c` -- 模拟 GPIO 寄存器:
   ```c
   // 模拟一个 32 位 GPIO 输出寄存器
   uint32_t GPIO_ODR = 0x00000000;

   void gpio_set_pin(uint8_t pin) { SET_BIT(GPIO_ODR, pin); }
   void gpio_clr_pin(uint8_t pin) { CLR_BIT(GPIO_ODR, pin); }
   void gpio_toggle_pin(uint8_t pin) { TOGGLE_BIT(GPIO_ODR, pin); }
   int  gpio_read_pin(uint8_t pin) { return READ_BIT(GPIO_ODR, pin); }
   void gpio_print_reg(uint32_t reg);  // 打印 32 位二进制: 0b00000000...
   ```
3. 进阶练习 -- CRC8 计算:
   ```c
   // 简单 CRC8 (多项式 0x07)
   uint8_t crc8(const uint8_t *data, size_t len);
   // 实现: 逐字节，逐位 XOR 移位
   ```
4. 测试: 设置 pin 0,3,7 -> 打印寄存器应显示 `0b10001001` (0x89)

**验收标准**
- [ ] `gpio_print_reg` 正确以二进制格式打印 32 位寄存器
- [ ] SET/CLR/TOGGLE/READ 操作全部正确
- [ ] 能解释 `(1U << n)` 中 `U` 后缀的作用（无符号，避免有符号左移未定义行为）
- [ ] CRC8 计算结果与在线工具验证一致

---

#### Day 13 -- 枚举、联合体与预处理器 (总计 2.5h)

**理论学习 (40min)**
- `enum`: 定义一组命名常量，编译器自动赋值 0,1,2...
  - `enum class` 不存在于 C 中（C++ 才有），C 的 enum 就是整数
  - 嵌入式中: 用于状态机状态、错误码、命令类型等
- `union`: 所有成员共享同一块内存，`sizeof` 等于最大成员
  - 嵌入式中: 协议解析（将字节数组重新解释为特定字段）
  - 注意: 类型双关 (type punning) 在严格别名规则下可能有问题，`union` 是 C 中合法的方式
- 预处理器高级:
  - `#define LOG(fmt, ...) printf("[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)`
  - `do { ... } while(0)` 包装多行宏的原因: 使宏在 `if` 语句中安全使用

**实践操作 (90min)**
1. 创建 `day13/variant.h` -- 变体类型:
   ```c
   typedef enum { VAR_INT, VAR_FLOAT, VAR_STRING } VarType;

   typedef struct {
       VarType type;
       union {
           int    i;
           float  f;
           char   s[32];
       } value;
   } Variant;

   Variant var_from_int(int v);
   Variant var_from_float(float v);
   Variant var_from_string(const char *s);
   void var_print(const Variant *v);
   int var_to_int(const Variant *v, int *out);  // 类型不匹配返回 -1
   ```
2. 创建 `day13/debug_macros.h` -- 调试宏:
   ```c
   #ifndef DEBUG_MACROS_H
   #define DEBUG_MACROS_H
   #include <stdio.h>

   #define LOG_LEVEL_DEBUG 0
   #define LOG_LEVEL_INFO  1
   #define LOG_LEVEL_WARN  2
   #define LOG_LEVEL_ERROR 3

   #ifndef CURRENT_LOG_LEVEL
   #define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG
   #endif

   #define LOG(level, level_str, fmt, ...) \
       do { \
           if ((level) >= CURRENT_LOG_LEVEL) \
               printf("[%s][%s:%d] " fmt "\n", level_str, __FILE__, __LINE__, ##__VA_ARGS__); \
       } while(0)

   #define LOG_D(fmt, ...) LOG(LOG_LEVEL_DEBUG, "DBG", fmt, ##__VA_ARGS__)
   #define LOG_I(fmt, ...) LOG(LOG_LEVEL_INFO,  "INF", fmt, ##__VA_ARGS__)
   #define LOG_W(fmt, ...) LOG(LOG_LEVEL_WARN,  "WRN", fmt, ##__VA_ARGS__)
   #define LOG_E(fmt, ...) LOG(LOG_LEVEL_ERROR, "ERR", fmt, ##__VA_ARGS__)
   #endif
   ```
3. 测试: 编译时通过 `-DCURRENT_LOG_LEVEL=2` 只显示 WARN 和 ERROR

**验收标准**
- [ ] Variant 类型能正确存取 int/float/string
- [ ] `var_to_int` 对 float 类型的 Variant 返回错误
- [ ] 修改 `CURRENT_LOG_LEVEL` 编译参数后，低级别日志确实不输出
- [ ] 能解释 `do { } while(0)` 在宏定义中的作用

---

#### Day 14 -- 周末: 环形缓冲区 (总计 4-5h)

**上午: 理论 + 设计 (1.5h)**
- 阅读资料: 搜索 "ring buffer embedded C implementation"
- 环形缓冲区原理:
  - 固定大小的数组 + 读指针(head) + 写指针(tail)
  - 写入: `buf[tail] = data; tail = (tail + 1) % size;`
  - 读取: `data = buf[head]; head = (head + 1) % size;`
  - 满判断: `(tail + 1) % size == head`（牺牲一个位置区分满和空）
  - 空判断: `head == tail`
- 在纸上画出连续写入/读取的过程（画圆形，标注 head/tail 的移动）
- `volatile` 的作用: 在中断+主循环场景下，head 在中断中修改，main 中读取，必须 volatile

**下午: 实现 + 测试 (3h)**

文件结构:
```
day14_ring_buffer/
├── ring_buffer.h
├── ring_buffer.c
├── test_ring_buffer.c
└── Makefile
```

`ring_buffer.h`:
```c
#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uint8_t *buffer;
    size_t size;          // 缓冲区总大小（必须是 2 的幂，方便优化取模）
    volatile size_t head; // 读位置
    volatile size_t tail; // 写位置
} RingBuffer;

int    rb_init(RingBuffer *rb, uint8_t *buf, size_t size);
void   rb_reset(RingBuffer *rb);
size_t rb_write(RingBuffer *rb, const uint8_t *data, size_t len);  // 返回实际写入字节数
size_t rb_read(RingBuffer *rb, uint8_t *data, size_t len);         // 返回实际读取字节数
size_t rb_peek(const RingBuffer *rb, uint8_t *data, size_t len);   // 读取但不移动 head
size_t rb_available(const RingBuffer *rb);   // 可读字节数
size_t rb_free_space(const RingBuffer *rb);  // 可写字节数
bool   rb_is_empty(const RingBuffer *rb);
bool   rb_is_full(const RingBuffer *rb);
#endif
```

测试用例至少包括:
1. 空缓冲区读取返回 0
2. 写满后再写返回 0（或部分写入）
3. 写 5 字节，读 3 字节，再写 4 字节 -- 验证环形回绕
4. 交替写读 1000 次 -- 验证不越界
5. `rb_peek` 不改变 head

**验收标准**
- [ ] 所有 5+ 测试用例通过
- [ ] 用 `-fsanitize=address` 编译运行无错误
- [ ] 代码注释解释了为什么 head/tail 用 `volatile`
- [ ] 提交到 GitHub

---

### 第 3 周: 文件 I/O、多文件项目与调试 (Day 15-21)

---

#### Day 15 -- 文件操作 (总计 2.5h)

**理论学习 (40min)**
- 阅读: "C Programming: A Modern Approach" 第 22 章（输入/输出）
- 文本模式 vs 二进制模式: `"r"/"w"` vs `"rb"/"wb"` -- Windows 下文本模式会自动转换 `\n` <-> `\r\n`
- 嵌入式相关: Flash/EEPROM 存储都是二进制操作，必须掌握 `fread/fwrite`
- `fseek(fp, offset, SEEK_SET/SEEK_CUR/SEEK_END)` -- 定位文件指针

**实践操作 (90min)**
1. 创建 `day15/kv_store.h`:
   ```c
   #define KEY_MAX_LEN   16
   #define VALUE_MAX_LEN 64
   #define MAX_ENTRIES   128

   typedef struct {
       char key[KEY_MAX_LEN];
       uint8_t value[VALUE_MAX_LEN];
       uint16_t value_len;
       uint8_t valid;        // 1=有效, 0=已删除
   } KV_Entry;              // 固定大小，方便二进制读写

   int kv_open(const char *filename);   // 打开或创建 KV 文件
   int kv_write(const char *key, const uint8_t *value, uint16_t len);
   int kv_read(const char *key, uint8_t *value, uint16_t *len);
   int kv_delete(const char *key);
   void kv_list_all(void);              // 打印所有有效的 key-value
   void kv_close(void);
   ```
2. 实现思路:
   - 文件格式: 连续的 `KV_Entry` 结构体（固定大小，用 `fwrite/fread` 直接读写）
   - 查找: 遍历所有 entry，`strcmp(entry.key, key)` 且 `entry.valid == 1`
   - 删除: 将 `valid` 标志设为 0，用 `fseek` 定位后 `fwrite` 覆盖
3. 测试: 写入 3 个 KV 对，关闭文件，重新打开，验证能读回

**验收标准**
- [ ] 关闭文件后重新打开，数据完整读回
- [ ] 删除一个 key 后，再次读取返回"未找到"
- [ ] 能解释为什么用 `"rb"/"wb"` 而不是 `"r"/"w"`

---

#### Day 16 -- 多文件项目组织 (总计 2.5h)

**理论学习 (30min)**
- 头文件守卫: `#ifndef XXX_H / #define XXX_H / #endif` 或 `#pragma once`
- `static` 关键字在文件作用域: `static void helper(void)` 只在当前 `.c` 文件内可见（内部链接）
- `extern`: 声明变量/函数在其他文件中定义（外部链接）
- 原则: 头文件只放**声明**，不放**定义**（否则多文件包含会链接报错）

**实践操作 (2h)**
1. 将之前所有独立的库整理到统一项目结构:
   ```
   c-libs/
   ├── CMakeLists.txt          // (先用 Makefile，Day 26 再迁移到 CMake)
   ├── Makefile
   ├── include/
   │   ├── ring_buffer.h
   │   ├── linked_list.h
   │   ├── dynarray.h
   │   ├── string_utils.h
   │   └── bitops.h
   ├── src/
   │   ├── ring_buffer.c
   │   ├── linked_list.c
   │   ├── dynarray.c
   │   └── string_utils.c
   ├── tests/
   │   ├── test_ring_buffer.c
   │   ├── test_linked_list.c
   │   └── test_main.c
   └── examples/
       └── demo.c
   ```
2. 更新 Makefile 支持 `include/` 目录: `CFLAGS += -Iinclude`
3. 确保 `make` 一次编译所有库和测试
4. 验证: 在 `demo.c` 中 `#include "ring_buffer.h"` 等头文件都能正确找到

**验收标准**
- [ ] 项目结构清晰，头文件和源文件分离
- [ ] `make` 能一次构建整个项目，无 warning
- [ ] 每个 `.c` 文件的 `static` 辅助函数不会导致链接冲突

---

#### Day 17 -- Makefile 与构建系统 (总计 2.5h)

**理论学习 (40min)**
- Makefile 核心语法:
  ```makefile
  target: dependency1 dependency2
  	command           # 必须是 Tab 缩进
  ```
- 自动变量: `$@` (目标)、`$<` (第一个依赖)、`$^` (所有依赖)
- 模式规则: `%.o: %.c` -- 所有 `.c` 到 `.o` 的通用规则
- `.PHONY` 伪目标: `clean` 不是真实文件，需声明 `.PHONY: clean`
- 变量: `CC = gcc`、`CFLAGS = -Wall -Wextra`、`OBJS = $(SRCS:.c=.o)`

**实践操作 (90min)**
1. 为 `c-libs/` 编写完整 Makefile:
   ```makefile
   CC = gcc
   CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Iinclude
   CFLAGS_DEBUG = $(CFLAGS) -g -O0 -fsanitize=address
   SRCS = $(wildcard src/*.c)
   OBJS = $(SRCS:src/%.c=build/%.o)
   TEST_SRCS = $(wildcard tests/*.c)

   .PHONY: all clean test debug

   all: build/libclibs.a

   build/%.o: src/%.c | build
   	$(CC) $(CFLAGS) -c $< -o $@

   build:
   	mkdir -p build

   build/libclibs.a: $(OBJS)
   	ar rcs $@ $^

   test: build/libclibs.a
   	$(CC) $(CFLAGS_DEBUG) $(TEST_SRCS) -Lbuild -lclibs -o build/test_runner
   	./build/test_runner

   clean:
   	rm -rf build
   ```
2. 添加 `debug` 目标: 用 `-g -O0` 编译
3. 测试: `make clean && make && make test`

**验收标准**
- [ ] `make` 生成静态库 `libclibs.a`
- [ ] `make test` 编译并运行所有测试
- [ ] `make clean` 清理所有构建产物
- [ ] 修改一个 `.c` 文件后，`make` 只重新编译那一个文件（增量构建）

---

#### Day 18 -- GDB 调试 (总计 2.5h)

**理论学习 (30min)**
- GDB 核心命令速查:
  - `gdb ./program` -- 启动
  - `break main` 或 `b 42` -- 在函数或行号设断点
  - `run` 或 `r` -- 运行
  - `next` (n) -- 单步跳过函数调用
  - `step` (s) -- 单步进入函数
  - `print var` (p) -- 打印变量值
  - `print *ptr` -- 打印指针指向的值
  - `print arr[0]@5` -- 打印数组前 5 个元素
  - `watch var` -- 变量变化时自动暂停
  - `backtrace` (bt) -- 打印调用栈
  - `continue` (c) -- 继续运行
  - `quit` (q) -- 退出

**实践操作 (2h)**
1. 在链表代码中故意引入 3 个 bug -- 创建 `day18/buggy_list.c`:
   - Bug 1 (野指针): `list_remove` 中 free 节点后不把前驱的 next 指向下一个
   - Bug 2 (内存泄漏): `list_destroy` 中直接 `list->head = NULL` 没有 free 节点
   - Bug 3 (越界): `list_get(index)` 不检查 index 是否超出链表长度
2. 编译: `gcc -g -O0 -fsanitize=address buggy_list.c -o buggy`
3. 用 GDB 逐个定位:
   - Bug 1: 运行后段错误 -> `bt` 看调用栈 -> `p prev->next` 发现指向已 free 的内存
   - Bug 2: 用 ASan 检测泄漏 -> 定位到 destroy 函数
   - Bug 3: 设断点在 `list_get`，`p index` 和 `p list->count` 对比
4. 修复每个 bug，重新运行直到零错误

**验收标准**
- [ ] 能用 GDB 设断点、单步执行、打印变量
- [ ] 3 个 bug 全部修复，ASan 零报错
- [ ] 能用 `bt` 看到段错误时的完整调用栈

---

#### Day 19 -- 静态分析与编码规范 (总计 2.5h)

**理论学习 (30min)**
- 编译警告级别: `-Wall` (常见警告) -> `-Wextra` (额外警告) -> `-Werror` (警告视为错误)
- `cppcheck`: 开源静态分析工具，检测 null 指针解引用、数组越界、未初始化变量
- `valgrind` (Linux/WSL): 运行时内存检测，检测泄漏、越界读写、使用未初始化内存
- MISRA-C: 汽车行业的 C 编码规范，核心规则: 禁止 `goto`、限制指针嵌套层级、所有 switch 必须有 default

**实践操作 (2h)**
1. 安装 cppcheck: `sudo apt install cppcheck` (WSL) 或 Windows 安装包
2. 对 `c-libs/` 全部源码运行: `cppcheck --enable=all --suppress=missingIncludeSystem src/`
3. 修复所有报告的问题
4. 在 WSL 下运行 valgrind:
   ```bash
   gcc -g -O0 src/*.c tests/test_main.c -Iinclude -o test_runner
   valgrind --leak-check=full --show-leak-kinds=all ./test_runner
   ```
5. 修复所有内存问题直到 valgrind 输出:
   ```
   All heap blocks were freed -- no leaks are possible
   ERROR SUMMARY: 0 errors from 0 contexts
   ```
6. 将 `-Wall -Wextra -Werror` 加入 Makefile 的 CFLAGS，确保全项目零 warning

**验收标准**
- [ ] cppcheck 零报告（或全部是已知误报）
- [ ] valgrind 零泄漏零错误
- [ ] 全项目 `-Werror` 编译通过

---

#### Day 20 -- 状态机编程模式 (总计 2.5h)

**理论学习 (45min)**
- 有限状态机 (FSM) 概念: 系统在任意时刻处于某个确定的状态，接收输入后转移到另一个状态
- 两种实现方式:
  - **switch-case 状态机**: 简单直观，适合状态少的情况
  - **表驱动状态机**: 用二维数组/结构体数组描述状态转移，扩展性好
- 嵌入式中的应用: 协议字节流解析、设备状态管理、按键去抖动
- 画状态转移图: 在纸上画出 AT 指令解析的状态转移图

**实践操作 (90min)**
1. 创建 `day20/at_parser.h`:
   ```c
   typedef enum {
       AT_STATE_IDLE,           // 等待响应
       AT_STATE_CR_RECEIVED,    // 收到 \r
       AT_STATE_IN_LINE,        // 正在接收一行
       AT_STATE_LINE_CR,        // 行末收到 \r
   } AT_State;

   typedef enum {
       AT_RESULT_NONE,
       AT_RESULT_OK,
       AT_RESULT_ERROR,
       AT_RESULT_DATA,          // +XXX: ... 格式的数据行
   } AT_Result;

   typedef struct {
       AT_State state;
       char line_buf[128];
       int line_pos;
       AT_Result last_result;
   } AT_Parser;

   void at_parser_init(AT_Parser *p);
   AT_Result at_parser_feed(AT_Parser *p, uint8_t byte);  // 逐字节喂入，返回解析结果
   const char *at_parser_get_line(const AT_Parser *p);      // 获取最后一行完整数据
   ```
2. 实现 `at_parser_feed` -- 核心是一个 switch-case 状态机:
   - 逐字节接收，遇到 `\r\n` 表示一行结束
   - 行内容为 "OK" -> 返回 `AT_RESULT_OK`
   - 行内容为 "ERROR" -> 返回 `AT_RESULT_ERROR`
   - 行内容以 "+" 开头 -> 返回 `AT_RESULT_DATA`
3. 测试: 模拟喂入 `"+CWJAP:\"MyWiFi\"\r\nOK\r\n"` 的每个字节

**验收标准**
- [ ] 逐字节喂入 `"OK\r\n"` 后返回 `AT_RESULT_OK`
- [ ] 逐字节喂入 `"+CSQ: 25,99\r\nOK\r\n"` 后，先返回 DATA（内容为 `+CSQ: 25,99`），再返回 OK
- [ ] 状态机不会因为乱序输入而卡死

---

#### Day 21 -- 周末: UART 协议解析器 (总计 4-5h)

**项目说明**

综合运用 Day 1-20 所学，实现一个**模拟 UART 协议解析器**（用文件输入模拟串口字节流）。

帧格式:
```
[0xAA] [LEN] [CMD] [DATA_0 ... DATA_N] [CRC8]
  帧头   数据长度  命令码    数据域           校验
  1字节   1字节   1字节    LEN 字节          1字节
```

**文件结构:**
```
day21_uart_parser/
├── protocol.h        // 帧结构定义、CRC8 函数声明
├── protocol.c        // CRC8 实现
├── frame_parser.h    // 状态机解析器
├── frame_parser.c    // 状态机实现 (WAIT_HEADER -> GET_LEN -> GET_CMD -> GET_DATA -> GET_CRC)
├── cmd_dispatch.h    // 命令分发表 (函数指针)
├── cmd_dispatch.c
├── main.c            // 从二进制文件读取字节流，喂入解析器
├── test_data.bin     // 测试用二进制数据（包含正确帧和错误帧）
└── Makefile
```

**实现步骤:**
1. 定义帧结构和状态枚举 (45min)
2. 实现 CRC8 计算 (15min, 复用 Day 12 的代码)
3. 实现状态机解析器 `parser_feed_byte()` (60min)
4. 实现命令分发: CMD=0x01 读温度、CMD=0x02 读湿度、CMD=0x03 控制 LED (30min)
5. 编写测试数据生成程序: 生成正确帧和故意损坏的帧 (30min)
6. 集成测试 + 修复 bug (60min)

**验收标准**
- [ ] 正确帧: 解析成功并分发到对应命令处理函数
- [ ] CRC 错误帧: 检测到并丢弃，打印警告
- [ ] 帧头错误: 丢弃非 0xAA 的字节，继续等待
- [ ] 连续多帧: 正确解析背靠背的多个帧
- [ ] 代码提交到 GitHub，README 包含帧格式说明

---

### 第 4 周: C 语言高级专题 (Day 22-28)

---

#### Day 22 -- 函数指针与回调 (总计 2.5h)

**理论学习 (40min)**
- 回调函数: 将函数 A 的指针传给函数 B，让 B 在适当时机调用 A
- "依赖反转": 底层模块不依赖上层实现，通过回调解耦
- 嵌入式中的典型场景: 中断回调、协议解析完成回调、定时器回调

**实践操作 (90min)**
1. 创建 `day22/event_system.h`:
   ```c
   #define MAX_EVENTS     16
   #define MAX_LISTENERS  8

   typedef uint8_t EventID;
   typedef void (*EventCallback)(EventID event, void *data);

   void event_init(void);
   int event_register(EventID event, EventCallback cb);
   int event_unregister(EventID event, EventCallback cb);
   void event_emit(EventID event, void *data);
   ```
2. 内部实现: `EventCallback listeners[MAX_EVENTS][MAX_LISTENERS]` 二维数组
3. `event_emit`: 遍历该事件的所有已注册回调，逐个调用
4. 测试:
   ```c
   enum { EVT_TEMP_HIGH = 0, EVT_BUTTON_PRESS = 1 };
   void on_temp_high(EventID e, void *data) {
       float *temp = (float *)data;
       printf("温度过高: %.1f\n", *temp);
   }
   // 注册、触发、验证
   ```

**验收标准**
- [ ] 一个事件可以注册多个回调，emit 时全部被调用
- [ ] unregister 后不再被调用
- [ ] 满员时 register 返回错误码

---

#### Day 23 -- 内存布局与嵌入式相关 (总计 2.5h)

**理论学习 (45min)**
- 程序内存布局:
  - `.text` -- 代码段（Flash 中，只读）
  - `.rodata` -- 只读数据（字符串字面量、const 变量）
  - `.data` -- 已初始化的全局/静态变量（启动时从 Flash 拷贝到 RAM）
  - `.bss` -- 未初始化的全局/静态变量（启动时清零）
  - `heap` -- 动态分配（向上增长）
  - `stack` -- 函数调用（向下增长）
- `volatile`: 告诉编译器不要优化对该变量的读写（每次都从内存读取）
  - 场景: 硬件寄存器、中断共享变量、多线程共享变量
- 链接脚本 `.ld`: 定义 Flash 和 RAM 的起始地址和大小，将各段放到正确位置

**实践操作 (90min)**
1. 创建 `day23/sections.c`:
   ```c
   #include <stdio.h>
   int global_init = 42;                          // .data
   int global_uninit;                              // .bss
   const char msg[] = "Read Only";                 // .rodata
   static int static_var = 100;                    // .data
   __attribute__((section(".my_section")))
   int custom_var = 0xDEADBEEF;                    // 自定义段

   int main(void) {
       int local = 10;                              // stack
       int *heap_var = malloc(sizeof(int));          // heap
       printf("global_init  addr: %p\n", &global_init);
       printf("global_uninit addr: %p\n", &global_uninit);
       printf("msg          addr: %p\n", msg);
       printf("local        addr: %p\n", &local);
       printf("heap_var     addr: %p\n", heap_var);
       free(heap_var);
       return 0;
   }
   ```
2. 编译后用 `nm sections` 查看各符号的地址和所在段（T=text, D=data, B=bss, R=rodata）
3. 用 `objdump -h sections` 查看各段的大小
4. volatile 实验:
   ```c
   // 不加 volatile，gcc -O2 可能优化掉循环
   volatile int flag = 0;
   while (!flag) { /* 等待中断修改 flag */ }
   // 编译: gcc -O2 -S 对比加/不加 volatile 的汇编差异
   ```

**验收标准**
- [ ] 能指出每个变量在哪个段
- [ ] `nm` 输出中能找到 `custom_var` 在 `.my_section`
- [ ] 对比汇编能看到 volatile 的区别

---

#### Day 24 -- C 语言实现面向对象 (总计 2.5h)

**实践操作 (全天以实践为主, 2.5h)**
1. 创建 `day24/shape.h`:
   ```c
   // "虚函数表"
   typedef struct Shape Shape;
   typedef struct {
       float (*area)(const Shape *self);
       void (*draw)(const Shape *self);
       void (*destroy)(Shape *self);
   } ShapeVTable;

   // "基类"
   struct Shape {
       const ShapeVTable *vtable;
   };

   // "派生类"
   typedef struct {
       Shape base;     // 必须是第一个成员（这样 Shape* 和 Circle* 地址相同）
       float radius;
   } Circle;

   typedef struct {
       Shape base;
       float width, height;
   } Rectangle;

   Shape *circle_create(float radius);
   Shape *rectangle_create(float width, float height);
   // 通用接口（通过虚函数表调用）
   static inline float shape_area(const Shape *s) { return s->vtable->area(s); }
   static inline void shape_draw(const Shape *s) { s->vtable->draw(s); }
   static inline void shape_destroy(Shape *s) { s->vtable->destroy(s); }
   ```
2. 实现: 每个"子类"有自己的 vtable 静态实例
3. 测试: `Shape *shapes[] = { circle_create(5), rectangle_create(3, 4) };` 用循环调用 `shape_area`

**验收标准**
- [ ] 通过基类指针 `Shape *` 能正确调用子类的 `area()` (多态)
- [ ] `shape_destroy` 正确释放各子类的内存
- [ ] 能解释这种模式与 C++ 虚函数表的对应关系

---

#### Day 25 -- 线程安全与并发基础 (总计 2.5h)

**理论学习 (40min)**
- `#include <pthread.h>` -- POSIX 线程 (Windows 下用 WSL 或 MinGW 的 pthread 实现)
- `pthread_create/pthread_join` -- 创建和等待线程
- `pthread_mutex_init/lock/unlock/destroy` -- 互斥锁
- 竞态条件: 多线程同时读写共享变量导致的不确定行为
- 生产者-消费者模型: 经典并发模式，RTOS 中用队列实现

**实践操作 (90min)**
1. 创建 `day25/producer_consumer.c`:
   - 共享资源: Day 14 的环形缓冲区
   - 生产者线程: 每 10ms 写入一个递增数字
   - 消费者线程: 每 15ms 读取并打印
   - 用 `pthread_mutex` 保护 `rb_write/rb_read`
2. 运行 10 秒，验证:
   - 数据不丢失、不重复
   - 无段错误、无数据损坏
3. 去掉互斥锁再运行，观察数据损坏现象

**验收标准**
- [ ] 有锁版本运行 10 秒无错误
- [ ] 无锁版本能观察到数据损坏（打印出异常值）
- [ ] 能解释为什么需要互斥锁

---

#### Day 26 -- CMake 构建系统 (总计 2.5h)

**理论学习 (30min)**
- CMake 基础:
  ```cmake
  cmake_minimum_required(VERSION 3.16)
  project(c-libs C)
  add_library(ring_buffer STATIC src/ring_buffer.c)
  target_include_directories(ring_buffer PUBLIC include)
  add_executable(test_runner tests/test_main.c)
  target_link_libraries(test_runner PRIVATE ring_buffer)
  ```
- `STATIC` vs `SHARED` 库
- `PUBLIC/PRIVATE/INTERFACE` 传播规则
- 构建流程: `mkdir build && cd build && cmake .. && make`

**实践操作 (2h)**
1. 为 `c-libs/` 项目编写 `CMakeLists.txt`:
   - 将每个库模块定义为 STATIC 库
   - 定义 test_runner 可执行文件
   - 添加自定义目标 `run_tests`
2. 测试: `cmake -B build && cmake --build build && ./build/test_runner`
3. 尝试生成 Debug 和 Release 配置:
   - `cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug`
   - `cmake -B build-release -DCMAKE_BUILD_TYPE=Release`

**验收标准**
- [ ] `cmake --build build` 编译成功
- [ ] Debug 版本包含调试信息 (-g)，Release 版本包含优化 (-O2)
- [ ] 能解释 CMake 的 out-of-source build 优势

---

#### Day 27 -- 单元测试 (总计 2.5h)

**理论学习 (30min)**
- Unity 测试框架 (https://github.com/ThrowTheSwitch/Unity): 纯 C，一个 `.c` + 一个 `.h` 文件
- 核心宏: `TEST_ASSERT_EQUAL(expected, actual)` / `TEST_ASSERT_TRUE(condition)` / `TEST_ASSERT_NULL(ptr)`
- 测试结构: `void setUp(void) {}` + `void tearDown(void) {}` + `void test_xxx(void) {}`

**实践操作 (2h)**
1. 下载 Unity: `git clone` 或直接复制 `unity.c` + `unity.h` + `unity_internals.h`
2. 为环形缓冲区编写测试 `tests/test_ring_buffer.c`:
   ```c
   void test_rb_init_success(void) {
       uint8_t buf[64];
       RingBuffer rb;
       TEST_ASSERT_EQUAL(0, rb_init(&rb, buf, 64));
       TEST_ASSERT_TRUE(rb_is_empty(&rb));
   }
   void test_rb_write_read_single(void) { ... }
   void test_rb_write_full(void) { ... }
   void test_rb_wrap_around(void) { ... }
   void test_rb_peek_no_consume(void) { ... }
   // ... 至少 15 个测试用例
   ```
3. 为链表编写测试 `tests/test_linked_list.c`
4. 创建 `tests/test_main.c` 运行所有测试

**验收标准**
- [ ] 至少 15 个测试用例全部 PASS
- [ ] 至少包含: 正常路径、边界条件(空/满)、错误输入 三类测试
- [ ] CMake 中添加 `ctest` 支持

---

#### Day 28 -- 阶段一综合项目: 嵌入式日志系统 (总计 5-6h)

**项目: 嵌入式日志系统 `elog`**

```
day28_elog/
├── CMakeLists.txt
├── include/
│   ├── elog.h           // 公共 API
│   └── elog_config.h    // 编译期配置（日志级别、缓冲区大小等）
├── src/
│   ├── elog.c           // 核心实现
│   └── elog_output.c    // 输出后端（终端 + 文件）
├── tests/
│   ├── test_elog.c
│   └── unity/           // Unity 框架
└── examples/
    └── demo.c
```

**elog.h API 设计:**
```c
typedef enum { ELOG_DEBUG, ELOG_INFO, ELOG_WARN, ELOG_ERROR } ElogLevel;
typedef void (*ElogOutputFunc)(const char *msg, size_t len);

int elog_init(ElogLevel min_level, size_t buf_size);
void elog_deinit(void);
void elog_set_level(ElogLevel level);
int elog_add_output(ElogOutputFunc func);  // 注册输出后端（最多 4 个）
void elog_log(ElogLevel level, const char *file, int line, const char *fmt, ...);

#define ELOG_D(fmt, ...) elog_log(ELOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG_I(fmt, ...) elog_log(ELOG_INFO,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG_W(fmt, ...) elog_log(ELOG_WARN,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG_E(fmt, ...) elog_log(ELOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
```

**实现要点:**
- 内部使用环形缓冲区缓存日志（复用 Day 14 的代码）
- `elog_log` 中用 `vsnprintf` 格式化消息，写入环形缓冲区
- 内置输出后端: `elog_output_stdout`(打印到终端) + `elog_output_file`(写入文件)
- 线程安全: 用 `pthread_mutex` 保护环形缓冲区
- 时间戳: `clock_gettime(CLOCK_MONOTONIC, &ts)` 获取毫秒级时间戳

**验收标准**
- [ ] `ELOG_I("Temperature: %.1f", 25.3)` 输出 `[12345ms][INF][main.c:42] Temperature: 25.3`
- [ ] 设置级别为 WARN 后，DEBUG 和 INFO 不输出
- [ ] 同时输出到终端和文件
- [ ] 多线程同时写日志不会乱码或崩溃
- [ ] Unity 测试全部 PASS
- [ ] **推送到 GitHub，第一个正式的独立库项目**

---

## 阶段二: C++ 核心 (第 5-7 周 / Day 29-49)

> 目标: 掌握现代 C++ (C++17) 的核心特性，能用 RAII 和模板编写安全高效的嵌入式友好代码

### 第 5 周: C++ 基础与面向对象 (Day 29-35)

---

#### Day 29 -- C++ 入门与命名空间 (总计 2.5h)

**理论学习 (45min)**
- 阅读: "A Tour of C++" (Bjarne Stroustrup) 第 1 章 或在线教程 learncpp.com Chapter 1-2
- C vs C++ 核心区别:
  - C++ 是 C 的超集: 所有合法 C 代码（几乎）都是合法 C++ 代码
  - 新增: 类、模板、引用、命名空间、函数重载、异常、STL
  - 编译: 用 `g++` 而非 `gcc`，文件后缀 `.cpp`
- `namespace`: 避免命名冲突 -- `namespace sensor { float read(); }` -> `sensor::read()`
- 引用 `&` vs 指针 `*`:
  - 引用是别名，必须初始化，不能重新绑定，不能为 NULL
  - 函数参数传引用: `void process(int &x)` -- 效果同指针但语法更安全
- `auto`: 编译器自动推断类型 -- `auto x = 42;` -> int, `auto s = std::string("hi");`

**实践操作 (90min)**
1. 创建 `day29/calc.cpp`:
   ```cpp
   #include <iostream>
   #include <string>

   namespace calc {
       double add(double a, double b) { return a + b; }
       double sub(double a, double b) { return a - b; }
       double mul(double a, double b) { return a * b; }
       // div: 用引用参数返回错误状态
       double div(double a, double b, bool &error) {
           if (b == 0.0) { error = true; return 0.0; }
           error = false;
           return a / b;
       }
   }
   // 任务: 在 main 中用 std::cin 读取输入，用 switch 分发计算，用 std::cout 输出
   // 对比 Day 3 的 C 版本 -- 引用参数比指针参数更简洁
   ```
2. 创建 `day29/reference_demo.cpp`:
   ```cpp
   // 对比三种参数传递:
   void by_value(int x)        { x = 100; }   // 不影响调用者
   void by_pointer(int *x)     { *x = 100; }  // 影响调用者
   void by_reference(int &x)   { x = 100; }   // 影响调用者，但语法更干净
   // 任务: 分别调用三个函数，打印前后值，验证差异
   ```
3. 编译: `g++ -std=c++17 -Wall -Wextra calc.cpp -o calc`

**验收标准**
- [ ] 能解释引用和指针的 3 个核心区别
- [ ] 计算器 C++ 版本功能与 C 版本一致，代码更简洁
- [ ] 理解 `namespace` 避免命名冲突的作用

---

#### Day 30 -- 类与对象 (总计 2.5h)

**理论学习 (40min)**
- 阅读: learncpp.com Chapter 13-14（类基础）
- 类的组成: 成员变量（数据）+ 成员函数（操作）
- 构造函数: 对象创建时自动调用，用于初始化
  - 成员初始化列表: `SensorData(float t) : temperature_(t) {}` -- 比在构造函数体内赋值更高效
- 析构函数: 对象销毁时自动调用，用于清理资源
- `public/private/protected`: 默认 class 是 private，struct 是 public
- `this` 指针: 指向当前对象的指针，`this->member` 或隐式使用

**实践操作 (90min)**
1. 创建 `day30/sensor_data.h`:
   ```cpp
   #pragma once
   #include <cstdint>

   class SensorData {
   public:
       SensorData();  // 默认构造
       SensorData(float temperature, float humidity, float pressure);

       // Getters
       float temperature() const { return temperature_; }
       float humidity() const { return humidity_; }
       float pressure() const { return pressure_; }
       uint32_t timestamp_ms() const { return timestamp_ms_; }

       // Setters
       void set_temperature(float t) { temperature_ = t; }
       void set_humidity(float h) { humidity_ = h; }
       void set_pressure(float p) { pressure_ = p; }

       // 业务方法
       void calibrate(float temp_offset, float hum_offset);
       float heat_index() const;        // 体感温度计算
       bool is_valid() const;            // 数据有效性检查
       void print() const;

   private:
       float temperature_{0.0f};         // C++11 默认成员初始化
       float humidity_{0.0f};
       float pressure_{0.0f};
       uint32_t timestamp_ms_{0};

       static constexpr float TEMP_MIN = -40.0f;
       static constexpr float TEMP_MAX = 85.0f;
   };
   ```
2. 实现 `day30/sensor_data.cpp`:
   - 构造函数用初始化列表
   - `calibrate`: 加偏移量
   - `is_valid`: 检查温度/湿度/气压在合理范围内
   - `print`: `printf("[%u ms] T=%.1f H=%.1f P=%.1f", ...)`
3. 测试: 创建多个 SensorData 对象，校准，打印

**验收标准**
- [ ] 构造函数使用初始化列表（不是在函数体内赋值）
- [ ] `const` 成员函数正确标注（getter 和 print 都应该是 const）
- [ ] `is_valid` 对超范围数据返回 false

---

#### Day 31 -- RAII 与资源管理 (总计 2.5h)

**理论学习 (45min)**
- RAII 核心思想: 构造函数获取资源，析构函数释放资源 -- 利用栈对象的生命周期自动管理资源
- 为什么重要: 即使发生异常（或提前 return），析构函数也会被调用，不会泄漏
- Rule of Three: 如果你定义了析构函数/拷贝构造/拷贝赋值中任何一个，你应该定义全部三个
- Rule of Five: C++11 新增移动构造/移动赋值，共五个
- Rule of Zero: 如果用标准库容器/智能指针管理资源，你不需要自定义任何一个
- `= delete`: 禁止某个操作 -- `FileHandle(const FileHandle&) = delete;`
- 移动语义: `std::move` 将资源"转移"而非"复制"（源对象变为空壳）

**实践操作 (90min)**
1. 创建 `day31/file_handle.h`:
   ```cpp
   #pragma once
   #include <cstdio>
   #include <utility>  // std::exchange

   class FileHandle {
   public:
       explicit FileHandle(const char *path, const char *mode);
       ~FileHandle();                                    // 自动关闭文件

       FileHandle(const FileHandle &) = delete;           // 禁止拷贝
       FileHandle &operator=(const FileHandle &) = delete;

       FileHandle(FileHandle &&other) noexcept;           // 允许移动
       FileHandle &operator=(FileHandle &&other) noexcept;

       bool is_open() const { return fp_ != nullptr; }
       FILE *get() const { return fp_; }

       size_t write(const void *data, size_t size);
       size_t read(void *data, size_t size);

   private:
       FILE *fp_{nullptr};
   };
   ```
2. 实现移动构造: `FileHandle(FileHandle &&other) noexcept : fp_(std::exchange(other.fp_, nullptr)) {}`
3. 测试:
   ```cpp
   {
       FileHandle f("test.txt", "w");
       f.write("Hello", 5);
   } // 离开作用域，自动关闭文件

   // FileHandle f2 = f;  // 编译错误! 拷贝被删除
   FileHandle f3 = std::move(f);  // OK: 移动
   ```

**验收标准**
- [ ] 离开作用域自动关闭文件（用调试打印验证析构函数被调用）
- [ ] 拷贝操作编译报错
- [ ] 移动后，源对象 `is_open()` 返回 false
- [ ] 能口头解释 RAII 解决了什么问题

---

#### Day 32 -- 继承与多态 (总计 2.5h)

**理论学习 (40min)**
- 继承: `class Circle : public Shape { ... };`
- `virtual` 函数: 允许通过基类指针调用派生类的实现（运行时多态）
- 纯虚函数: `virtual float area() const = 0;` -- 包含纯虚函数的类是抽象类，不能实例化
- `override`: 显式标注重写虚函数，拼写错误时编译报错
- 虚析构函数: 通过基类指针 `delete` 派生类对象时**必须**有虚析构函数，否则内存泄漏

**实践操作 (90min)**
1. 创建 `day32/shapes.h` -- C++ 版本的形状层次结构:
   ```cpp
   #pragma once
   #include <cmath>
   #include <cstdio>

   class Shape {
   public:
       virtual ~Shape() = default;          // 虚析构函数
       virtual float area() const = 0;       // 纯虚函数
       virtual void draw() const = 0;
       virtual const char *name() const = 0;
   };

   class Circle : public Shape {
   public:
       explicit Circle(float r) : radius_(r) {}
       float area() const override { return 3.14159f * radius_ * radius_; }
       void draw() const override { printf("Drawing circle r=%.1f\n", radius_); }
       const char *name() const override { return "Circle"; }
   private:
       float radius_;
   };

   class Rectangle : public Shape { /* 类似实现 */ };
   class Triangle : public Shape { /* 加分项 */ };
   ```
2. 测试:
   ```cpp
   #include <memory>
   #include <vector>
   std::vector<std::unique_ptr<Shape>> shapes;
   shapes.push_back(std::make_unique<Circle>(5.0f));
   shapes.push_back(std::make_unique<Rectangle>(3.0f, 4.0f));
   for (const auto &s : shapes) {
       printf("%s: area = %.2f\n", s->name(), s->area());
   }
   // 离开作用域，unique_ptr 自动 delete，虚析构函数确保正确释放
   ```
3. 对比 Day 24 的 C 版本: 代码行数差异、可维护性差异

**验收标准**
- [ ] 通过基类指针正确调用派生类方法
- [ ] 去掉 `override` 后故意拼错函数名，确认编译器不报错（这就是为什么要用 override）
- [ ] `unique_ptr` 自动管理生命周期，无泄漏

---

#### Day 33 -- 运算符重载与友元 (总计 2.5h)

**理论学习 (30min)**
- 运算符重载: 让自定义类型支持 `+` `-` `==` `<<` 等运算符
- `operator<<`: 与 `std::ostream` 配合，通常声明为友元: `friend std::ostream& operator<<(std::ostream &os, const T &obj);`
- `operator==` / `operator!=`: C++20 有 `<=>` 太空船运算符，但嵌入式 C++17 用传统方式
- `operator[]`: 下标访问
- 原则: 不要滥用重载，只在语义清晰时使用

**实践操作 (2h)**
1. 为 Day 30 的 `SensorData` 添加运算符:
   ```cpp
   // sensor_data.h 中添加:
   friend std::ostream &operator<<(std::ostream &os, const SensorData &d);
   bool operator==(const SensorData &other) const;
   bool operator!=(const SensorData &other) const { return !(*this == other); }
   SensorData operator+(const SensorData &other) const;  // 取两个传感器的平均值
   ```
2. 实现 `operator<<`:
   ```cpp
   std::ostream &operator<<(std::ostream &os, const SensorData &d) {
       os << "[" << d.timestamp_ms_ << "ms] T=" << d.temperature_
          << " H=" << d.humidity_ << " P=" << d.pressure_;
       return os;
   }
   ```
3. 测试: `std::cout << data1 << std::endl;` 和 `if (data1 == data2) ...`

**验收标准**
- [ ] `std::cout << sensorData` 输出格式化字符串
- [ ] `==` 比较浮点数时使用 epsilon 容差（不直接用 `==`）
- [ ] 能解释为什么 `operator<<` 需要声明为 `friend`

---

#### Day 34 -- 异常处理与错误码 (总计 2.5h)

**理论学习 (40min)**
- C++ 异常: `throw` 抛出 -> `try/catch` 捕获
- 嵌入式中禁用异常的原因:
  - 二进制体积增大 10-30%（异常表数据）
  - 运行时开销不确定（不利于实时系统）
  - 编译选项: `-fno-exceptions`
- 替代方案:
  - 错误码（C 风格，最常用）
  - `std::optional<T>`: 值或空，表示"可能失败"
  - `Result<T, E>` 模式（类似 Rust）: 值或错误
  - `std::expected<T, E>` (C++23): 标准化的 Result

**实践操作 (90min)**
1. 创建 `day34/result.h` -- 实现简化版 Result:
   ```cpp
   #pragma once
   #include <variant>
   #include <utility>

   template<typename T, typename E>
   class Result {
   public:
       static Result ok(T value) { return Result(std::move(value)); }
       static Result err(E error) { return Result(std::move(error)); }

       bool is_ok() const { return std::holds_alternative<T>(data_); }
       bool is_err() const { return !is_ok(); }

       const T &value() const { return std::get<T>(data_); }
       const E &error() const { return std::get<E>(data_); }

       // 链式调用: 如果 ok 则执行 func
       template<typename F>
       auto and_then(F func) const -> decltype(func(std::declval<T>())) {
           if (is_ok()) return func(value());
           return Result::err(error());
       }

   private:
       explicit Result(T value) : data_(std::move(value)) {}
       explicit Result(E error) : data_(std::move(error)) {}
       std::variant<T, E> data_;
   };
   ```
2. 测试:
   ```cpp
   enum class Error { DivByZero, Overflow, InvalidInput };
   Result<double, Error> safe_div(double a, double b) {
       if (b == 0.0) return Result<double, Error>::err(Error::DivByZero);
       return Result<double, Error>::ok(a / b);
   }
   auto r = safe_div(10.0, 0.0);
   if (r.is_err()) printf("Error!\n");
   ```

**验收标准**
- [ ] `Result<T, E>` 能正确区分 ok 和 err
- [ ] 能用 `-fno-exceptions` 编译整个项目
- [ ] 能解释嵌入式中为什么不用异常

---

#### Day 35 -- 周末: C++ 环形缓冲区模板类 (总计 4-5h)

**任务: 实现 `RingBuffer<T, N>` 模板类**

```cpp
#pragma once
#include <array>
#include <cstddef>
#include <optional>

template<typename T, size_t N>
class RingBuffer {
    static_assert(N > 0, "Buffer size must be > 0");
    static_assert((N & (N - 1)) == 0, "N must be power of 2");  // 方便用位运算取模
public:
    bool push(const T &item);
    std::optional<T> pop();
    std::optional<T> peek() const;
    size_t size() const;
    size_t capacity() const { return N; }
    bool empty() const { return head_ == tail_; }
    bool full() const { return size() == N; }
    void clear() { head_ = tail_ = 0; }
private:
    std::array<T, N + 1> buffer_{};  // +1 用于区分满和空
    size_t head_{0};
    size_t tail_{0};
    size_t mask() const { return N; }  // (N+1) 取模，但这里简化
};
```

**实现要点:**
- 完全在头文件中实现（模板类必须如此）
- 用 `std::optional<T>` 作为返回值，比 C 版本的 error code 更优雅
- 编写 15+ 个 Google Test 或 Catch2 测试用例（或继续用 Unity 的 C++ 兼容模式）
- 对比 Day 14 的 C 版本: 类型安全、接口更清晰、无需手动管理缓冲区指针

**验收标准**
- [ ] `RingBuffer<int, 16>` 和 `RingBuffer<SensorData, 8>` 都能正常工作
- [ ] `static_assert` 在 N 不是 2 的幂时编译报错
- [ ] 15+ 测试用例全部通过
- [ ] 整个类零堆分配（`std::array` 是栈上的固定数组）

---

### 第 6 周: 模板与标准库 (Day 36-42)

---

#### Day 36 -- 模板基础 (总计 2.5h)

**理论学习 (45min)**
- 阅读: learncpp.com Chapter 26（模板）
- 函数模板: `template<typename T> T max(T a, T b) { return (a > b) ? a : b; }`
- 类模板: `template<typename T, size_t N> class FixedArray { T data[N]; };`
- 模板特化: 为特定类型提供不同实现
- 非类型模板参数: `template<size_t N>` -- 编译期常量，用于指定数组大小
- 模板实例化: 编译器为每种使用到的类型生成一份代码 -> 二进制可能膨胀

**实践操作 (90min)**
1. 创建 `day36/sort.h`:
   ```cpp
   #pragma once
   #include <functional>

   // 通用插入排序 -- 对迭代器范围排序
   template<typename It, typename Comp = std::less<>>
   void insertion_sort(It begin, It end, Comp comp = Comp{}) {
       for (auto it = begin + 1; it != end; ++it) {
           auto key = *it;
           auto j = it - 1;
           while (j >= begin && comp(key, *j)) {
               *(j + 1) = *j;
               --j;
           }
           *(j + 1) = key;
       }
   }
   ```
2. 测试: 对 `int[]`、`float[]`、`SensorData[]` 排序（SensorData 按温度排序）
3. 实现一个 `template<typename T, size_t N> class StaticVector` -- 固定容量的 vector:
   - 内部用 `std::array<T, N>` + `size_t count_`
   - `push_back/pop_back/operator[]/size/capacity/begin/end`

**验收标准**
- [ ] `insertion_sort` 能对不同类型排序
- [ ] 自定义比较器能实现降序排序
- [ ] `StaticVector<int, 32>` 的 `sizeof` 等于 `32 * sizeof(int) + sizeof(size_t)`（无堆分配）

---

#### Day 37 -- STL 容器 (总计 2.5h)

**理论学习 (40min)**
- `std::array<T, N>`: 固定大小，栈上分配，嵌入式首选
- `std::vector<T>`: 动态大小，堆分配，嵌入式中谨慎使用（碎片问题）
- `std::string`: 动态字符串，堆分配，SSO (Small String Optimization) 对短字符串可能栈上
- `std::map<K, V>`: 红黑树，O(log n) 查找，内存开销大
- `std::unordered_map<K, V>`: 哈希表，O(1) 查找，内存开销更大
- 嵌入式选择原则: `std::array` > `StaticVector`(自写) > `std::vector` > 其他

**实践操作 (90min)**
1. 创建 `day37/device_registry.h` -- 固定容量设备注册表:
   ```cpp
   #pragma once
   #include <array>
   #include <cstdint>
   #include <cstring>
   #include <optional>

   struct DeviceInfo {
       uint32_t id;
       char name[32];
       char ip[16];
       bool online;
   };

   template<size_t MaxDevices = 16>
   class DeviceRegistry {
   public:
       bool add(const DeviceInfo &dev);
       bool remove(uint32_t id);
       std::optional<DeviceInfo> find(uint32_t id) const;
       size_t count() const { return count_; }
       const DeviceInfo &operator[](size_t idx) const { return devices_[idx]; }
   private:
       std::array<DeviceInfo, MaxDevices> devices_{};
       size_t count_{0};
   };
   ```
2. 测试: 添加 5 个设备，查找、删除、列举

**验收标准**
- [ ] 注册表满时 `add` 返回 false
- [ ] `find` 对不存在的 ID 返回 `std::nullopt`
- [ ] 整个类零堆分配

---

#### Day 38 -- STL 算法与迭代器 (总计 2.5h)

**理论学习 (30min)**
- 迭代器: 容器元素的"广义指针"，`begin()` 指向首元素，`end()` 指向末尾之后
- 范围 for: `for (const auto &item : container)` 内部使用 `begin()/end()`
- 常用算法: `std::find_if`、`std::sort`、`std::count_if`、`std::transform`、`std::accumulate`

**实践操作 (2h)**
1. 为 `DeviceRegistry` 添加迭代器支持:
   ```cpp
   // 添加到 DeviceRegistry 类中:
   const DeviceInfo *begin() const { return devices_.data(); }
   const DeviceInfo *end() const { return devices_.data() + count_; }
   DeviceInfo *begin() { return devices_.data(); }
   DeviceInfo *end() { return devices_.data() + count_; }
   ```
2. 测试范围 for 和 STL 算法:
   ```cpp
   DeviceRegistry reg;
   // ... 添加设备
   for (const auto &dev : reg) {
       printf("%s: %s\n", dev.name, dev.ip);
   }
   auto online_count = std::count_if(reg.begin(), reg.end(),
       [](const DeviceInfo &d) { return d.online; });
   ```

**验收标准**
- [ ] 范围 for 循环正常遍历所有设备
- [ ] `std::find_if` 能按条件查找设备
- [ ] `std::sort` + lambda 能按 ID 排序设备列表

---

#### Day 39 -- 智能指针 (总计 2.5h)

**理论学习 (40min)**
- `std::unique_ptr<T>`: 独占所有权，不可拷贝，可移动。析构时自动 delete
  - 零额外开销（和裸指针一样大小和速度）
  - 嵌入式首选智能指针
- `std::shared_ptr<T>`: 共享所有权，引用计数，最后一个释放
  - 有额外开销: 引用计数（原子操作）+ 控制块内存
  - 嵌入式中尽量避免
- `std::weak_ptr<T>`: 弱引用，不增加引用计数，用于打破循环引用
- `std::make_unique<T>(args...)`: C++14，推荐创建方式

**实践操作 (90min)**
1. 创建 `day39/cmd_parser_cpp.h` -- C++ 版本的命令解析器:
   ```cpp
   #pragma once
   #include <memory>
   #include <string>
   #include <functional>
   #include <array>

   class ICommand {
   public:
       virtual ~ICommand() = default;
       virtual const char *name() const = 0;
       virtual const char *help() const = 0;
       virtual void execute(int argc, const char **argv) = 0;
   };

   class CmdParser {
   public:
       bool register_cmd(std::unique_ptr<ICommand> cmd);
       void execute(const std::string &input);
       void print_help() const;
   private:
       static constexpr size_t MAX_CMDS = 16;
       std::array<std::unique_ptr<ICommand>, MAX_CMDS> commands_;
       size_t count_{0};
   };
   ```
2. 实现具体命令: `class HelloCommand : public ICommand { ... };`
3. 测试:
   ```cpp
   CmdParser parser;
   parser.register_cmd(std::make_unique<HelloCommand>());
   parser.execute("hello world");
   ```

**验收标准**
- [ ] `unique_ptr` 自动管理命令对象生命周期
- [ ] 命令注册使用移动语义: `register_cmd(std::move(cmd))`
- [ ] CmdParser 析构时所有命令对象自动释放

---

#### Day 40 -- Lambda 与 std::function (总计 2.5h)

**理论学习 (30min)**
- Lambda 语法: `[captures](params) -> return_type { body }`
  - `[]` 不捕获, `[=]` 值捕获全部, `[&]` 引用捕获全部, `[x, &y]` 混合
  - 嵌入式中: 无捕获的 lambda 可以转换为函数指针（与 C 回调兼容）
- `std::function<Ret(Args...)>`: 通用可调用对象包装器
  - 可存储: 函数指针、lambda、bind 表达式、仿函数
  - 有一定开销（类型擦除，可能堆分配）
  - 嵌入式中: 如果开销不可接受，用模板参数代替

**实践操作 (2h)**
1. 重写 Day 22 的事件系统为 C++ 版本:
   ```cpp
   using EventCallback = std::function<void(uint8_t event_id, const void *data)>;

   class EventBus {
   public:
       bool subscribe(uint8_t event_id, EventCallback cb);
       void publish(uint8_t event_id, const void *data);
   private:
       static constexpr size_t MAX_EVENTS = 16;
       static constexpr size_t MAX_SUBS = 8;
       struct Subscription {
           EventCallback callbacks[MAX_SUBS];
           size_t count{0};
       };
       std::array<Subscription, MAX_EVENTS> subs_;
   };
   ```
2. 测试使用 lambda:
   ```cpp
   EventBus bus;
   bus.subscribe(0, [](uint8_t id, const void *data) {
       auto temp = *static_cast<const float *>(data);
       printf("Event %d: temperature = %.1f\n", id, temp);
   });
   float t = 25.5f;
   bus.publish(0, &t);
   ```

**验收标准**
- [ ] Lambda 回调正确执行
- [ ] 一个事件支持多个订阅者
- [ ] 无捕获的 lambda 能转为函数指针: `void (*fp)(int) = [](int x) { ... };`

---

#### Day 41 -- 现代 C++ 特性精选 (总计 2.5h)

**理论学习 (40min)**
- `constexpr`: 编译期求值 -- 常量表达式在编译时计算，不占运行时
- `enum class`: 强类型枚举 -- 不隐式转换为 int，不同枚举的值不会冲突
- `std::optional<T>`: 可能有值也可能为空（比指针更安全的"可空"语义）
- `std::variant<A, B, C>`: 类型安全的联合体（替代 `union` + `enum` 手动标记）
- `std::string_view`: 非拥有的字符串引用，零拷贝（替代 `const char *` + `size_t`）
- 结构化绑定: `auto [x, y] = std::make_pair(1, 2);`

**实践操作 (90min)**
1. `constexpr` CRC 查找表:
   ```cpp
   constexpr uint8_t crc8_byte(uint8_t byte) {
       for (int i = 0; i < 8; ++i)
           byte = (byte & 0x80) ? (byte << 1) ^ 0x07 : (byte << 1);
       return byte;
   }
   constexpr std::array<uint8_t, 256> make_crc8_table() {
       std::array<uint8_t, 256> table{};
       for (int i = 0; i < 256; ++i)
           table[i] = crc8_byte(static_cast<uint8_t>(i));
       return table;
   }
   constexpr auto CRC8_TABLE = make_crc8_table();  // 编译期生成!
   ```
2. 验证: `static_assert(CRC8_TABLE[0] == 0x00);` -- 编译期检查
3. `enum class` + `std::variant` 练习: 用 `variant<int, float, std::string>` 重写 Day 13 的 Variant

**验收标准**
- [ ] CRC 表在编译期生成（用 `static_assert` 验证）
- [ ] `enum class` 的值不能隐式转为 int
- [ ] `std::variant` + `std::visit` 能正确分发到不同类型

---

#### Day 42 -- 周末: C++ 协议帧构建/解析器 (总计 4-5h)

**项目: 类型安全的协议帧库**

```
day42_frame_codec/
├── include/
│   ├── frame_codec.h       // 核心帧编解码
│   ├── crc.h               // CRC8/CRC16 (constexpr)
│   └── byte_writer.h       // 零拷贝字节写入器
├── tests/
│   └── test_frame_codec.cpp
└── CMakeLists.txt
```

核心设计:
```cpp
// 帧格式: [0xAA] [LEN] [CMD] [DATA...] [CRC8]
template<size_t MaxPayload = 64>
class FrameBuilder {
public:
    FrameBuilder &set_command(uint8_t cmd);
    FrameBuilder &add_uint8(uint8_t val);
    FrameBuilder &add_uint16(uint16_t val);     // 自动处理大端
    FrameBuilder &add_uint32(uint32_t val);
    FrameBuilder &add_bytes(const uint8_t *data, size_t len);
    std::optional<std::array<uint8_t, MaxPayload + 4>> build();  // 返回完整帧
private:
    // ...
};

template<size_t MaxPayload = 64>
class FrameParser {
public:
    enum class Result { Incomplete, Complete, CrcError, FrameError };
    Result feed(uint8_t byte);              // 状态机逐字节解析
    uint8_t command() const;
    const uint8_t *payload() const;
    size_t payload_len() const;
};
```

**验收标准**
- [ ] `FrameBuilder` 构建的帧能被 `FrameParser` 正确解析
- [ ] CRC 错误帧被 `FrameParser` 检测并报告
- [ ] 整个库零堆分配
- [ ] 单元测试覆盖: 空帧、最大帧、CRC 错误、不完整帧
- [ ] 推送到 GitHub

---

### 第 7 周: C++ 工程实践 (Day 43-49)

---

#### Day 43 -- 嵌入式 C++ 子集与限制 (总计 2.5h)

**理论学习 (40min)**
- 嵌入式 C++ 的"禁区": 异常 (`-fno-exceptions`)、RTTI (`-fno-rtti`)、大量动态分配
- `-fno-threadsafe-statics`: 禁用局部静态变量的线程安全初始化（RTOS 中自行管理）
- ETL (Embedded Template Library): 替代 STL，所有容器固定大小、无堆分配
  - `etl::vector<int, 32>`、`etl::string<64>`、`etl::map<int, float, 16>`
- `new/delete` 替代: 定制 `operator new` 使其从固定内存池分配，或直接禁用

**实践操作 (90min)**
1. 创建一个简单程序，编译两个版本:
   ```bash
   g++ -std=c++17 -Os demo.cpp -o demo_full          # 完整 C++ 特性
   g++ -std=c++17 -Os -fno-exceptions -fno-rtti demo.cpp -o demo_lite  # 嵌入式子集
   ```
2. 用 `size` 或 `ls -la` 比较二进制大小
3. 在程序中使用 `try/catch`、`typeid`、`dynamic_cast`，然后用 `-fno-exceptions -fno-rtti` 编译，观察报错
4. 尝试用 ETL: `git clone https://github.com/ETLCPP/etl`，将 `include/etl` 加入项目

**验收标准**
- [ ] 能量化禁用异常/RTTI 节省的二进制大小（通常 10-30%）
- [ ] 能解释嵌入式中为什么要用 `-fno-exceptions`
- [ ] 尝试了 ETL 的至少一个容器

---

#### Day 44 -- CMake 进阶与交叉编译 (总计 2.5h)

**理论学习 (30min)**
- 交叉编译: 在 x86 PC 上编译出 ARM 可执行文件
- 工具链: `arm-none-eabi-gcc` / `arm-none-eabi-g++` / `arm-none-eabi-objcopy`
- CMake Toolchain File: 告诉 CMake 用哪个编译器、链接器、sysroot

**实践操作 (2h)**
1. 安装: `sudo apt install gcc-arm-none-eabi` (WSL) 或 Windows 下载 ARM GCC
2. 创建 `toolchain-arm-cortex-m4.cmake`:
   ```cmake
   set(CMAKE_SYSTEM_NAME Generic)
   set(CMAKE_SYSTEM_PROCESSOR arm)
   set(CMAKE_C_COMPILER arm-none-eabi-gcc)
   set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
   set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
   set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
   set(CMAKE_C_FLAGS_INIT "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
   set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -fno-exceptions -fno-rtti")
   set(CMAKE_EXE_LINKER_FLAGS_INIT "-specs=nosys.specs -specs=nano.specs")
   ```
3. 编译: `cmake -B build-arm -DCMAKE_TOOLCHAIN_FILE=toolchain-arm-cortex-m4.cmake`
4. 验证: `file build-arm/program` 应显示 `ELF 32-bit LSB executable, ARM`

**验收标准**
- [ ] 交叉编译成功生成 ARM ELF 文件
- [ ] `arm-none-eabi-size` 能查看 text/data/bss 段大小
- [ ] 能解释 toolchain file 中每个编译选项的含义

---

#### Day 45 -- 设计模式在嵌入式中的应用 (总计 2.5h)

**实践操作 (2.5h, 以实践为主)**
1. **单例模式** -- 用于硬件外设:
   ```cpp
   class UART {
   public:
       static UART &instance() {
           static UART inst;
           return inst;
       }
       void send(const char *data, size_t len);
       UART(const UART &) = delete;
       UART &operator=(const UART &) = delete;
   private:
       UART() { /* 初始化硬件 */ }
   };
   ```
2. **状态模式** -- 设备状态管理:
   ```cpp
   class DeviceState {
   public:
       virtual ~DeviceState() = default;
       virtual void enter() = 0;
       virtual void update() = 0;
       virtual void exit() = 0;
       virtual const char *name() const = 0;
   };
   class IdleState : public DeviceState { /* LED 慢闪 */ };
   class RunningState : public DeviceState { /* LED 常亮，采集数据 */ };
   class ErrorState : public DeviceState { /* LED 快闪，停止采集 */ };

   class DeviceController {
   public:
       void transition_to(std::unique_ptr<DeviceState> new_state);
       void update();
   private:
       std::unique_ptr<DeviceState> state_;
   };
   ```
3. 测试: Idle -> Running -> Error -> Idle 的状态切换流程

**验收标准**
- [ ] 单例全局只有一个实例
- [ ] 状态切换时 `exit()` 和 `enter()` 被正确调用
- [ ] 能解释单例模式在嵌入式中的常见用途（外设驱动、日志系统）

---

#### Day 46 -- 并发编程 C++ 风格 (总计 2.5h)

**实践操作 (2.5h)**
1. 用 C++ 标准库重写 Day 25 的生产者-消费者:
   ```cpp
   #include <thread>
   #include <mutex>
   #include <condition_variable>

   RingBuffer<int, 64> buffer;
   std::mutex mtx;
   std::condition_variable cv_not_empty;
   std::condition_variable cv_not_full;

   void producer() {
       for (int i = 0; i < 1000; ++i) {
           std::unique_lock lock(mtx);
           cv_not_full.wait(lock, [&] { return !buffer.full(); });
           buffer.push(i);
           cv_not_empty.notify_one();
       }
   }
   void consumer() {
       for (int i = 0; i < 1000; ++i) {
           std::unique_lock lock(mtx);
           cv_not_empty.wait(lock, [&] { return !buffer.empty(); });
           auto val = buffer.pop();
           cv_not_full.notify_one();
           printf("Got: %d\n", val.value());
       }
   }
   ```
2. `std::atomic<bool>` 练习: 用原子变量做停止标志，无需互斥锁
3. 测试多生产者多消费者场景

**验收标准**
- [ ] 1000 个数据无丢失无重复
- [ ] `condition_variable` 正确避免忙等待
- [ ] 能解释 `unique_lock` vs `lock_guard` 的区别

---

#### Day 47 -- 编译期计算与元编程入门 (总计 2.5h)

**实践操作 (2.5h)**
1. `constexpr` 字节序转换:
   ```cpp
   constexpr uint16_t htons(uint16_t v) {
       return (v >> 8) | (v << 8);
   }
   constexpr uint32_t htonl(uint32_t v) {
       return ((v >> 24) & 0xFF) | ((v >> 8) & 0xFF00) |
              ((v << 8) & 0xFF0000) | ((v << 24) & 0xFF000000);
   }
   static_assert(htons(0x1234) == 0x3412);
   static_assert(htonl(0xDEADBEEF) == 0xEFBEADDE);
   ```
2. `if constexpr` 用于编译期分支:
   ```cpp
   template<typename T>
   void serialize(uint8_t *buf, T value) {
       if constexpr (sizeof(T) == 1) {
           buf[0] = static_cast<uint8_t>(value);
       } else if constexpr (sizeof(T) == 2) {
           buf[0] = static_cast<uint8_t>(value >> 8);
           buf[1] = static_cast<uint8_t>(value);
       } else if constexpr (sizeof(T) == 4) {
           // ... 大端序列化
       }
   }
   ```
3. 类型特征: `std::is_integral_v<T>` 配合 `static_assert` 限制模板参数

**验收标准**
- [ ] `static_assert` 验证编译期计算结果正确
- [ ] `if constexpr` 在编译期消除了无用分支（查看汇编确认）
- [ ] 能解释 `constexpr` 和 `const` 的区别

---

#### Day 48-49 -- 阶段二综合项目: 嵌入式通信中间件 (总计 8-10h)

**项目: `emb-comm` -- 零堆分配的嵌入式通信中间件**

```
day48_emb_comm/
├── CMakeLists.txt
├── include/
│   ├── message_bus.h       // 发布/订阅消息总线
│   ├── serializer.h        // 结构体序列化/反序列化
│   ├── message_queue.h     // 固定大小消息队列
│   ├── crc.h               // constexpr CRC
│   └── result.h            // Result<T,E> (复用 Day 34)
├── src/
│   └── message_bus.cpp
├── tests/
│   ├── test_message_bus.cpp
│   ├── test_serializer.cpp
│   └── test_queue.cpp
├── examples/
│   └── sensor_demo.cpp     // 传感器数据发布/订阅演示
├── .github/
│   └── workflows/
│       └── ci.yml          // GitHub Actions CI
└── README.md
```

**核心模块:**
1. `MessageQueue<T, N>`: 基于 `RingBuffer<T, N>` 的线程安全消息队列
2. `MessageBus`: 发布/订阅总线，Topic 用 `uint8_t` ID，回调用 `std::function`
3. `Serializer`: 将结构体序列化为大端字节流（用模板自动处理不同类型）
4. GitHub Actions CI: push 时自动编译 + 运行测试

**CI 配置 `.github/workflows/ci.yml`:**
```yaml
name: CI
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - run: sudo apt-get install -y cmake
      - run: cmake -B build -DCMAKE_BUILD_TYPE=Debug
      - run: cmake --build build
      - run: cd build && ctest --output-on-failure
```

**验收标准**
- [ ] 消息总线能发布/订阅至少 8 种 Topic
- [ ] 序列化/反序列化往返正确（serialize -> deserialize 得到原始数据）
- [ ] 整个库零堆分配（可在嵌入式环境运行）
- [ ] GitHub Actions CI 绿色通过
- [ ] README 包含 API 说明和使用示例
- [ ] **这是你 GitHub 上的第二个正式项目**

---

## 阶段三: 嵌入式编程基础 (第 8-10 周 / Day 50-70)

> 目标: 在真实硬件（STM32）上编程，掌握 GPIO/UART/I2C/SPI 等核心外设

### 硬件准备（在 Day 50 前完成采购）

- **STM32 Nucleo-F446RE** 开发板 (~100 元)
- **BME280** 环境传感器模块（I2C）(~15 元)
- **MPU6050** 六轴传感器模块（SPI/I2C）(~10 元)
- **ESP32 DevKit** (后续用于 WiFi/MQTT)(~30 元)
- 杜邦线、面包板 (~15 元)
- ST-Link 调试器（Nucleo 板自带）
- USB-TTL 串口模块 (~10 元)

### 第 8 周: GPIO 与 UART (Day 50-56)

**Day 50** -- STM32 开发环境搭建
- 安装 STM32CubeIDE 或 VS Code + PlatformIO
- 安装 `arm-none-eabi-gcc` 工具链
- 安装 OpenOCD 调试工具
- **任务:** 创建第一个 STM32 项目，点亮板载 LED (GPIO 输出)

**Day 51** -- GPIO 深入
- GPIO 寄存器: MODER/OTYPER/OSPEEDR/PUPDR/IDR/ODR
- 直接寄存器操作 vs HAL 库
- **任务:** 不使用 HAL 库，纯寄存器操作实现 LED 闪烁 + 按键读取

**Day 52** -- 中断系统
- NVIC、中断优先级、中断向量表
- EXTI 外部中断
- 中断服务程序（ISR）的编写规则: 尽量短、不用 printf、volatile 变量
- **任务:** 用外部中断实现按键触发 LED 切换（去抖动处理）

**Day 53** -- 系统时钟与定时器
- HSE/HSI/PLL 时钟树配置
- SysTick 定时器
- 通用定时器 TIMx: 基本计时、PWM 输出
- **任务:** 用定时器产生 PWM 信号，实现 LED 呼吸灯效果

**Day 54** -- UART 通信
- UART 协议: 起始位、数据位、校验位、停止位、波特率
- STM32 USART 配置
- 中断接收 + 环形缓冲区
- **任务:** 实现 UART 收发，在 PC 端用串口工具发送命令，STM32 回显并执行（重用阶段一的命令解析器思路）

**Day 55** -- DMA 传输
- DMA 概念: 无需 CPU 干预的数据搬运
- UART + DMA 高效接收
- 空闲中断（IDLE）+ DMA 的不定长数据接收方案
- **任务:** 用 DMA + 空闲中断实现 UART 高效不定长数据接收

**Day 56** -- 周末复习 + 小项目
- **任务:** 实现一个完整的 **UART CLI Shell**
  - 命令注册/解析/执行
  - 支持 `help`、`led on/off`、`pwm <duty>`、`status`
  - 使用 DMA + 环形缓冲区接收
  - 历史记录（可选）

### 第 9 周: I2C 与 SPI (Day 57-63)

**Day 57** -- I2C 协议原理
- I2C 总线: SDA/SCL、起始/停止条件、ACK/NACK
- 7 位地址、读/写位
- 时序图阅读方法
- **任务:** 阅读 BME280 数据手册的 I2C 通信章节，画出读取温度的时序图

**Day 58** -- I2C 驱动编写
- STM32 I2C 外设配置
- 读取 BME280 芯片 ID (寄存器 0xD0，应返回 0x60)
- **任务:** 编写 BME280 I2C 驱动 -- 初始化 + 读取芯片 ID + 读取原始温湿度气压数据

**Day 59** -- BME280 完整驱动
- 补偿公式实现（参考数据手册）
- 驱动抽象: `bme280_init/bme280_read_temperature/bme280_read_humidity/bme280_read_pressure`
- **任务:** 完成 BME280 完整驱动，通过 UART 每秒打印温湿度和气压

**Day 60** -- SPI 协议原理
- SPI 总线: MOSI/MISO/SCK/CS、4 种工作模式 (CPOL/CPHA)
- 全双工通信特点
- SPI vs I2C 对比
- **任务:** 阅读 MPU6050 数据手册 SPI 章节（或使用 I2C 模式），理解寄存器映射

**Day 61** -- MPU6050 驱动编写
- 芯片初始化序列
- 读取加速度计和陀螺仪原始数据
- 原始数据到物理单位的转换
- **任务:** 编写 MPU6050 驱动，通过 UART 打印三轴加速度和角速度

**Day 62** -- 驱动架构优化
- HAL 抽象层: 将 I2C/SPI 底层操作抽象为统一接口
- 驱动与平台解耦: 传感器驱动不直接调用 STM32 HAL
- **任务:** 重构 BME280 和 MPU6050 驱动，定义 `typedef struct { int (*read)(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len); ... } I2C_Interface;`，驱动通过接口操作

**Day 63** -- 周末复习 + 小项目
- **任务:** 实现一个**多传感器数据采集系统**
  - 同时读取 BME280 和 MPU6050
  - 通过 UART CLI 可以选择查看哪个传感器数据
  - 支持设置采样率

### 第 10 周: 低功耗与 Flash 操作 (Day 64-70)

**Day 64** -- 看门狗与低功耗模式
- 独立看门狗 (IWDG) 与窗口看门狗 (WWDG)
- STM32 低功耗模式: Sleep / Stop / Standby
- 唤醒源配置
- **任务:** 实现一个低功耗传感器节点 -- 采集一次数据后进入 Stop 模式，定时唤醒

**Day 65** -- 内部 Flash 编程
- STM32 Flash 架构: 扇区/页、擦除/编程规则
- Flash 编程流程: 解锁 -> 擦除 -> 写入 -> 锁定
- **任务:** 实现一个简单的 Flash 存储模块 -- 将配置参数存入 Flash 最后一个扇区，重启后能读回

**Day 66** -- 非易失存储设计
- 磨损均衡基础概念
- 简易 KV 存储实现
- **任务:** 基于 Flash 实现一个简易的键值对存储: `nv_write(key, value, len)` / `nv_read(key, value, len)`

**Day 67** -- 链接脚本与启动过程
- STM32 启动流程: 复位 -> 向量表 -> Reset_Handler -> SystemInit -> main
- 链接脚本 `.ld` 文件详解: MEMORY 段、SECTIONS 段
- `.text/.rodata/.data/.bss` 段在 Flash 和 RAM 中的分布
- **任务:** 修改链接脚本，为 Bootloader 和 App 预留不同的 Flash 空间

**Day 68** -- Bootloader 概念
- Bootloader 的职责: 硬件初始化 -> 固件校验 -> 跳转执行
- 向量表重定位 (VTOR)
- 从 Bootloader 跳转到 App 的方法
- **任务:** 编写一个最简 Bootloader -- 初始化后直接跳转到固定地址的 App

**Day 69** -- UART Bootloader
- 通过 UART 接收固件并写入 Flash
- XMODEM 协议简介
- **任务:** 扩展 Bootloader，支持通过 UART 接收新固件（简化版，用自定义协议）

**Day 70** -- 阶段三总结
- **任务:** 整理代码，将所有驱动和库模块推送到 GitHub
  - 清晰的 README 文档
  - 硬件接线图
  - 编译和烧录说明

---

## 阶段四: FreeRTOS 与网络通信 (第 11-12 周 / Day 71-84)

> 目标: 掌握 RTOS 多任务编程和 MQTT 通信

### 第 11 周: FreeRTOS 核心 (Day 71-77)

**Day 71** -- FreeRTOS 入门
- RTOS vs 裸机 (bare-metal) 超级循环
- FreeRTOS 移植到 STM32 (或使用 STM32CubeMX 生成)
- 任务创建: `xTaskCreate`
- **任务:** 创建两个任务 -- 一个闪烁 LED，一个打印传感器数据，观察并发执行

**Day 72** -- 任务管理
- 任务优先级与调度策略（抢占式）
- 任务状态: Running / Ready / Blocked / Suspended
- `vTaskDelay` vs `vTaskDelayUntil`
- 栈溢出检测
- **任务:** 创建 4 个不同优先级的任务，用 UART 打印观察调度顺序

**Day 73** -- 队列 (Queue)
- `xQueueCreate/xQueueSend/xQueueReceive`
- 队列用于任务间数据传递
- **任务:** 传感器采集任务将数据放入队列，显示任务从队列取出并打印

**Day 74** -- 信号量与互斥量
- 二值信号量: 任务同步（如中断通知任务）
- 计数信号量
- 互斥量: 保护共享资源（如 UART 输出）
- 优先级反转与优先级继承
- **任务:** 用互斥量保护 UART 打印，使多个任务输出不会乱码

**Day 75** -- 软件定时器与事件组
- 软件定时器: `xTimerCreate`
- 事件组: `xEventGroupCreate` / `xEventGroupSetBits` / `xEventGroupWaitBits`
- **任务:** 用事件组实现: 传感器采集完成 -> 设置事件 -> 数据处理任务被唤醒

**Day 76** -- FreeRTOS 最佳实践
- 任务栈大小估算
- 避免在 ISR 中调用阻塞 API（使用 `FromISR` 后缀版本）
- `configASSERT` 调试宏
- 运行时统计: `vTaskGetRunTimeStats`
- **任务:** 为传感器系统添加运行时统计，通过 CLI 命令 `tasks` 查看各任务 CPU 占用

**Day 77** -- 周末复习 + 重构
- **任务:** 将之前的传感器采集系统完全重构为 FreeRTOS 架构:
  - Task 1: 传感器采集 (高优先级)
  - Task 2: 数据处理 (中优先级)
  - Task 3: CLI Shell (低优先级)
  - Task 4: LED 状态指示 (最低优先级)
  - 用队列、信号量、事件组协调

### 第 12 周: ESP32 通信与 MQTT (Day 78-84)

**Day 78** -- STM32 与 ESP32 通信
- UART 连接 STM32 <-> ESP32
- AT 指令协议（ESP32 AT 固件）
- **任务:** 给 ESP32 烧录 AT 固件，STM32 通过 UART 发送 AT 指令连接 WiFi

**Day 79** -- AT 指令引擎
- 实现异步 AT 指令收发引擎
- 超时处理、响应解析
- URC（Unsolicited Result Code）处理
- **任务:** 实现 `AT_Engine` 模块: `at_send_cmd(cmd, timeout)` -> 返回 `AT_OK/AT_ERROR/AT_TIMEOUT`

**Day 80** -- MQTT 协议基础
- MQTT 概念: Broker/Client、Topic、QoS、Retain
- MQTT CONNECT/PUBLISH/SUBSCRIBE 报文格式
- **任务:** 在 PC 上搭建 Mosquitto Broker，用 MQTT Explorer 工具测试发布/订阅

**Day 81** -- ESP32 MQTT 连接
- 通过 AT 指令配置 ESP32 连接 MQTT Broker
- 或: 给 ESP32 写简单的 Arduino 固件作为 MQTT 网关
- **任务:** STM32 采集传感器数据 -> UART 发送到 ESP32 -> ESP32 通过 MQTT 发布到 Broker

**Day 82** -- 双向通信
- 从云端下发控制命令到设备
- 订阅命令 Topic，解析 JSON 指令
- **任务:** 实现远程控制: 云端发送 `{"cmd":"set_interval","value":5}` -> 设备修改采样间隔

**Day 83** -- 系统集成
- 完整数据流: 传感器 -> FreeRTOS 队列 -> 数据处理 -> MQTT 发布
- 断线重连机制
- **任务:** 实现完整的数据上报链路，包括断线检测和自动重连

**Day 84** -- 阶段四总结
- **任务:** 系统稳定性测试 -- 连续运行 24 小时，记录:
  - MQTT 消息成功率
  - 内存使用变化（是否有泄漏）
  - 任务栈使用水位
  - 异常重连次数

---

## 阶段五: 最终项目 -- 安全 OTA Bootloader + IoT 网关 (第 13-16 周 / Day 85-112)

> 目标: 整合所有技能，构建一个生产级的 IoT 设备固件系统

### 第 13 周: OTA Bootloader 设计与实现 (Day 85-91)

**Day 85** -- 系统架构设计
- Flash 分区规划: Bootloader (64KB) | App Bank A (256KB) | App Bank B (256KB) | NV Storage (64KB)
- 镜像元数据结构设计: 版本号、CRC32、SHA256、签名、大小
- 升级流程状态机设计
- **任务:** 画出完整的系统架构图和升级流程状态机图，写入设计文档

**Day 86** -- Bootloader 核心逻辑
- 启动流程: 最小硬件初始化 -> 读取元数据 -> 校验固件完整性 -> 选择 Bank -> 跳转
- CRC32 校验实现
- 向量表重定位
- **任务:** 实现 Bootloader 核心: 能校验 App 并跳转执行

**Day 87** -- 双 Bank 切换
- A/B Bank 选择逻辑: 优先启动高版本、校验失败回退
- 升级标志管理（存储在 NV 区域）
- **任务:** 实现 A/B Bank 切换逻辑，测试: 刷入 Bank A (v1.0) 和 Bank B (v2.0)，验证启动 v2.0

**Day 88** -- 断电恢复机制
- 升级状态持久化: `IDLE -> DOWNLOADING -> COMPLETE -> VERIFIED -> COPY_PENDING -> COPYING -> DONE`
- 任意步骤断电后的恢复策略
- **任务:** 实现断电恢复逻辑，模拟各阶段断电场景测试

**Day 89** -- 固件接收通道
- 通过 MQTT 接收固件分块（通过 ESP32 转发）
- 分块写入 Flash: 逐块 CRC 校验 -> 写入 -> 读回验证
- 进度上报
- **任务:** 实现 OTA 固件接收: 订阅 `device/{id}/ota` Topic，逐块接收并写入备用 Bank

**Day 90** -- 版本管理与回滚
- "试运行"机制: 新固件首次启动后需在超时前确认（`ota_confirm()`），否则回滚
- 版本号比较与降级防护
- **任务:** 实现试运行 + 自动回滚机制

**Day 91** -- 周末测试
- **任务:** 完整 OTA 流程端到端测试:
  1. 设备运行 v1.0
  2. 通过 MQTT 推送 v2.0 固件
  3. 设备接收、校验、重启
  4. Bootloader 加载 v2.0
  5. v2.0 运行确认
  6. 测试回滚: 推送故意损坏的 v3.0 -> 验证自动回滚到 v2.0

### 第 14 周: 安全加固与应用层完善 (Day 92-98)

**Day 92** -- SHA256 完整性校验
- SHA256 算法集成 (使用 mbedTLS 或轻量级实现)
- 固件签名验证流程
- **任务:** 在 Bootloader 中添加 SHA256 校验，替代简单的 CRC32

**Day 93** -- 安全传输
- 固件传输加密（简化版: AES-128-CBC 加密固件）
- 防降级攻击: 记录最低允许版本号
- **任务:** 实现固件加密传输 + 解密写入流程

**Day 94** -- 应用层传感器融合
- 多传感器数据融合策略
- 数据滤波: 移动平均、卡尔曼滤波简化版
- **任务:** 为 BME280 和 MPU6050 数据添加滤波，消除噪声

**Day 95** -- 完整 CLI 系统
- 系统状态查询: `status`、`version`、`tasks`、`memory`
- 传感器命令: `sensor read`、`sensor config`
- OTA 命令: `ota status`、`ota rollback`
- 网络命令: `mqtt status`、`wifi status`
- **任务:** 实现完整的 CLI 命令集

**Day 96** -- 错误处理与日志
- 系统级错误处理框架
- Hard Fault Handler 实现: 打印寄存器信息辅助调试
- 持久化错误日志（写入 NV 存储）
- **任务:** 实现 Hard Fault Handler + 错误日志系统

**Day 97** -- 代码质量
- 静态分析: `cppcheck` 全项目扫描
- 编码规范检查
- 代码覆盖率评估
- **任务:** 修复所有静态分析警告，编写关键模块的单元测试

**Day 98** -- 周末整合测试
- 完整系统联调
- 压力测试: 高频数据上报 + OTA 并行
- 边界条件测试

### 第 15 周: 配套 PC 端工具 (Day 99-105)

**Day 99** -- PC 端固件打包工具
- 用 C++ 编写命令行工具
- 功能: 读取 `.bin` 文件 -> 添加元数据头 (版本/CRC32/SHA256/大小) -> 输出签名固件包
- **任务:** 实现 `fw_packager --input app.bin --version 2.0.0 --output app.pkg`

**Day 100** -- PC 端 OTA 推送工具
- 连接 MQTT Broker
- 将固件包分块发送到设备
- 接收设备进度反馈
- **任务:** 实现 `ota_push --broker mqtt://... --device-id xxx --firmware app.pkg`

**Day 101** -- PC 端串口监控工具
- 串口连接与数据收发
- 数据解析与格式化显示
- **任务:** 实现简单的串口监控工具，能解析设备上报的数据格式

**Day 102-103** -- 工具完善与测试
- 错误处理、超时重试
- 跨平台编译 (Windows/Linux)
- 使用说明文档
- **任务:** 完善所有工具，确保在 Windows 和 Linux 下都能正常工作

**Day 104-105** -- 端到端流程验证
- 从编译固件 -> 打包 -> 推送 -> 设备升级 -> 验证，全流程自动化
- **任务:** 编写一个脚本串联整个流程，一键完成固件构建和 OTA 推送

### 第 16 周: 文档、开源发布与面试准备 (Day 106-112)

**Day 106** -- README 与文档
- 项目 README: 架构图、功能列表、硬件清单、快速开始
- API 文档: Doxygen 生成
- **任务:** 编写高质量 README + 生成 API 文档

**Day 107** -- 架构设计文档
- 系统架构图
- Flash 分区图
- OTA 升级流程图
- 任务调度图
- **任务:** 编写完整的设计文档，包含所有架构图

**Day 108** -- 演示视频
- 录制项目演示视频:
  1. 设备启动 + 传感器数据上报
  2. 云端数据展示
  3. OTA 升级全流程
  4. 故障回滚演示
- **任务:** 录制并上传演示视频

**Day 109** -- GitHub 发布
- 整理仓库结构
- 添加 LICENSE (MIT)
- 创建 Release
- **任务:** 正式发布 v1.0.0

**Day 110** -- 面试准备 -- 技术线
- 准备 C/C++ 高频面试题（指针、内存、虚函数、智能指针等）
- 准备嵌入式高频题（中断、DMA、低功耗、通信协议）
- **任务:** 整理 50 道高频题并写出答案

**Day 111** -- 面试准备 -- 项目线
- 项目深挖准备:
  - **系统架构**: 为什么选择双 Bank 而不是单 Bank + Download 区?
  - **关键瓶颈**: OTA 传输速度优化（分块大小、并行校验）
  - **调试案例**: 一个具体的 bug 定位过程（如 Flash 写入时序问题）
  - **性能指标**: OTA 传输速度 (KB/s)、升级总耗时、最大支持固件大小、MQTT 延迟
- **任务:** 准备项目深挖的 4 条主线，每条能展开讲 5-10 分钟

**Day 112** -- 总结与规划
- 回顾 16 周的学习路径
- 技能自评
- 下一步规划: 嵌入式 Linux? Rust? AI on Edge?
- **任务:** 更新简历，突出项目经验和技术栈

---

## 每日时间建议

- **工作日**: 2-3 小时（1 小时理论学习 + 1-2 小时编码练习）
- **周末**: 4-6 小时（综合项目时间）
- **总投入**: 约 350-450 小时

## 推荐学习资源

**书籍:**
- "C Programming: A Modern Approach" (K.N. King) -- C 语言最佳教材
- "Making Embedded Systems" (Elecia White) -- 嵌入式入门圣经
- "Embedded C Coding Standard" (Michael Barr) -- 编码规范
- "Mastering the FreeRTOS Real Time Kernel" -- FreeRTOS 官方教程 (免费 PDF)

**在线:**
- FreeRTOS 官方文档: https://freertos.org
- STM32 参考手册 (RM0390 for STM32F446)
- ARM Cortex-M4 技术参考手册

**工具:**
- STM32CubeIDE 或 VS Code + PlatformIO
- Git + GitHub
- Mosquitto (MQTT Broker)
- Wireshark (网络抓包)
- Logic Analyzer (可选，淘宝 ~50 元)
