# Quark 工具目录

`tools/` 已经包含 Quark 的配置、构建、产物整理、代码生成、文档发布、
依赖安装和开发辅助工具。这里记录主要入口及调用关系，避免仅凭脚本名称猜测
它在构建流程中的位置。

不要因为某个脚本没有被 `Makefile` 直接引用就立即删除它；GYP action、
qkmake 导出流程或其他脚本也可能调用它。

## 主要构建流程

### 配置与普通构建

```text
./configure <参数>
  -> tools/configure.js
  -> out/config.mk
  -> out/config.gypi

make build
  -> build.mk
  -> GYP 生成平台工程或 Makefile
  -> 编译目标
  -> tools/cp_library.js
```

- `configure`：Shell 入口，准备本地 Node 工具并调用 `tools/configure.js`。
- `configure.js`：检测主机、编译器、SDK 和依赖，生成当前构建配置。
- `common.gypi`：Quark 自身构建所用的公共编译与链接配置。
- `default_target.gypi`：默认 GYP target 配置。
- `touch.js`：GYP 文件生成后修正部分产物的时间戳，避免不必要的重复构建。
- `cp_library.js`：`make build` 完成后的发布复制步骤：
  - Android：复制 `libquark.so` 到 `product/android/jniLibs/<ABI>/`；
  - Linux：复制 `libquark.so` 和 `quark` 到 `product/linux/<arch>/`；
  - macOS：复制 `quark` 到 `product/mac/<arch>/`；
  - iOS：不在这里复制，由 Apple Framework 流程统一处理。

`cp_library.js` 当前直接读取 `out/config.gypi` 计算源目录和目标目录，
不使用 `quark.gyp` 中旧的 `product_dir`、`product_so_subdir` 变量。

### Apple 动态库与 Framework

```text
Apple 平台 make build
  -> quark.gyp 的 mk_quark_dylib action
  -> tools/build_dylib.sh
  -> libquark.dylib 或 libquark.v8.dylib
  -> 当前构建目录中的临时 quark.framework

make mac / make ios 的各架构构建完成后
  -> tools/gen_apple_frameworks.sh
  -> tools/gen_apple_framework.js
  -> out/qkmake/product/<mac|ios>/Frameworks/
```

- `build_dylib.sh`：收集 Quark 各静态 target 的目标文件和依赖库，链接 Apple
  动态库，并生成单架构的临时 Framework。它不负责复制 macOS 命令行可执行文件。
- `gen_apple_frameworks.sh`：选择已经完成的 macOS/iOS 真机和模拟器产物，组织最终
  Framework 目录；需要合并架构时交给下一脚本处理。
- `gen_apple_framework.js`：创建 `Info.plist`、复制公开头文件，并使用 `lipo`
  合并传入的动态库。
- `ios-framework.plist`、`mac-framework.plist`：Framework 的 plist 模板。
- `build_ios_static.sh`：历史/专项 iOS 静态库构建脚本，不属于当前标准
  `make ios` 的主要链路。

### qkmake 产品目录

- `cp_qkmake.js`：构建并整理 qkmake 发布目录，复制 TypeScript 类型、公开头文件、
  示例、`product.gypi` 和 qkmake 自身文件。
- `product.gypi`：使用已发布 Quark 产品开发外部工程时采用的 GYP 配置。
- `cp_header.js`：按项目规则复制公开头文件。
- `gen_releases_lib.sh`：已脱离当前构建流程的旧发布打包脚本。没有入口调用，
  其中的 Linux 产物目录和默认远程地址也已经过期；保留仅供历史参考。

## 代码与资源生成

- `gen_font_natives.js`：把内置字体转换为 C/C++ 原生资源。
- `gen_js_natives.js`：把 JavaScript 资源转换为 C/C++ 原生资源。
- `gen_glsl_natives.js`：生成各渲染后端使用的内嵌 Shader 数据。
- `gen_glsl_blur.js`：生成模糊 Shader 的派生代码。
- `compress_json.py`：压缩 JSON 数据。
- `read_version.js`：读取并同步 Quark 版本信息。

生成类脚本通常由 GYP action 调用。修改生成结果前，应先确认对应源文件和生成器，
不要只修改输出文件。

## 文档工具

- `gen_readme.js`：读取源码/API 信息并参与 README/API 文档生成。
- `gen_html_doc.js`：把根 README、`docs/guides/` 和 API reference 生成最终 HTML 文档。
- `doc_template.html`：HTML 文档模板。
- `doxygen.h`：API 文档分析辅助声明。

公开文档的移动或链接修改，需要同时验证 Markdown 源文件和生成后的 HTML 路径。

## 依赖与工具链

- `configure.js`：除生成配置外，也会检测所需命令；缺失的可选源码依赖会从
  `tools/deps/` 选择性初始化和构建。
- `build_ffmpeg.sh`：FFmpeg 专项构建。
- `install-android-toolchain`：旧 Android standalone toolchain 生成脚本，目前没有
  构建入口调用。它依赖旧 NDK 的 `make-standalone-toolchain.sh` 和 GCC 4.9
  toolchain 名称，已不适用于当前直接使用 NDK LLVM toolchain 的构建流程。
- `android_AR_host.mac`、`android_LINK_host.mac`：macOS 主机交叉编译 Android 时的
  归档和链接包装器。
- `gas-preprocessor.pl`：汇编源码兼容处理。
- `v_all.ver`、`v_small.ver`：ELF 动态库导出符号版本脚本。

`deps/`、`tools/deps/`、`tools/ndk/`、`tools/linux/` 和 `tools/pkgs/` 体积较大，
日常排查工具调用关系时不要默认遍历这些目录。

## 开发辅助

- `sync_watch`：`make watch` 使用的源码监听与远程同步工具。
- `remote_build.sh`：本地主控脚本，生成远端执行内容，通过 SSH 启动构建，
  再使用 SCP 取回 `out/remote_build.tgz` 并解压。
- `remote_build1.sh`：实际在远程主机执行配置和 `make`，然后按调用方指定的
  product 子目录打包产物。当前远程编译和产物回传由这两个脚本配合完成。
- `server.js`：`make web` 使用的本地调试服务器。
- `gen_ide_config.js`：生成 clangd/IDE 所需的本地配置。
- `check.js`：项目专项检查脚本。
- `python`：构建流程使用的 Python 入口包装器。
- `empty.c`：需要一个空源码来承载聚合链接 target 时使用。

## 常用入口速查

| 目的 | 入口 | 后续主要工具 |
|---|---|---|
| 配置当前平台 | `./configure ...` | `configure.js` |
| 编译当前配置 | `make build` | `build.mk`、`touch.js`、`cp_library.js` |
| 构建 macOS 产品 | `make mac` | `build_dylib.sh`、`gen_apple_frameworks.sh` |
| 构建 iOS 产品 | `make ios` | `build_dylib.sh`、`gen_apple_frameworks.sh` |
| 构建 Android 产品 | `make android` | `build.mk`、`cp_library.js` |
| 远程尝试构建 | `make try_android` / `make try_linux` | `remote_build.sh` |
| 同步开发目录 | `make watch` | `sync_watch` |
| 生成 HTML 文档 | `make doc` | `gen_html_doc.js` |
| 整理 qkmake | `make install` | `cp_qkmake.js` |

## Linux 库与头文件诊断命令

下面是原有的查库、查头文件和内存诊断记录：

```sh
ld --verbose | grep SEARCH_DIR | tr -s ' ;' \\012
ldconfig -p| grep fontconfig
ldd test1
nm -D /usr/lib/aarch64-linux-gnu/libc.so.6 | grep 'A GLIBC_'

echo "#include <stdio.h>" | gcc -fsyntax-only -v -xc -H -
echo "#include <fontconfig/fontconfig.h>" | gcc -fsyntax-only -v -xc -H -
syntax-gcc '#include <fontconfig/fontconfig.h>'

valgrind --tool=memcheck --leak-check=full --show-reachable=yes --trace-children=yes ./leak

sudo dpkg --add-architecture arm64
sudo dpkg --add-architecture armhf
```

- Ubuntu 18.04 使用 GLIBC 2.27
- Ubuntu 20.04 使用 GLIBC 2.31
- Ubuntu 22.04 使用 GLIBC 2.35
- Ubuntu 24.04 使用 GLIBC 2.39

## 字体与图标工具

* https://fontello.com/
