# Quark

[English](README.md) · [使用教程](docs/guides/README-cn.md) · [API 文档](https://quarks.cc/doc/)

Quark 是一个支持 Android、iOS、macOS 和 Linux 的跨平台 GUI 框架，核心由
C++、自研 GPU 渲染器、轻量级布局引擎和嵌入式 JavaScript/JSX 运行时组成。

Quark 不是浏览器运行时。它拥有独立的 View 树、样式与布局系统、事件模型、
渲染管线和平台集成，API 强调显式结构、可预测行为和可控性能。

## 核心能力

- 在 Android、iOS、macOS 和 Linux 上进行原生 GUI 渲染。
- 使用 C++ 实现性能敏感系统，使用 JavaScript/JSX 编写应用逻辑和声明 UI。
- 使用显式布局引擎，不依赖浏览器 DOM 或浏览器 CSS。
- 支持连续 class、后代和直接子级选择器，以及 `:normal`、`:hover`、
  `:active` 状态的 CSS-like 样式子集。
- 在统一 View 与事件模型中提供图片、文本、渐变、裁剪、滤镜、动画、媒体、
  滚动、输入以及 World/Entity View。

## 平台与渲染器状态

| 目标平台 | 宿主/工具链 | 可用渲染器 | 当前状态 |
|---|---|---|---|
| iOS | macOS + Xcode | OpenGL ES 或 Metal | Metal 与 GL 路径均已实现，并用于当前测试场景。 |
| macOS | macOS + Xcode | OpenGL 或 Metal | 两个后端均已实现。 |
| Android | macOS/Linux + JDK、Android SDK/NDK | OpenGL ES；可选 Vulkan | Vulkan 呈现和 Canvas/CAPA 编码已实现，仍在扩大设备验证。 |
| Linux | Linux 原生工具链 | OpenGL ES；可选 Vulkan/Xlib | Vulkan 呈现已实现，仍需继续 Linux 运行时验证。 |

当前 Linux 构建与运行测试环境为 Ubuntu 20.04，其他 Linux 发行版尚未完成验证。

源码配置当前默认启用 GL。Apple 构建在关闭 GL 时选择 Metal；Android/Linux
可以在保留 GL 的同时启用 Vulkan。当 Vulkan 与 GL 同时存在时会优先尝试
Vulkan，使用 `--gl` 可强制选择 GL。Android Vulkan 要求 Android 10/API 29
和 Vulkan 1.1 或更高版本；不支持的设备会回退到 GL。

## 渲染系统

Quark 拥有自己的 GPU 渲染栈，不使用浏览器 Canvas、CSS 绘制或 Web 合成。

```txt
Canvas API
  -> GPUCanvas 共享状态、裁剪、path/image/text 分发
  -> OpenGL command pack、Metal command encoder 或 Vulkan command buffer
  -> 平台 surface 或离屏 image
```

渲染器组合使用两种互补的反走样策略：

- **AASide** 是快速几何边沿带路径，用于文字、hairline、简单边缘以及
  GL/GLES 兼容路径。
- **CAPA**（Coverage Area Pipeline Anti-Aliasing）是 Metal/Vulkan-class
  后端处理复杂有序填充的 compute 路径。它把边分配到 tile，计算面积
  coverage，并按顺序合成 layer，避免相邻图元独立反走样时产生背景漏光。

Metal 是已经稳定使用的 CAPA 运行时路径。Vulkan CAPA 命令编码已经实现，
仍在进行更广泛的运行时验证。

运行时可传入 `--gl` 强制使用 GL 渲染器；也可传入 `--aaside`，在
Metal/Vulkan 上禁用 CAPA 并强制使用 AASide 反走样路径。

| ![Screenshot](http://quarks.cc/img/000.jpg) | ![Screenshot](http://quarks.cc/img/001.jpg) | ![Screenshot](http://quarks.cc/img/002.jpg) |
|--|--|--|

## 使用 qkmake 快速开始

当前支持的宿主工作流是 macOS 和 Linux；Apple 目标需要 macOS/Xcode。
Windows 暂不支持。

安装 Node.js 和 Python，然后安装已发布的工具包：

```sh
sudo npm install -g qkmake
```

创建项目：

```sh
mkdir myproj
cd myproj
qkmake init
```

### 最小应用

```tsx
import { Jsx, Application, Window } from 'quark'

new Application()
new Window().render(
	<text value="Hello world" fontSize={48} align="centerMiddle" />
)
```

更多完整应用可查看[示例代码]。

### 构建应用代码与资源

`qkmake build` 会安装项目依赖、转换 TypeScript/JavaScript 并打包应用资源：

```sh
qkmake build
```

新初始化的项目也可以先导出；导出流程会在需要时构建缺失的应用产物。

### 导出或打开平台工程

```sh
qkmake export ios
qkmake export mac
qkmake export android
qkmake export linux
```

生成的工程位于 `project/<platform>`。Apple 工程使用 Xcode，Android 导出
使用 Android Studio/CMake，Linux 导出使用原生 Make 工程。执行
`qkmake open <platform>` 可以在需要时先生成工程，再使用宿主机可用工具打开。

### 运行与调试

运行应用：

```sh
qkmake start .
qkmake start . --gl
qkmake start . --aaside
```

`--gl` 用于强制选择 GL 渲染器。`--aaside` 不改变已经选择的 Metal/Vulkan
渲染器，但会禁用 CAPA，强制使用 AASide 反走样路径。这两个参数适合用于
兼容性检查和渲染效果对比。

启动开发服务器和文件监听：

```sh
qkmake watch
```

watcher 默认在 `1026` 端口提供应用服务，在 TS/TSX 文件变化时通知已连接设备，
使调试构建可以更新界面而无需完整重启应用。

## 从源码构建 Quark

源码构建需要 Node.js、Python 和目标平台对应的工具链：

- iOS/macOS 需要 Xcode；
- Android 需要 JDK、Android SDK 和 Android NDK；
- Linux 需要本机编译器和开发包。

构建 Android 前，需要设置以下三个环境变量；`configure` 会读取它们来定位
Android SDK、NDK 和 Java 工具链：

```sh
export ANDROID_SDK=$HOME/Install/android-sdk
export ANDROID_NDK=$ANDROID_SDK/ndk/29.0.13113456
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
```

路径按实际安装位置调整；macOS 的 `JAVA_HOME` 通常指向 JDK 的
`Contents/Home` 目录。

在已有 checkout 中同步当前分支和全部 submodule：

```sh
make sync
```

`make sync` 会同时执行 `git pull` 和
`git submodule update --init --recursive`，因此只应在确实准备更新当前 checkout
时使用。

根 Makefile 提供 `make ios`、`make mac`、`make android` 和 `make linux`。
`make all` 会构建完整平台产物，并可能通过配置的 `REMOTE_COMPILE_HOST` 构建
本机无法构建的目标。`make install` 会构建这些产物并安装本地 qkmake；
`make install-only` 则安装已有的 `out/qkmake` 产物。

## 文档

- [使用教程](docs/guides/README-cn.md)：面向任务的中英文教程，中文文件统一
  使用 `-cn.md` 后缀。
- [自动生成的 API reference](https://quarks.cc/doc/)：从 `libs/quark` 生成的
  模块、类、接口、属性、方法与事件文档。
- [视图布局与样式](docs/guides/VIEW_STYLE-cn.md)：View 继承、JSX 布局、class
  选择器、伪状态和样式过渡。
- [Action 与关键帧](docs/guides/ACTIONS-cn.md)：动画组合、关键帧、播放和事件。

`make doc` 会运行 `tools/gen_html_doc.js`，从 TypeScript 源码生成 API Markdown，
校验中英文教程是否成对，并加入两份根 README、`docs/guides/`、教程资源文件和
License，最终把完整的可发布 HTML 树输出到 `out/doc/html`。

## 项目状态与参与贡献

GL 与 Metal 路径已经建立，Vulkan 正处于跨 Android/Linux 驱动的运行时验证
和稳定化阶段。欢迎在 [GitHub 仓库]提交范围明确的 issue 和 pull request。
后端改动应保持 Canvas 行为对齐，并附带最小且相关的验证。

纯文档改动至少应通过 `git diff --check`；影响发布文档的改动还应运行
`make doc`。

## License

Quark 使用 BSD License 分发，详见 [LICENSE](LICENSE)。

[示例代码]: https://github.com/louis-tru/quark/tree/master/examples
[GitHub 仓库]: https://github.com/louis-tru/quark

<script>
	<!--
	var language = (navigator.browserLanguage || navigator.language).toLowerCase();
	var isLanguageCn = language.indexOf('cn') >= 0;
	var isPageCn = location.href.indexOf('README-cn') >=0;
	var isHtml = typeof src == 'string'; // html page will have a src variable

	if ( isLanguageCn ) { // cn
		if ( !isPageCn ) { // goto to cn
			location.href = isHtml ? 'README-cn.html' : 'README-cn.md';
		}
	} else { // en
		if ( isPageCn ) { // goto to en
			location.href = isHtml ? 'README.html' : 'README.md';
		}
	}
	-->
</script>
