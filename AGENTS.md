# Quark 项目 AI 上下文

## 先读这些

AI 助手进入项目后，先读：

1. `docs/AI_CONTEXT.md`：长期项目记忆和工作规则
2. `docs/CURRENT_WORK.md`：当前正在推进的改动和风险
3. `docs/RENDERING.md`：渲染系统结构和 GL/Metal 对齐规则

不要默认阅读这些依赖/产物目录，通常会浪费上下文：

- `deps/`
- `tools/node_modules/`
- `tools/ndk/`
- `tools/pkgs/`
- `tools/linux/`
- `libs/*/node_modules/`
- `out/`

## 项目简介

**Quark** 是一个跨平台 GUI 框架，支持 Android / iOS / macOS / Linux。

- 核心用 **C++** 实现，底层以自定义 **OpenGL** 渲染管线为主，仓库中也包含正在完善的 **Metal** 后端
- 嵌入 **JavaScript / JSX** 运行时，用于 UI 描述和业务逻辑
- 不是 Web 运行时，有独立布局引擎，不依赖浏览器 DOM
- 类 CSS 样式系统，支持 class 选择器、层级选择器和伪状态
- 运行时模型强调显式视图树、显式事件处理和可控性能；不是浏览器兼容层

## 项目结构

```
quark/
├── src/        # C++ 核心源码
├── deps/       # 依赖库
├── examples/   # 示例代码
├── libs/       # 库文件
├── test/       # 测试
├── tools/      # 构建工具
└── trial/      # 试验代码
```

## 开发工具链

- 使用 `qkmake` 工具初始化、构建、导出项目
- 安装：`sudo npm install -g qkmake`
- 依赖同步：`make sync`
- 构建：`qkmake build`
- 导出：`qkmake export ios` / `qkmake export android`
- 调试服务器：`qkmake watch`
- README 说明目前工具链主要面向 macOS，Windows 暂不支持

## AI 协作约定

- 修改前优先阅读相关实现，尤其是同一功能在 GL / Metal / 平台后端中的对应代码。
- 保持改动范围小，不做无关重构，不格式化整文件。
- 工作区经常有未提交改动，除非明确要求，不要回滚、删除或覆盖别人的修改。
- 新增实现时优先沿用项目已有类型、宏和资源生命周期管理方式，例如 `ImageSource` / `TexStat` / `RenderResource`。
- 渲染相关改动要注意命令编码器、纹理所有权、mipmap、blend mode、clip 和 z depth 的状态恢复。
- 如果只是补 TODO 或修局部 bug，避免跑过重检查；能做轻量编译或局部 grep 验证即可。
- 完成重要改动后，必要时更新 `docs/CURRENT_WORK.md` 或相关模块文档，让下一次 AI 会话能接上。

## 渲染代码提示

- GL 后端通常是行为参考；Metal 后端实现应尽量对齐 GL 的语义，但不要照搬 GL 的状态机做法。
- `Canvas` 公共逻辑在 `src/render/gpu_canvas.*`，后端只实现 `*Cmd` 方法。
- 纹理传递通过 `ImageSource::texture()` 和 `setTex_SourceImage()` 维护，Metal 指针需要正确 retain/release。
- Metal 中切换 render target 需要结束当前 `MTLRenderCommandEncoder`，但不一定需要新建 `MTLCommandBuffer`。
- 采样槽位优先使用 shader 结构里的 `fragment.*` 索引，不要硬编码。

## 负责人

- GitHub: louis-tru
- 仓库: https://github.com/louis-tru/quark
- API 文档: http://quarks.cc/doc/
