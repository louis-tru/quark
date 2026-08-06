# `thread_local` 汇编实验记录

本文记录 Qk 中 `thread_local` 访问方式的实际汇编结果，供后续优化
`thread_self_id()`、`thread_self()` 和 `Allocator::_current` 时比较。

所有结论以实际 Release 汇编为准，不只根据 C++ 语言层面的初始化规则推断。

## 测试环境

- 日期：2026-08-06
- 平台：macOS，arm64
- 编译器：Apple Clang
- 构建类型：Release
- 观察目标：
  `out/mac.arm64.Release/obj.target/quark-util/src/util/thread/thread.o`

本次 `thread.o` 已成功生成。最终 `libquark.dylib` 链接被工作区中原有的
Metal/CAPA 缺失符号阻挡，因此本文记录的是编译器生成的优化目标代码，尚未包含
最终链接器可能执行的地址松弛。该链接失败与本次 TLS 实验无关。

## 指令统计口径

本文同时记录两种数量：

- “核心指令数”只统计文中展示的 TLS、判断和取值逻辑；
- “完整路径指令数”从 `thread_self_id()` 入口统计到 `ret`，包含函数序言、
  函数尾声和分支。

需要特别注意：

```asm
blr x8
```

在调用点只算一条指令，但它会进入 Apple TLV thunk 并执行更多代码。因此，除了
核心与完整路径指令数，还必须单独统计 TLV thunk 和 `pthread_self()` 的调用次数。

## 基线：函数内动态初始化

当前代码已经恢复为最早的实现：

```cpp
ThreadID thread_self_id() {
  thread_local ThreadID tid = std::this_thread::get_id();
  return tid;
}
```

Apple Clang 为 `tid` 生成了单独的 TLS 初始化 guard。`nm` 中可以看到：

```text
qk::thread_self_id()::tid
guard variable for qk::thread_self_id()::tid
```

稳定路径的核心汇编为：

```asm
# 取得 guard 的 TLV 描述符并解析当前线程中的 guard 地址
adrp  x20, guard@TLVPPAGE
ldr   x20, [x20, guard@TLVPPAGEOFF]
ldr   x21, [x20]
mov   x0, x20
blr   x21
ldrb  w8, [x0]

# 准备 tid 的 TLV 描述符
adrp  x19, tid@TLVPPAGE
ldr   x19, [x19, tid@TLVPPAGEOFF]
tbz   w8, #0, initialize

# 解析当前线程中的 tid 地址并读取缓存
ldr   x8, [x19]
mov   x0, x19
blr   x8
ldr   x0, [x0]
```

包含函数序言和尾声的完整稳定路径为 21 条：

```asm
stp   x22, x21, [sp, #-0x30]!
stp   x20, x19, [sp, #0x10]
stp   x29, x30, [sp, #0x20]
add   x29, sp, #0x20
adrp  x20, guard@TLVPPAGE
ldr   x20, [x20, guard@TLVPPAGEOFF]
ldr   x21, [x20]
mov   x0, x20
blr   x21
ldrb  w8, [x0]
adrp  x19, tid@TLVPPAGE
ldr   x19, [x19, tid@TLVPPAGEOFF]
tbz   w8, #0, initialize
ldr   x8, [x19]
mov   x0, x19
blr   x8
ldr   x0, [x0]
ldp   x29, x30, [sp, #0x20]
ldp   x20, x19, [sp, #0x10]
ldp   x22, x21, [sp], #0x30
ret
```

包括函数序言和尾声后，统计结果为：

| 路径 | 核心指令数 | 完整路径指令数 | TLV thunk | `pthread_self()` |
|---|---:|---:|---:|---:|
| 已初始化稳定路径 | 13 | 21 | 2 | 0 |
| 当前线程首次调用 | 约 24 | 约 32 | 4 | 1 |

首次调用需要：

1. 解析并读取 guard；
2. 调用 `pthread_self()`；
3. 解析 `tid` 地址并写入线程 ID；
4. 再次解析 guard 地址并设置初始化标记；
5. 跳回稳定路径，再次解析 `tid` 地址并返回。

## 实验一：默认构造后手动判断空 ID

实验代码：

```cpp
ThreadID thread_self_id() {
  static thread_local ThreadID tid;
  if (Qk_UNLIKELY(tid == ThreadID())) {
    tid = std::this_thread::get_id();
  }
  return tid;
}
```

实验目的，是用空 `ThreadID` 作为初始化标记，尝试替代编译器 guard。

结果没有达到目的。Apple libc++ 的 `std::thread::id` 默认构造函数在本次构建中
仍被 Clang 作为动态初始化处理，因此编译器 guard 没有消失。在它之外，代码又
增加了一次空 ID 判断。

稳定路径仍然包含：

- 约 15 条核心指令、21 条完整路径指令；
- 两次 TLV thunk；
- 额外的空 ID 比较。

包含函数序言和尾声的完整稳定路径为 21 条：

```asm
stp   x20, x19, [sp, #-0x20]!
stp   x29, x30, [sp, #0x10]
add   x29, sp, #0x10
adrp  x9, guard@TLVPPAGE
ldr   x9, [x9, guard@TLVPPAGEOFF]
ldr   x10, [x9]
mov   x0, x9
blr   x10
ldrb  w11, [x0]
adrp  x8, tid@TLVPPAGE
ldr   x8, [x8, tid@TLVPPAGEOFF]
tbz   w11, #0, initialize_default_id
ldr   x9, [x8]
mov   x0, x8
blr   x9
mov   x19, x0
ldr   x0, [x0]
cbz   x0, initialize_current_id
ldp   x29, x30, [sp, #0x10]
ldp   x20, x19, [sp], #0x20
ret
```

该实验已撤销。

## 实验二：POD TLS 缓存与 placement new

为了从语言层面保证 TLS 变量只需要零初始化，实验使用纯 POD 存储保存
`ThreadID`：

```cpp
ThreadID thread_self_id() {
  struct ThreadIDCache {
    alignas(ThreadID) unsigned char storage[sizeof(ThreadID)];
    bool initialized;
  };

  static_assert(std::is_trivially_destructible<ThreadID>::value,
    "ThreadID cache requires trivial destruction");

  static thread_local ThreadIDCache cache = {};
  auto tid = reinterpret_cast<ThreadID*>(cache.storage);

  if (Qk_UNLIKELY(!cache.initialized)) {
    new (tid) ThreadID(std::this_thread::get_id());
    cache.initialized = true;
  }
  return *tid;
}
```

这一版本成功消除了编译器生成的 guard。`nm` 中只有 `cache`，不再出现对应的
guard variable。

稳定路径的核心汇编为：

```asm
# 解析当前线程中的 cache 地址
adrp  x0, cache@TLVPPAGE
ldr   x0, [x0, cache@TLVPPAGEOFF]
ldr   x8, [x0]
blr   x8
mov   x19, x0

# 检查并读取缓存
ldrb  w8, [x0, #8]
cmp   w8, #1
b.ne  initialize
ldr   x0, [x19]
```

包含函数序言和尾声的完整稳定路径为 15 条：

```asm
stp   x20, x19, [sp, #-0x20]!
stp   x29, x30, [sp, #0x10]
add   x29, sp, #0x10
adrp  x0, cache@TLVPPAGE
ldr   x0, [x0, cache@TLVPPAGEOFF]
ldr   x8, [x0]
blr   x8
mov   x19, x0
ldrb  w8, [x0, #8]
cmp   w8, #1
b.ne  initialize
ldr   x0, [x19]
ldp   x29, x30, [sp, #0x10]
ldp   x20, x19, [sp], #0x20
ret
```

统计结果为：

| 路径 | 核心指令数 | 完整路径指令数 | TLV thunk | `pthread_self()` |
|---|---:|---:|---:|---:|
| 已初始化稳定路径 | 9 | 15 | 1 | 0 |
| 当前线程首次调用 | 约 13 | 约 19 | 1 | 1 |

相对基线，稳定路径的完整路径减少了 6 条指令和一次 TLV thunk。但对于“取得
当前线程 ID”这一简单操作，9 条核心指令、15 条完整路径指令加一次 TLV thunk
仍然偏多，而且原始存储、placement new 和手动生命周期增加了维护复杂度。

该实验已撤销，`thread_self_id()` 已恢复为基线实现。

## 实验三：命名空间级内部链接 TLS

实验将 `tid` 移到函数外，并使用 `static` 保持编译单元内部链接：

```cpp
static thread_local ThreadID tid = std::this_thread::get_id();

ThreadID thread_self_id() {
  return tid;
}
```

Apple Clang 仍然需要保证每个线程已经执行 `get_id()`，所以生成了当前编译单元的
TLS guard 和 `___cxx_global_var_init` 初始化函数。稳定路径核心为：

```asm
# 解析并检查编译单元的 TLS guard
adrp  x8, tls_guard@TLVPPAGE
ldr   x8, [x8, tls_guard@TLVPPAGEOFF]
ldr   x9, [x8]
mov   x0, x8
blr   x9
ldrb  w10, [x0]
cbz   w10, initialize

# 解析并读取命名空间级 tid
adrp  x0, tid@TLVPPAGE
ldr   x0, [x0, tid@TLVPPAGEOFF]
ldr   x8, [x0]
blr   x8
ldr   x0, [x0]
```

包含函数序言和尾声的完整稳定路径为 16 条：

```asm
stp   x29, x30, [sp, #-0x10]!
mov   x29, sp
adrp  x8, tls_guard@TLVPPAGE
ldr   x8, [x8, tls_guard@TLVPPAGEOFF]
ldr   x9, [x8]
mov   x0, x8
blr   x9
ldrb  w10, [x0]
cbz   w10, initialize
adrp  x0, tid@TLVPPAGE
ldr   x0, [x0, tid@TLVPPAGEOFF]
ldr   x8, [x0]
blr   x8
ldr   x0, [x0]
ldp   x29, x30, [sp], #0x10
ret
```

初始化函数的核心为：

```asm
bl    _pthread_self
mov   x8, x0
adrp  x0, tid@TLVPPAGE
ldr   x0, [x0, tid@TLVPPAGEOFF]
ldr   x9, [x0]
blr   x9
str   x8, [x0]
```

统计结果为：

| 路径 | 核心指令数 | 完整路径指令数 | TLV thunk | `pthread_self()` |
|---|---:|---:|---:|---:|
| 已初始化稳定路径 | 12 | 16 | 2 | 0 |
| 当前线程首次调用，含初始化函数 | 约 25 | 约 33 | 4 | 1 |

与函数内基线相比，命名空间级内部链接让稳定路径从 13 条核心指令降到 12 条，
完整路径从 21 条减少到 16 条，但没有消除 guard TLV 和数据 TLV 的两次解析。
首次路径也没有改善。该实验完成后已撤销，源码和 `thread.o` 均恢复为基线版本。

## Apple 平台的 `std::this_thread::get_id()`

当前 Xcode 使用 macOS 26.2 SDK。libc++ 头文件中的调用链为：

```cpp
inline __thread_id get_id() noexcept {
  return __libcpp_thread_get_current_id();
}

inline __libcpp_thread_id __libcpp_thread_get_current_id() {
  const __libcpp_thread_t current = pthread_self();
  return __libcpp_thread_get_id(&current);
}
```

Darwin 的 `pthread_t` 本身就是 libc++ 使用的线程 ID，优化后中间包装全部消失。

为了验证 Qk 调用端，曾临时将函数改为：

```cpp
ThreadID thread_self_id() {
  return std::this_thread::get_id();
}
```

macOS arm64 Release `thread.o` 的正常路径为：

```asm
stp  x29, x30, [sp, #-0x10]!
mov  x29, sp
bl   _pthread_self
ldp  x29, x30, [sp], #0x10
ret
```

即 Qk 包装函数自身为 1 条核心指令、5 条完整路径指令和一次 `_pthread_self`
调用。目标文件中还有
一个 `___clang_call_terminate` 异常落点，但正常路径不会执行它。这个直接调用
实验完成后也已撤销，源码和 `thread.o` 均恢复为基线版本。

当前系统 `/usr/lib/system/libsystem_pthread.dylib` 中，arm64
`_pthread_self` 的快速路径为：

```asm
_pthread_self:
  mrs   x0, TPIDRRO_EL0
  ldr   x8, [x0, #-0xe0]!
  adrp  x9, 34
  ldr   x9, [x9, #0x60]
  eor   x8, x9, x8
  cmp   x0, x8
  b.ne  cold_path
  ret
```

正常快速路径共 8 条系统库指令。它读取线程寄存器，并用系统保存的值验证得到的
pthread 指针；只有验证失败才进入冷路径。

不计最终 Framework 可能添加的符号桩，直接版本正常路径目前可见为：

| 部分 | 核心指令数 | 完整路径指令数 | 额外调用 |
|---|---:|---:|---:|
| Qk `thread_self_id()` | 1 | 5 | `_pthread_self` 一次 |
| 系统 `_pthread_self` 快速路径 | 8 | 8 | 0 |
| 合计 | 9 | 13 | 0 个 TLS thunk |

因此，Apple 平台直接调用版本明显短于 21 条加两次 TLV thunk 的基线缓存版本，
也短于 15 条加一次 TLV thunk 的 POD 缓存版本。最终是否采用仍应在可成功链接的
Framework 上复查符号桩，并进行必要的微基准测试。

## Linux x86_64 的 `pthread_self()`

当前实测环境为：

- Ubuntu 20.04；
- Linux 5.15.0-139-generic；
- x86_64；
- glibc 2.31-0ubuntu9.18。

`pthread_self@@GLIBC_2.2.5` 位于：

```text
/lib/x86_64-linux-gnu/libc.so.6
```

符号大小为 14 字节，完整执行路径只有 3 条指令：

```asm
pthread_self:
  endbr64
  mov  %fs:0x10, %rax
  retq
```

逐条作用如下：

1. `endbr64` 是 x86 CET 的间接分支目标保护指令；
2. `mov %fs:0x10, %rax` 从当前线程控制块的固定偏移读取 `pthread_t`；
3. `retq` 返回调用者。

反汇编中紧随其后的：

```asm
xchg %ax, %ax
```

位于 `pthread_self` 的 14 字节符号范围之外，是下一个函数前的对齐填充，不属于
`pthread_self()`，正常调用不会执行它。

按本文统计口径：

| 平台系统函数 | 核心取值指令 | 完整函数指令 | TLS thunk |
|---|---:|---:|---:|
| macOS arm64 `_pthread_self` | 7 条读取/校验与分支指令 | 8 | 0 |
| Ubuntu 20 x86_64 `pthread_self` | 1 条 `mov` | 3 | 0 |

两种架构的单条指令能力和安全机制不同，不能只用条数判断耗时。但 Linux x86_64
版本明确没有 TLS 地址解析函数、锁或系统调用，线程 ID 的实际获取就是一次基于
`%fs` 的内存读取。

Ubuntu 20 使用的 GCC 9 libstdc++ 中，`std::this_thread::get_id()` 还会先通过
`__gthread_active_p()` 判断 pthread 是否可用。它检查的是弱符号
`__pthread_key_create` 的地址是否为空，不是读取普通的运行时布尔变量。

使用 GCC 9 以 `-O3 -fPIC -shared -pthread` 编译等价的直接调用函数：

```cpp
ThreadID thread_self_id_test() {
  return std::this_thread::get_id();
}
```

最终 `.so` 中正常 pthread 路径为 7 条函数内指令：

```asm
thread_self_id_test:
  endbr64
  cmpq  $0, __pthread_key_create(%rip)
  je    no_pthread
  sub   $8, %rsp
  call  pthread_self@plt
  add   $8, %rsp
  retq
```

其中 `sub/add %rsp` 用于在嵌套调用前后维持 x86_64 ABI 要求的栈对齐。稳定状态下
`pthread_self@plt` 还执行两条跳板指令：

```asm
endbr64
bnd jmpq *pthread_self@GOTPCREL(%rip)
```

PLT 中紧随其后的 `nop` 是对齐填充，跳转成功后不会执行。再加上 glibc
`pthread_self()` 本身的 3 条指令，完整稳定路径为：

| 部分 | 完整路径指令数 |
|---|---:|
| libstdc++ 内联后的 Qk 等价包装函数 | 7 |
| `pthread_self@plt` 稳定跳板 | 2 |
| glibc `pthread_self()` | 3 |
| 合计 | 12 |

如果只计算 `cmp/je/call` 三条核心包装逻辑和 `pthread_self()` 的三条函数指令，
可以得到 6 条；但这不是 CPU 实际经过的完整动态库路径。首次动态符号解析还可能
进入链接器的慢路径，稳定后才是上表的两条 PLT 跳板。

### Linux 直接调用 `pthread_self()` 的实验

当前 Ubuntu GCC 9 的 libstdc++ 允许使用其公开的原生句柄构造函数：

```cpp
ThreadID thread_self_id_from_pthread() {
  return ThreadID(pthread_self());
}
```

最终 `.so` 中的包装函数为 5 条：

```asm
thread_self_id_from_pthread:
  endbr64
  sub   $8, %rsp
  call  pthread_self@plt
  add   $8, %rsp
  retq
```

它确实消除了 `__gthread_active_p()` 的 `cmp/je` 两条指令。加上两条 PLT 稳定
跳板和三条 glibc 指令，完整稳定路径从 `get_id()` 版本的 12 条降到 10 条。

如果函数直接返回原生 `pthread_t`：

```cpp
pthread_t thread_self_native() {
  return pthread_self();
}
```

GCC 9 可以生成尾调用：

```asm
thread_self_native:
  endbr64
  jmp pthread_self@plt
```

此时 Qk 等价包装为 2 条，连同 PLT 和 glibc 后的稳定路径为 7 条。

| Ubuntu 20 x86_64 版本 | 包装函数 | PLT | glibc | 完整稳定路径 |
|---|---:|---:|---:|---:|
| `std::this_thread::get_id()` | 7 | 2 | 3 | 12 |
| `ThreadID(pthread_self())` | 5 | 2 | 3 | 10 |
| 原生 `pthread_t` 尾调用 | 2 | 2 | 3 | 7 |

但是 `ThreadID(pthread_self())` 依赖当前 libstdc++ 暴露的非标准原生句柄构造方式；
Apple libc++ 的对应构造函数是私有的，不能作为 Qk 的统一跨平台实现。把 Qk 的
`ThreadID` 整体改成 `pthread_t` 还需要重新处理不同 POSIX 实现的线程 ID 比较、
哈希和公开 API，不应只根据这里节省的两条指令直接修改。

### GCC 9 关闭弱 pthread 检查的实验

GCC 9 的内部 gthread 头文件支持：

```text
-D_GLIBCXX_GTHREAD_USE_WEAK=0
```

设置后，`__gthread_active_p()` 在普通 Linux 目标上直接内联为：

```cpp
return 1;
```

使用与前文相同的 `.so` 测试并加入该宏后，`cmp/je` 被完全消除：

```asm
thread_self_id_test:
  endbr64
  sub   $8, %rsp
  call  pthread_self@plt
  add   $8, %rsp
  retq
```

结果与 Linux 上直接写 `ThreadID(pthread_self())` 相同：包装函数 5 条，连同两条
PLT 稳定跳板和三条 glibc 指令后，完整稳定路径为 10 条。

该开关只适合确认永远启用 pthread 的 libstdc++ 构建，并有以下约束：

- 编译和链接都必须始终使用 `-pthread`；
- 它会影响包含 gthread 头文件的所有 C++ 编译单元，不只影响 `get_id()`；
- 为避免同一个内联函数在不同编译单元出现不同定义，不能只给 `thread.cc` 设置；
- `_GLIBCXX_GTHREAD_USE_WEAK` 是 libstdc++ 内部宏，不是标准 C++ 接口，升级 GCC
  或更换标准库时需要重新验证；
- Apple libc++、Android libc++ 等非 libstdc++ 构建不应使用它。

本次只验证了宏和汇编效果，尚未将它加入 Qk 的 Linux 构建参数。

## 当前结论

1. 函数内 `thread_local ThreadID tid = get_id()` 在 Apple 动态产物中需要
   guard TLV 和数据 TLV 两次解析。
2. 单纯改成默认构造并手动判断空 ID，不能消除 Apple Clang 生成的 guard。
3. POD 原始存储可以将稳定路径降为一次 TLV 解析，但实现复杂度较高，仍有
   9 条核心指令、15 条完整路径指令。
4. 命名空间级内部链接 TLS 的稳定路径为 12 条核心指令、16 条完整路径指令，
   仍需要两次 TLV thunk，首次初始化路径没有改善。
5. Apple 平台的直接 `get_id()` 版本在 Qk 调用端为 1 条核心指令、5 条完整路径
   指令，系统 `_pthread_self` 快速路径为 8 条；不计最终符号桩时合计为 9 条核心
   指令、13 条完整路径指令，且没有 TLV thunk。
6. Linux ELF、Android ELF、macOS Mach-O 和 iOS Mach-O 必须分别检查，不能用
   一个平台的 TLS 汇编推断其他平台。
7. Ubuntu 20 x86_64 glibc 2.31 的 `pthread_self()` 完整路径只有 3 条指令，核心
   取值是一次 `%fs:0x10` 内存读取；GCC 9 直接 `get_id()` 的等价 `.so` 稳定路径
   加上 pthread 可用性检查、栈对齐和 PLT 跳板后共实测 12 条指令。
8. Linux GCC 9 使用 `ThreadID(pthread_self())` 可将完整稳定路径降到 10 条；直接
   返回原生 `pthread_t` 可尾调用并降到 7 条，但两者都不适合作为未经审计的统一
   跨平台替换。
9. Linux libstdc++ 构建使用 `_GLIBCXX_GTHREAD_USE_WEAK=0` 也可将 `get_id()` 路径
   降到 10 条，但这是影响全工程的内部宏，目前只完成实验，未加入构建配置。
