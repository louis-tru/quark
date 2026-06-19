# GPU 2D 抗锯齿研究记录

本文记录 Quark/Qk 2D GPU 渲染器的抗锯齿研究、当前问题、外部项目学习结论，以及后续可能的架构路线。

这不是最终方案说明，而是持续更新的技术笔记。目标是把 AA 质量和性能推到 GUI 渲染器能承受的上限，同时避免为了局部问题把管线复杂度推爆。

## 为什么这件事重要

Quark 是 GUI 框架，不是 3D 游戏渲染器。GUI 的 2D 边缘质量要求更苛刻：

- 文字、图标、边框、圆角矩形和矢量形状会长期静止显示。
- 用户很容易看到边缘抖动、alpha 过强、角点不均匀、细线忽粗忽细。
- 大面积纯色和清晰曲线相邻时，coverage 的一点点错误都会很明显。
- 沉浸感不仅来自性能，也来自安静、稳定、精确的二维边缘。

因此 AA 不能只是“有一圈模糊边”。它需要接近真实像素覆盖，且在平移、缩放、旋转、裁剪和不同后端之间稳定。

## 当前 Qk 状态

当前 GPU 路径主要依赖 AASide 风格的边缘条带：

- 实体区域正常绘制。
- 边缘外生成一圈 soft band。
- fragment 根据插值的 `aaSide` 计算 alpha。
- 它更像一种 SDF/距离条带近似，不是真正逐像素几何覆盖。
- 当前顶点数据仍是 `{x, y, aaSide}` 这种 `Vec3` 形式。

相关代码区域：

- `src/render/gpu_canvas.*`：共享的 path fill/stroke 绘制逻辑，包含 `_phy2Pixel` 和缩放处理。
- `src/render/pathv_cache.*`：路径三角形、边缘条带和 stroke geometry 缓存。
- `src/render/shader/_util.glsl`：AA、clip、fragment 辅助函数。
- `src/render/shader/color*.glsl`、`image*.glsl`、`triangles.glsl`：消费 AA 字段的 fragment 路径。
- `src/render/gl/gl_command.*`：当前 GL 行为参考。
- `src/render/metal/mtl_canvas.*`：正在对齐中的 Metal 后端。
- `src/render/cgaa.cc`、`src/render/shader/_cgaa.glsl`：CGAA / compute AA 原型方向。

目前经验判断：

- AASide 对普通 UI 边缘仍然有价值，尤其是细 stroke/hairline。
- CGAA 对复杂 fill 有潜力，但现在还不能直接替代所有路径。
- 极小 stroke，尤其是物理 1px 左右的线，CGAA 视觉上容易显粗；AASide 中心线向外扩散的距离场处理反而更可控。
- 对 stroke 来说，经过 stroke tess 后一般不会出现 fill 那种极限角度、几乎重合边、复杂 winding 问题，所以 stroke 可以优先保留 AASide 特化路径。
- 对 fill 来说，单纯几何 AA 很难解决所有 coverage 和多轮廓问题，compute/tile 方向更值得继续。

## 当前问题清单

### AASide 的问题

- 它不是精确 coverage，只是软边启发式。
- alpha 容易随缩放、变换和子像素位置变化。
- 角点、join、cap、圆角处可能出现不均匀 ramp。
- 极细线的视觉宽度很难同时满足物理准确和主观好看。
- 对强斜切、非均匀缩放、透视式变形，按局部边距离处理可能会变差。

### CGAA 的问题

CGAA 目标是更准确地计算 tile 内边覆盖，但目前观察到的问题包括：

- 性能开销很大，binning、tile 处理和 mask 输出成本都明显。
- 直接用于小 stroke 时视觉不够好，尤其物理 1px 附近显粗。
- 多路径、多轮廓分别 AA 后再合成，会出现背景漏光或拼接缝隙。
- 单个路径内部的 coverage 可以算得更准确，但多个相邻图元如果独立 resolve，仍会在边界产生错误。

边界裁剪相关经验：

- 逻辑裁剪 range 需要先转换到边坐标/屏幕像素坐标再参与边处理。
- 对右/下边界，边正好落在 `end` 上时不能用 `>= end` 丢弃；应使用 `> end` 这种严格在外侧的判断。
- 被裁掉的边可能让某些 tile 不再有 crossing edge，从而被当成纯色 tile，表现为颜色向右或向下延伸。
- 这类问题本质上不是“水平边/垂直边能不能忽略”，而是 tile 是否仍然拥有足够的边信息来决定 coverage 和 backdrop。

### 逻辑图元 coverage 问题

这是目前最关键的质量问题之一。

多个几何轮廓各自做 AA，再按普通 alpha blend 合成，会在共享边界漏出背景。典型例子是 border 被拆成多块、同色或异色区域相接、圆角边框和填充区域分开绘制。

同色图元比较容易处理：

- 可以在 CPU 合并成一个逻辑图元。
- 或者在同一 mask / coverage pass 中统一 resolve。

不同颜色或不同填充更难：

- 不能简单合并为一个颜色。
- 共享边界处不能让两个图元各自半透明后露出背景。
- 正确做法更接近“同一个像素/采样点只属于某个前景图元，或者由有序图元共同 resolve”，而不是每个路径独立 AA 后再 blend。

这说明最终架构应从“每个 path 独立 coverage atlas”走向“按目标 tile 收集图元命令，然后在 tile 内按顺序统一 coverage/color resolve”。

## 设计目标

后续 AA 工作应满足：

- 子像素平移、缩放、旋转下稳定。
- GL、Metal、未来 Vulkan 尽量一致。
- 曲线、join、cap、圆角矩形、clip 边缘质量高。
- premultiplied alpha 行为可预测。
- 常见 GUI 场景性能足够好。
- 能与文字、图片 mask、blur、render-to-texture、clip stack 共存。
- 不以 CPU 软件光栅作为主路线，保持 GPU-first。

## 候选技术路线

### 改进 AASide

保留当前边缘条带，但让 `aaSide` 语义更明确：

- `aaSide < 0`：shape 内部。
- `aaSide = 0`：真实几何边。
- `aaSide > 0`：shape 外部。

fragment 使用 `fwidth(aaSide)`、`dfdx`、`dfdy` 推导当前设备像素下的 AA 宽度，而不是只相信 CPU 生成的条带宽度。

优点：

- 改动小，适合保留现有 path cache。
- stroke/hairline 可以继续走这条路。
- 对简单边界、按钮、普通 UI shape 够快。

风险：

- 仍然是启发式，不是真正面积覆盖。
- join/corner/极小图形需要额外策略。
- 多图元共享边界的漏光问题不能靠单个 AASide 解决。

### 解析 coverage

在 shader 中根据边方程、signed distance 或边积分计算像素覆盖。

优点：

- 子像素行为更稳定。
- AA 宽度可以明确绑定到设备像素。
- 对 fill 的理论质量更好。

风险：

- 复杂路径、自交、fill rule、clip stack 和多轮廓处理难。
- 需要完整 tile/bin/backdrop 机制，否则无法解决全局 winding。

### MSAA 或 hybrid AA

MSAA 对三角形边缘天然有效，可以作为对比或特定 surface 的 fallback。

但它不应成为主路线：

- offscreen surface 和 render-to-texture 成本高。
- 不解决 shader mask、image filter、复杂 clip 的所有问题。
- 后端 resolve 行为难以完全一致。

### Coverage mask / tile mask

把路径 coverage 先写入 mask，再 composite。

优点：

- 可复用复杂路径 mask。
- 适合 clip 和 path fill 统一。

风险：

- 额外 pass 和内存成本。
- 如果 coverage 计算仍然粗糙，mask 只是存储错误结果。
- per-path mask 不能自然解决多图元共享边界漏光。

### Compute AA / CGAA

把路径边分配到 tile，在 compute shader 中计算 tile 内 coverage。

这是当前最值得研究的 fill 方向，但要注意：

- 它不是单个 shader 可以解决的问题，而是完整的路径编译、binning、backdrop、tile resolve 管线。
- 如果仍然每个 path 独立生成 mask，再普通 alpha blend，边界漏光仍会存在。
- 性能关键在于减少 CPU/GPU 同步、减少全局原子、减少临时 buffer、控制每 tile 工作量。

## Skia 研究结论

Skia 的经验说明，成熟 2D 渲染器通常不是单一 AA 算法，而是一组特化 renderer：

- 简单形状走快速 analytic renderer。
- stroke/hairline 有专门路径。
- 复杂 path、clip、mask 走其他路径。
- 大量质量问题通过“选择正确 renderer”解决，而不是一个算法包打天下。

CCPR（Coverage Counting Path Renderer）值得学习思想，但不适合作为 Qk 近期直接实现目标：

- 架构复杂，依赖 Skia 自身的 atlas、batch、资源管理。
- 对 Qk 当前 GL/Metal 对齐阶段来说成本太高。
- 它强调 coverage counting 和 mask atlas，这与 Qk 的 CGAA 方向有相似处，但不能照搬。

Graphite / Vello 方向说明现代 GPU 2D 正在往 compute-driven pipeline 发展：

- CPU 记录 compact scene。
- GPU 做 path 处理、binning、coarse/fine raster。
- tile 内统一 resolve coverage 和 color。

这与 Qk 后续要解决的多图元 AA、clip stack、性能问题方向一致。

## Pathfinder 研究记录

本地项目位置：

- `/Users/louis/Project/graphics/pathfinder`

重点文件：

- `renderer/src/builder.rs`
- `renderer/src/gpu/d3d11/renderer.rs`
- `renderer/src/tiler.rs`
- `renderer/src/gpu_data.rs`
- `shaders/d3d11/dice.cs.glsl`
- `shaders/d3d11/bin.cs.glsl`
- `shaders/d3d11/bound.cs.glsl`
- `shaders/d3d11/propagate.cs.glsl`
- `shaders/d3d11/fill.cs.glsl`
- `shaders/d3d11/tile.cs.glsl`
- `shaders/d3d11/sort.cs.glsl`
- `shaders/d3d11/fill_area.inc.glsl`
- `shaders/d3d11/tile_fragment.inc.glsl`

### 总体架构

Pathfinder 3 是一个 GPU-based vector rasterizer，支持 GL/WebGL/Metal。D3D11 路径使用 compute shader，D3D9 风格路径则更多依赖传统硬件 rasterization。

D3D11 compute 管线大致为：

```text
CPU Scene / DisplayList / Paint
  -> CPU 编码 path segment、tile bounds、batch metadata
  -> GPU dice 曲线/线段，生成 microline
  -> GPU bin microline，写入 per-tile fill linked list 和 backdrop delta
  -> GPU propagate 按列累积 backdrop，处理 clip、solid/alpha tile、tile list
  -> GPU fill alpha tile，把面积 coverage 写入 mask atlas
  -> GPU sort 每个 framebuffer tile 的 draw list
  -> GPU tile 按顺序 composite 最终颜色
```

### CPU 做什么

Pathfinder D3D11 并不是“CPU 展平路径，其它全在 GPU”这么简单。更准确地说：

- CPU 记录 scene/display list/paint。
- CPU 计算 path bounds、tile bounds 和 batch metadata。
- CPU 上传 contour points、segment index、segment flags。
- 曲线仍以 quadratic/cubic flag 保留下来。
- 真正的 dice/flatten 在 GPU `dice.cs.glsl` 中执行。

这点很重要：它的 CPU 不是完全做 coverage，也不是完全做 flatten，而是做 scene 编码和调度准备。

### GPU dice

`dice.cs.glsl` 每个 invocation 处理一个原始 segment：

- line 按 `MICROLINE_LENGTH = 16` 近似拆分。
- quadratic/cubic 根据 tolerance 估算需要拆成多少 microline。
- 输出 packed microline，供后续 bin 使用。

这让 GPU 端拥有足够细的直线边，但 CPU 不需要预先把曲线完全展开。

### GPU bin

`bin.cs.glsl` 基本是 CPU tiler 的 GPU 版：

- 每条 microline 被分配到穿过的 16x16 tile。
- tile 内写 fill segment linked list。
- 对跨 tile 边界的线更新 backdrop delta。
- 使用类似 Amanatides/Woo 的格子遍历方式。

这一步是 Pathfinder 的核心之一：它不是直接画三角形边，而是先把边变成 tile-local 工作。

### GPU propagate

`propagate.cs.glsl` 按 path tile column 工作：

- 从上到下累积 backdrop。
- 处理 clip。
- 区分 solid tile 和 alpha tile。
- 为需要细算 coverage 的 tile 分配 alpha tile。
- 建立 framebuffer tile 对应的绘制链表。

也就是说，Pathfinder 不只计算 coverage，还在 GPU 上组织“哪些目标 tile 需要画哪些 path”。

### GPU fill

`fill.cs.glsl` 每个 alpha tile 一个 workgroup：

- tile 大小是 16x16。
- local size 是 16x4。
- RGBA 通道存四个垂直像素。
- 使用 `fill_area.inc.glsl` 中的 `computeCoverage` 和 area LUT。
- coverage 来自 line area contribution + backdrop。

这不是简单 supersampling，而是边面积积分。

### GPU tile composite

`tile.cs.glsl` 每个 framebuffer tile 一个 workgroup：

- 读取排序后的 tile list。
- 读取 mask/backdrop。
- 计算 paint/color。
- 按 draw order 合成最终颜色。

这一点对 Qk 很关键：Pathfinder 的最终单位不是“某个 path 的 mask”，而是“目标 framebuffer tile 中的一串绘制项”。这比当前 per-path atlas 更接近解决多图元 AA 的方向。

### CPU tiler 路径

`renderer/src/tiler.rs` 中的 CPU tiler 实现了论文 Random-Access Rendering of General Vector Graphics 的类似思路，并结合 Amanatides/Woo tile traversal：

- `process_line_segment` 先把线段 clip 到 view。
- 再遍历穿过的 tile。
- 向 tile 写入局部 fill segment。
- 穿过 top/bottom boundary 时写辅助 segment。
- 对 right boundary enter/leave 调整 backdrop。

这部分对理解 CGAA 的 tile/backdrop 很有参考价值。

### Pathfinder 对 Qk 的启发

Pathfinder 不是单纯 AA 算法，而是完整 tile scene renderer。

值得 Qk 学的东西：

- path fill 应该从 per-path 独立处理，升级到 batch/tile 级别组织。
- CPU 可以负责 scene 编码、bounds、batch metadata，不必 CPU raster coverage。
- 曲线可以在 GPU dice 成 microline，减少 CPU flatten 压力。
- coverage mask 和 color composite 应分离，但最终要在目标 tile 内有序 resolve。
- tile list / path tile / framebuffer tile 三层结构，可以解决很多当前 CGAA 的调度混乱。

不宜直接照搬的东西：

- Pathfinder D3D11 的 buffer/linked-list/atomic 方案复杂，直接搬到 Qk 会很重。
- Qk 还要兼容 GL/Metal/Vulkan，不应先把全部架构绑到某个 compute 版本。
- 当前最现实的是先吸收 tile/backdrop/ordered tile list 的结构思想。

## Vello 研究记录

本地项目位置：

- `/Users/louis/Project/graphics/vello`

重点文件：

- `README.md`
- `doc/ARCHITECTURE.md`
- `vello/src/render.rs`
- `vello/src/shaders.rs`
- `vello_encoding/src/encoding.rs`
- `vello_encoding/src/path.rs`
- `vello_encoding/src/estimate.rs`
- `vello_shaders/shader/shared/config.wgsl`
- `vello_shaders/shader/flatten.wgsl`
- `vello_shaders/shader/binning.wgsl`
- `vello_shaders/shader/coarse.wgsl`
- `vello_shaders/shader/path_tiling.wgsl`
- `vello_shaders/shader/fine.wgsl`
- `vello_shaders/shader/shared/ptcl.wgsl`

### 总体架构

Vello 是 GPU compute-centric 的 2D renderer，使用 wgpu。它把传统上很串行的工作，比如 path 扫描、clip、排序、tile 命令构建，尽量转成 prefix-sum / monoid / scan 形式在 GPU 上并行执行。

Vello 的层次：

- `Scene`：应用侧构建的场景。
- `Encoding`：线性编码流，记录 path、draw、style、transform、resource。
- `Recording`：渲染前准备出的命令和资源。
- `WgpuEngine`：执行 shader pipeline。

它不是 CPU 生成一堆三角形再交给 GPU，而是 CPU 记录紧凑 scene，GPU 编译成 tile 工作。

### Encoding

`vello_encoding` 把场景拆成线性流：

- path tags/data
- draw tags/data
- transforms
- styles
- images/resources

`Style` 同时编码 fill/stroke。路径可以是 `LineSoup`、`SegmentCount`、`PathSegment`、`PathTag` 等形式。

这说明 Vello 的 CPU 主要是“编码意图”，不是直接生成最终 coverage geometry。

### 渲染管线

`vello/src/render.rs` 中的主流程大致是：

```text
CPU Encoding / Resolver
  -> pathtag_reduce
  -> pathtag_reduce2
  -> pathtag_scan1
  -> pathtag_scan
  -> bbox_clear
  -> flatten
  -> draw_reduce
  -> draw_leaf
  -> clip_reduce
  -> clip_leaf
  -> binning
  -> tile_alloc
  -> path_count_setup
  -> path_count
  -> backdrop
  -> coarse
  -> path_tiling_setup
  -> path_tiling
  -> fine_area 或 fine_msaa8 / fine_msaa16
```

这比 Pathfinder 更进一步：不仅 dice/bin/fill 在 GPU，连 variable-length path stream 的 prefix scan、draw/clip reduction、tile command list 构建也都在 GPU。

### flatten

`flatten.wgsl` 在 GPU 上把 path segment 转为 line soup：

- 对 cubic/quadratic 进行 GPU flatten。
- 使用 Euler spiral / cubic 相关逻辑估算分段。
- 同时更新 path bbox。

这与 Pathfinder 的 GPU dice 类似，但 Vello 和后续 prefix pipeline 结合得更深。

### binning

`binning.wgsl` 把 draw object 分配到 bin：

- tile size 是 16x16。
- bin 是 16x16 个 tile 的更大块。
- 使用 workgroup bitmap 和 bump allocator。

这个设计目标是减少全局散写和提高并行组织效率。

### coarse

`coarse.wgsl` 是 Vello 架构核心：

- 根据 bin 中的 draw object 构建 per-tile command list，简称 PTCL。
- 处理 clip stack、blend stack、solid/color/image/gradient 等命令。
- 为 fine stage 准备 tile-local 命令流。

这一步与 Qk 当前问题高度相关。多图元边界漏光的问题，本质上需要 tile 内统一命令流和统一 resolve，而不是 path 独立 AA。

### path_tiling

`path_tiling.wgsl` 把 path segment 写成 tile-relative segment buffer：

- 处理鲁棒 epsilon。
- 为 fine raster 提供 tile 内线段。
- 与 backdrop / path count 等数据配合。

### fine

`fine.wgsl` 是最终 tile 内 raster/composite：

- 支持 `AaConfig::Area`。
- 支持 `Msaa8` / `Msaa16`。
- workgroup size 典型为 4x16。
- 每个 thread 处理 4 个横向像素。
- 读取 PTCL 命令流，按顺序执行 fill、solid、color、gradient、image、clip、blend 等。

Vello 的 area AA 是在 fine stage 内对 line segment 做面积积分；MSAA 模式则用 shared memory / SWAR winding 处理采样。

这说明 Vello 的 AA 不是孤立模块，而是和 tile command execution 绑定在一起。

### Vello 对 Qk 的启发

Vello 的核心启发：

- 最终正确方向是 per-tile command list + fine raster/composite。
- GPU prefix-sum/monoid 可以把复杂 scene 编译搬到 GPU，但实现成本很高。
- 独立 path mask 不是终局；tile 内有序 resolve 才能解决多图元边界。
- area AA 和 MSAA 可以作为同一 fine stage 的不同配置。
- 需要 robust buffer allocation / readback / fallback，因为 GPU 端动态分配可能失败。

对 Qk 来说，Vello 是长期目标参考，不是近期直接重写模板。

## Pathfinder 与 Vello 对比

| 维度 | Pathfinder | Vello |
| --- | --- | --- |
| CPU 角色 | 构建 scene/display list、bounds、batch metadata、segment buffer | 记录 compact scene encoding，打包资源 |
| 曲线处理 | D3D11 中 GPU dice/flatten 成 microline | GPU flatten 成 line soup |
| 工作组织 | path tile、alpha tile、framebuffer tile list | bin、tile allocation、PTCL、fine stage |
| coverage | alpha tile 中面积积分 + backdrop | fine stage 中 area AA 或 MSAA |
| composite | tile shader 按 tile list 有序合成 | fine shader 执行 PTCL 并合成 |
| clip/blend | 有 clip 和 tile list 处理，但整体较 Pathfinder 自身风格 | clip/blend stack 深度融合进 PTCL |
| 架构复杂度 | 已经很复杂，但比 Vello 更像固定 pipeline | 更像 GPU scene compiler，prefix scan 很重 |
| 对 Qk 近期价值 | 高，适合学习 tile/backdrop/list 结构 | 高，但更偏长期架构参考 |

一句话总结：

- Pathfinder 教 Qk 如何从“路径”走向“tile scene renderer”。
- Vello 教 Qk 如何进一步把 scene 编译、clip、blend、AA 都收进 GPU tile pipeline。

## 对 Qk 的修改方向

### 近期策略

保持保守，不要立刻重写整个 renderer。

推荐：

- stroke/hairline 继续走 AASide 或专门细线 renderer。
- 小于约 1.8 物理像素的 stroke，可优先使用中心线向外扩散的 AASide 距离场。
- CGAA 先限定在 fill path，不急着覆盖 stroke。
- CGAA 继续修正边界、clip、tile backdrop 和性能问题。
- 把 direct-target output 作为实验，但必须避免无序 overlapping compute write。
- 文档和测试先记录清楚哪些 shape 走哪条 AA 路径。

不推荐：

- 不要期望 CGAA 一次解决 stroke、fill、border、clip、image mask 的所有问题。
- 不要为了局部漏光把 UI 层强行合并所有图元。
- 不要把 per-path mask 当作最终架构。

### 中期策略

建立 Qk 自己的 tile-scene AA 结构。

方向：

- CPU 仍负责路径记录、bounds、paint、clip 的初步编码。
- GPU 或 CPU 把 path 分配到目标 tile。
- 每个 framebuffer tile 拥有有序 draw list / command list。
- coverage 和 color 分离存储，但最终在 tile 内统一 resolve。
- fill path 使用 area coverage + backdrop。
- stroke/hairline 作为专门命令进入 tile list，或继续走现有快速路径。

这一步更接近 Pathfinder，而不是直接跳到 Vello。

### 长期策略

如果 Qk 的 compute 后端成熟，尤其是 Metal/Vulkan 稳定后，可以考虑 Vello 式架构：

- CPU 记录 compact scene stream。
- GPU 做 prefix scan、path flatten、draw/clip reduction。
- GPU 构建 PTCL。
- fine stage 统一执行 tile 内命令。
- area AA/MSAA/hairline/stroke/clip/blend/image 都以 tile 命令形式接入。

这是质量和性能的上限方向，但实现成本也最高。

## 推荐路线图

### 第一阶段：稳定当前 AA 路径

- 保留 AASide 作为 stroke/hairline 主路径。
- 修正 CGAA 的边界裁剪、tile 覆盖、backdrop 传播问题。
- 明确 CGAA 只先服务 fill。
- 建立一组固定截图测试：圆角矩形、1px stroke、border、clip、不同颜色相邻区域、旋转缩放。

### 第二阶段：CGAA fill 可用化

- 优化 binning 成本。
- 降低 tile buffer / mask buffer 写入成本。
- 区分 solid tile 和 alpha tile。
- 避免对完全空白或完全实心 tile 做昂贵 coverage。
- 评估直接写目标 surface 与 mask atlas 的性能差异。

### 第三阶段：tile draw list

- 引入 framebuffer tile 级 draw list。
- 同一目标 tile 内按 draw order resolve 多个 path。
- 先支持 solid color / simple fill。
- 再支持 gradient、image、clip、blend。

这是解决多图元 AA 漏光的关键阶段。

### 第四阶段：更完整的 GPU scene pipeline

- 研究 GPU flatten/dice 是否值得引入。
- 研究 prefix scan / monoid 是否能减少 CPU 预处理和 draw call。
- 逐步接近 Vello 式 coarse/fine 架构。

## 测试与评估计划

必须持续用图片和性能一起评估：

- 物理 1px、2px、3px stroke。
- 小半径圆角矩形。
- 大圆、细圆环、斜线、近水平/近垂直边。
- 多个同色图元拼接。
- 多个异色图元共享边。
- border + fill 分离绘制。
- clip path，尤其是不规则 clip。
- 非整数平移。
- 2x/3x surface scale。
- 非均匀缩放、旋转、斜切。
- offscreen surface / render-to-texture。

评估指标：

- 是否漏背景。
- 是否显粗或显细。
- 子像素移动是否抖动。
- GL/Metal 是否一致。
- GPU 时间、buffer 写入量、tile 数、path 数增长时的曲线。

## 当前结论

1. AASide 不是失败路线，它仍适合 stroke/hairline 和简单 UI 边缘。
2. CGAA 可以提高 fill coverage 精度，但它必须走向 tile-scene resolve，否则无法根治多图元边界问题。
3. Pathfinder 的最大价值是展示了 path tile、alpha tile、framebuffer tile list、area coverage、ordered composite 这一整套结构。
4. Vello 的最大价值是展示了更激进的 GPU scene compiler：encoding、prefix scan、coarse PTCL、fine raster/composite。
5. Qk 不应直接复制任意一个项目，而应按阶段吸收：
   - 近期：AASide stroke + CGAA fill。
   - 中期：Pathfinder-like tile draw list。
   - 长期：Vello-like GPU scene pipeline。

真正要解决的不是“某条边怎么 AA”，而是“一个目标 tile 内所有图元如何共同决定每个像素最终属于谁、覆盖多少、颜色是什么”。

这才是 Qk 渲染器最重要的一块拼图。
