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

当前 GPU 渲染已经进入混合策略，而不是单一 AA 算法：

- CAPA 是当前 Metal-class 复杂填充路径的主线，用面积 coverage、
  tile/layer 计划和有序 composite 修复多图元共享边界漏光。
- AASide 保留给 hairline、文本和简单边缘。它不是严格面积 coverage，
  但距离/边沿带的视觉效果在直线和细线场景中通常更好。
- CGAA 不再是当前主线；除非明确要求，不应继续围绕 CGAA 做大改。

CAPA 封版判断：

- CAPA coverage 必须保持线性面积语义，不能在漏光修复前做
  `f(coverage)`、gamma、smoothstep、感知滤波或邻域扩散。
- 如果要尝试离散/感知 coverage，只能在 coverage group 完成并且不再
  参与几何归属后作为显示实验。当前对应实验 flag 是
  `CAPA_FLAG_COMPOSITE_QUANTIZE_COVERAGE`，默认关闭。
- CAPA 不追求在所有单边视觉质量上超过 AASide。它解决的是有序面积
  归属、复杂重叠和背景 seam；简单边缘质量靠 renderer selection。
- 移动端 CPU 成本需要纳入 renderer selection。CAPA 在大批量提交、
  大量重叠、需要有序 composite/漏光修复的场景才最有优势；如果被
  clip、image、text、blur/filter 等状态频繁打断并不断 flush，固定 pass
  和提交成本会压过收益。后续方向应是减少 flush，把 clip 和更多合成项
  纳入 CAPA/CPAP 主管线，或把特殊效果先离屏成 texture 后再交给 CAPA
  合成。

历史上 GPU 路径主要依赖 AASide 风格的边缘条带：

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
- CAPA 已经取代 CGAA 成为复杂 fill 的主要研究/实现方向。
- 极小 stroke，尤其是物理 1px 左右的线，CGAA 视觉上容易显粗；AASide 中心线向外扩散的距离场处理反而更可控。
- 对 stroke 来说，经过 stroke tess 后一般不会出现 fill 那种极限角度、几乎重合边、复杂 winding 问题，所以 stroke 可以优先保留 AASide 特化路径。
- 对 fill 来说，单纯几何 AA 很难解决所有 coverage 和多轮廓问题，
  CAPA 的 compute/tile/order composite 方向已经成为当前默认复杂路径。

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

### 2026-06 CAPA / 浏览器 / CGAA 对比结论

这轮实验从浏览器 SVG、Canvas bitmap、CGAA、AASide 和 CAPA 的动态旋转
对比得到几个阶段性判断：

- 浏览器 SVG 的最佳表现并不是“完美解析边缘”。小图时边缘看起来更稳，
  但放大后仍能看到纹理/采样毛边，说明它很可能经过了中间纹理或
  grid/sample 类路径，而不是每帧做无限精度的真实几何覆盖。
- SVG geometry 动态变形时更接近 GRID 采样，观察上像 4x4 或 2x2 grid。
  它也有波浪/锯齿，只是形态不同。
- Canvas cached bitmap 在比例、采样、mipmap 没处理好时质量最差；修正比例
  后，单张高分辨率纹理旋转并配合合适 mip/filter，小图可以隐藏很多锯齿，
  但放大后锯齿会回来。
- CGAA 4x4 与浏览器 SVG 的若干表现接近，尤其是离散 grid 造成的阶梯感。
- CAPA 当前面积积分结果与 AASide 的主观质量非常接近。这说明 AASide 的
  距离边 ramp 在很多普通边缘上已经很接近面积积分。
- 面积积分本身并不自动消除动态波浪。CAPA/AASide 的波浪更均匀、柔和；
  GRID/CGAA 的波浪更像离散采样造成的锯齿流动。二者都是代价不同的
  approximation，并没有免费的“浏览器级”神秘 AA。

当前 CAPA 工程状态：

- 分支 `experiment/capa` 的提交 `7564f7a92`
  (`Restore CAPA prototype snapshot`) 是当前恢复点。
- CAPA 和 CGAA 应并存；当前研究继续在 CAPA 上进行，不要再默认切回 CGAA。
- CAPA 的目标已经收窄：它只应作为解决多图元相邻边界漏光 /
  background seam 的 ordered tile resolve/compositor 研究路径。如果不解决
  漏光，CAPA 相比 AASide/CGAA 没有足够优势。
- AASide、CGAA、CAPA 三条线可以并行存在并通过宏或运行时参数切换：
  AASide 负责最快普通 UI/stroke/hairline，CGAA 负责较轻的 coverage/area
  路径，CAPA 负责高复杂度 no-leak compositor。
- 后端范围要收敛：GL/GLES 只需要保底 AASide，不计划支持 CGAA 或 CAPA。
  CGAA/CAPA 属于 Metal/Vulkan-class compute 后端方向。
- 当前正确图像依赖 `capa_tile.glsl` 中 `rawCount > 0u` 时走完整
  `capa_area_coverage(...)` 的 fallback。
- CAPA 早期恢复图像时仍有约 7 ms/frame GPU 时间，主要问题是无效 tile
  work 和昂贵全量 area fallback。后续经过 empty-tile removal、full-tile
  preblend、small-tile staging、boundary/classify/prefix 拆分后，100 个重复
  五角星本地测试约为：CGAA `4.5ms`，AASide `3.1ms`，CAPA `3.2ms`。这个
  场景有大量重复重叠，预混合排除了大部分下游工作；但当前结论已经从
  “CAPA GPU 明显落后”变成“CAPA GPU 接近 AASide、超过 CGAA”，并且 CPU
  时间明显领先 AASide/CGAA。
- 下一步应先理解 CAPA 的 pass、bin/tile/backdrop 数据流，再决定大结构优化。
  重点是减少无效 tile work、减少昂贵全量 area fallback，并逐步把 CPU 侧
  tile/bin/backdrop 准备搬到 GPU，而不是在 CPU 侧继续堆逻辑。
- Xcode Metal capture 需要把 frame capture `Count` 设为 2 才能稳定抓到
  当前 Qk/Metal 帧。一次实际 capture 显示 `CAPA tile coverage` 占绝对
  大头（约 93.8%），而 `CAPA prepare` 与 `CAPA short-edge bin` 都很小
  （约 0.05% 与 0.18%）。这说明当前瓶颈不是 atomic binning 或矩阵/短边
  准备，而是 tile coverage 中仍然依赖全路径扫描/fallback。
- 如果 CAPA 真要根治漏光，它最终不能只是单通道 coverage atlas。它必须在
  CAPA/tile 管线内按 draw order 直接混合目标颜色，包含 solid color、
  gradient、image sampling 和 blend。这样会失去“只存边框/coverage atlas”
  和纹理压缩式轻量路径的优势，性能不应再与 AASide/CGAA 做同类预期。
- blur/filter/output image/readback 等依赖临时 A/B 绘图或复杂后处理的操作
  不应强行塞进 CAPA 管线。遇到这些操作时可以结束/flush CAPA pass，再回到
  普通 backend 路径。显式 clip 变化也可以先作为 CAPA pass 边界处理：
  CAPA 第一版只消费当前 clip 状态，不负责内部生成 clip。这样可能留下少量
  跨 pass 图元交错漏光，但当前 Qk GUI 场景预计 90% 以上不会遇到可见问题。
- CAPA ordered compositor 需要把 image paint 的纹理资源做成每个 pass 的
  texture table，命令流只存 `textureIndex` / `samplerIndex`。Metal 优先用
  argument buffer indexed textures；Vulkan 运行时查询
  `VkPhysicalDeviceLimits` 的 sampled-image 数量限制，并查询 descriptor
  indexing / non-uniform indexing 能力。默认目标可以是每个 CAPA pass 32 个
  texture；如果设备只保证 16 个，就降到 16。达到上限时结束当前 CAPA pass。
- CAPA ordered color blending 不应为每个 `path.tile` 分配完整 RGBA
  像素空间。颜色由最终混合阶段现场根据 path paint 计算；中间缓存只保存
  必需的 coverage。空 tile、full tile、uniform coverage tile 不分配
  coverage/backdrop page，只在 path-tile/header 上保存常量语义。只有
  AA/edge path-tile 才从固定预算的 R8 coverage/backdrop page pool 中分配
  `16x16` page。这个前提很重要：实心 tile 与空 tile 必须能完全脱离边界
  coverage 和 row-backdrop 存储，否则长度预算无法成立。
- CPU 侧无法只靠 conservative bounds 证明真实 coverage page 数量，因此
  不要回到最保守的 bounds 面积分配。第一版应使用 GPU atomic page
  allocator，并为 `boundaryTileIndex` 保留高位特殊值：`CAPA_NIL` 表示
  空/外部/分配失败，`CAPA_FULL_TILE = CAPA_NIL - 1` 表示实心/full tile，
  真实 boundary tile index 从 `0` 开始。Overflow 是预算/debug 失败，不是
  正常 shader correctness fallback，因为边界 coverage 与 row-backdrop
  状态是一体的，不能在 final compositor 便宜重建。需要记录 `usedPages`、
  `overflowPathTiles`、`overflowGlobalTiles`、`edgeTileCount`、
  `fullTileCount`、`emptyTileCount` 等计数，帮助选择合适的 pool 预算。
- 一个更实用的 CPU 侧 coverage page 预算不是 path bounds 面积，而是
  surface-space 总边长除以 `16`：`estimatedEdgeTiles = ceil(totalEdgeLength
  / kCAPATileSize)`。同一个 tile 内常有多条边，因此这个估算通常会多不少，
  但相比 `boundsArea / 256` 对空心/大描边路径会精确得多。矩阵缩放可以先
  用变换后的边向量长度估算；若要更便宜，可用矩阵列长度/最大奇异值一类的
  保守 scale 放大 path-space 边长。CPU 根据这个估算和预算倍率分配 R8
  coverage page pool；如果估算本身已经超过允许内存，就拆 CAPA pass 或回退
  AASide，不启动 CAPA。
- 如果采用上面的长度预算，shader 里的 overflow 可以先不做昂贵的正确性
  fallback。生产路径可承认 overflow 为应避免的预算错误并丢弃该 edge
  path-tile；调试路径把 overflow tile/layer 用固定颜色叠到目标上。目标是
  通过 length-based sizing 和调试计数把正常 UI 场景的 overflow 降到 0。

### CAPA vNext ordered compositor plan

当前 CAPA 下一版的最新可执行 pass 表以 `docs/CAPA_PASS_PROCESS.md` 和
`src/render/shader/capa/` 为准：1 个 CPU 准备步骤 + 10 个 shader pass。
其中 `capa_prepare_tiles.glsl`、`capa_prepare_dispatch.glsl` 和 `capa_boundary.glsl` 是真实
调度链路的一部分，不能按旧的 7-pass 记忆省略。它的目标不是
替代 AASide/CGAA 的普通 coverage，而是把多 path/layer 的颜色混合移动到
有序 tile compositor 中，解决 shared edge/background seam 漏光。

#### pass0：CPU 构建 path 输入

- CPU 只为每个 path 构建简单展平边数据，不在 CPU 侧构建 CGAA 式精确
  tile list。
- 记录当前 root matrix 的 surface 偏移。这个偏移现在 `GPUCanvas` 没有
  直接刻录成 CAPA 输入；blur 等路径会调整 root 偏移，所以 CAPA pass0
  需要单独保存一份。
- 为每个 path 记录 paint、fill rule、path order、edge offset/count、粗略
  资源预算信息。
- 用变换后的总边长估算 coverage/backdrop page pool：
  `estimatedEdgeTiles = ceil(totalEdgeLength / 16) * budgetMultiplier`。
  如果估算超过预算，优先拆 CAPA pass 或回退 AASide，而不是启动一个必然
  overflow 的 CAPA pass。

#### pass1：prepare paths / edges / tasks

- 对原始边计算单位方向、长度、矩阵变换后的 surface-space 边、粗略裁剪、
  path screen bounds，并处理 pass0 记录的 root matrix 偏移。
- 写短边 task。当前最不均衡的工作预计就是长边拆短边 task。
- 当一个 path 的 bounds 已经可用时，不必等待所有短边 task 写完；完成该
  path 的线程可以先记录 `pathsDone`，并为当前 path 分配连续
  `path.tiles` 存储：

```txt
path.originTileX / originTileY
path.tileEndX / tileEndY
path.tileOffset
```

- 如果 `path.tiles` 空间不足，丢弃该 path 或标记为不可进入 CAPA。
- 关键 atomic：

```txt
atomicAdd(path.edgesDone, 1u)
atomicAdd(pathsDone, 1u)
```

- 最后完成全部 paths 的线程写出后续 pass 的 indirect dispatch 参数，例如
  short-edge task groups、path.tile groups、path.tile row groups 等。短边
  task 的真实数量仍需等 task 写完；如果不等待，就必须用 conservative
  task count 启动短边 binning pass 并在 shader 内跳过无效 task。

#### pass2：按 path 顺序建立 global tile layer 链表

- 一个线程处理一个 global target tile。global tile 自带 `tileX/tileY`。
- 线程按 path order 顺序遍历 paths，用简单范围比对判断当前 global tile
  是否落在 path tile rect 内：

```txt
localX = tileX - path.originTileX
localY = tileY - path.originTileY
inside = localX < path.tileSpanX && localY < path.tileSpanY
pathTileIndex = path.tileOffset + localY * path.tileSpanX + localX
```

- 命中后把该 `path.tile` 按当前 path 顺序串入 global tile 链表：

```txt
pathTile.next = NIL
previous.next = pathTileIndex
globalTile.head = firstPathTileIndex
```

- 同时清零 path.tile 状态。为了省空间，path.tile 可暂不分配默认短边存储，
  短边链表在短边 binning pass 动态分配。
- 这一步负载不均衡，但每个任务很轻；更重要的是它把 final composite 需要的 ordered
  layer list 固定下来，避免 unordered atomic append 破坏 draw order。

#### 短边 binning / boundary tile allocation

- 处理 pass1 写出的短边 task，把短边加入对应 path.tile 的短边链表。短边
  可带一个“完全位于 path.tileX0 左侧”的标记，方便后续面积积分快速排除。
- 当某个 path.tile 添加第一条边时，把它加入 `boundaryTiles`。这个
  boundaryTile 包含该边界 tile 的 row-backdrop 与 R8 coverage page。
- `boundaryTileIndex` 保留特殊值：

```txt
CAPA_NIL       = empty / outside / no boundary / allocation failure
CAPA_FULL_TILE = CAPA_NIL - 1 = solid / full tile
0..N           = real boundaryTile index
```

- 分配逻辑：

```glsl
if (pathTile.boundaryTileIndex == CAPA_NIL) {
  uint idx = atomicAdd(boundaryTiles.count, 1u); // count starts at 0
  if (idx < boundaryTileCapacity)
    pathTile.boundaryTileIndex = idx;
  else
    pathTile.boundaryTileIndex = CAPA_NIL;
}
```

- 不再保留失败/debug coverage tile。分配失败直接回写 `CAPA_NIL`，final
  compositor 不把它入链。
- `capa_boundary.glsl` 是最后一个动态分配阶段。完成线程写出与 `boundaryTiles` 相关的
 后续 indirect dispatch 参数，例如 backdrop/coverage 需要的 boundaryTile row
  groups。

#### backdrop：计算 boundaryTile local backdrop

- 只处理真实 boundaryTiles；真实 boundary tile index 从 `0` 开始。
- 每个 boundaryTile 分配 16 个 row 线程，当前 `local_size_y=2`，一个
  workgroup 处理两个 boundary tiles。
- `capa_backdrop.glsl` 计算每个 boundaryTile 的本地 row area/backdrop：
  - `tileX < path.tileX0` 的贡献临时保存到 tileX0 的
    `CAPAPathTileRow.backdrop[row]`，作为 tileX0 的初始前缀累计值。
  - 每个 boundary tile 自身的 local row value 保存到
    `boundaryTile.backdrop[row]`。
  - `capa_prefix.glsl` 后，`backdrop[row]` 被改写成 coverage pass 需要的
    tile-left row prefix；final coverage pass 会把 packed R8 coverage 写入
    独立的 `CAPACoverageTile.values[64]`。

#### classify + prefix：把 boundary backdrop 转成横向前缀

- `capa_classify.glsl` 扫描 `CAPASmallTile`，用当前 boundary tile 的 row0
  backdrop/prefix 判断 edge-free small tile 是 empty 还是 solid/full。
- `capa_prefix.glsl` 对真实 boundary tile 的每行 backdrop 做横向前缀。
  这里的“row 线程”沿 tileX 从左到右循环，不引入额外 scan。
- `tileX0` 在 `capa_backdrop.glsl` 后已经有左侧前缀值；prefix pass 从
  对应的 `CAPAPathTileRow.backdrop[row]` 读出初始值，然后：

```txt
prefix = tileX0.leftPrefix
tileX0.backdrop = prefix
prefix += tileX0.localSelf
tileX1.backdrop = prefix
prefix += tileX1.localSelf
tileX2.backdrop = prefix
...
```

- 对无边 tile，用前缀值和 path fill rule 判断它是 empty 还是 solid/full：
  `boundaryTileIndex = CAPA_FULL_TILE` 表示实心/full，否则保持 `CAPA_NIL`。
- 到 `capa_prefix.glsl` 结束，面积积分所需的 boundary tile 前缀 backdrop
  准备完成。

#### coverage：boundaryTiles 面积积分写 R8 coverage

- 只处理真实 boundaryTiles，从索引 `0` 开始。每个 boundaryTile row 一个
  线程，通常 4 个 boundaryTile row 组成一个 threadgroup。
- 每个 row 线程使用线程本地 `float[16]` 保存该行 16 个像素的连续 signed
  area / local coverage 累计。
- 遍历该 boundaryTile 的短边链表。每条短边对其影响的行只计算一次，写入
  线程本地 `float[16]`；中间空白区域可用前缀/填充方式处理，避免每个像素
  重复扫同一条边。
- 最终使用 path fill rule 把 signed area 转为 coverage，再写入
  `boundaryTile.coverage[0..255]`：
  - NonZero/Positive/Negative 使用连续 signed area 语义。
  - EvenOdd 使用之前确定的 triangle-wave 折叠，不退回 GRID/parity 采样。
- 如果 path rule 访问太远，可在 boundaryTile 中保存一份 rule 拷贝。

#### composite：ordered global tile color compositor

- 每个 global target tile 启动 256 个线程，即一个像素一个线程。
- 当前实现由 `capa_layer_plan.glsl` 按每个 global tile 命中的 path `tileRect`
  层数重新分配连续 `CAPAPathTile` span，`CAPAGlobalTile.head/count` 指向这段
  连续内存。layer plan 的 count 阶段只做 rect 命中统计；写入阶段再读
  `CAPASmallTile.value`、过滤 `CAPA_NIL`，并合并连续 SrcOver full tiles。
- 每个线程按这段 global tile path.tile span 顺序遍历 layers：
  - `boundaryTileIndex == CAPA_NIL`：不入链，或 coverage = 0。
  - `boundaryTileIndex == CAPA_FULL_TILE`：coverage = 1，可以直接走常量分支；也可以由
    CPU/初始化数据提供全 1 coverage，但最好保留显式判断。
  - 其它值：读取真实 `boundaryTile.coverage[pixelIndex]`。
- 根据 path paint 现场计算颜色。image paint 通过 CAPA pass 的 texture
  table 用 `textureIndex/samplerIndex` 访问；solid/gradient 根据 path paint
  参数求值。
- 第一版至少要保证 premultiplied `SrcOver` 正确：

```txt
src.rgb = premulPaint.rgb * coverage
src.a   = paint.a * coverage
dst     = src + dst * (1 - src.a)
```

- 其它 blend mode 可后续扩展，但这里是 CAPA 解决颜色漏光的关键位置，不能
  简化成普通相加，除非 blend mode 本身就是 plus。

CGAA 并行方向：

- 如果目标只是更好的单 path coverage 或面积积分质量，CGAA 仍然是更轻的
  路线。把 CGAA 从 GRID 采样进一步改成面积积分后，它可以保留 coverage
  atlas/mask 的轻量优势，不需要把颜色、渐变和图片全部纳入 ordered CAPA
  compositor。
- CGAA 可以研究更大的 `64x64` tile：面积积分版本不需要每个像素都重新扫边，
  可以让 64 个线程处理 64 行，并让每条边每行只计算一次。这可能降低 CPU
  tile 管理/binning 成本，同时保持 GPU 时间与 AASide 在同一量级附近。

CAPA tile 数据结构方向：

- 暂时不要在 CAPA 中引入 prefix scan / scan 作为主要短边分配机制。它在
  算法上漂亮，能生成紧凑连续数组，但一旦 tile 数超过单个 threadgroup，
  就需要 block scan、block sums scan、prefix add-back 等多 pass 结构，
  还会引入 threadgroup memory/barrier 和递归式边界处理。对当前 CAPA
  来说，这会显著增加实现和调试复杂度，性能也未必更好。
- 优先研究 tile-owned append storage：每个 tile 持有自己的 header，
  后续 pass 通过 tile index 访问该 tile 的 `offset/count/cursor`、
  inline edge 区和 overflow 状态。常见 tile 直接写固定小 inline 区；
  超过 inline 容量时再通过全局 atomic chunk allocator 分配 overflow
  chunk，并用 atomic exchange / compare-exchange 维护链表头。
- 当前首选结构是 per-tile initial chunk + overflow chunk list。每个 tile
  初始拥有一个 `CHUNK_CAP = 16` 的 chunk，绝大多数 tile 应该只命中这个
  初始空间；只有复杂 tile 才通过 `growLock` 扩容。这样 `atomicAdd` 主要
  发生在 tile 当前 chunk 的 `used` 字段，竞争范围被限制在单个 tile 内；
  全局 chunk allocator 只在溢出时触发。

建议的结构语义：

```cpp
const uint CHUNK_CAP = 16;
const uint NIL = 0xffffffffu;

struct CAPATile {
	uint head;
	uint growLock;
	uint flags;
	uint _pad;
};

struct CAPAChunk {
	uint next;
	uint used;
	ShortEdge edges[CHUNK_CAP];
};

struct CAPAChunkAllocator {
	uint count;
	uint overflow;
};
```

初始化策略：

```txt
每个 tile 预分配一个 initial chunk
tile.head = tileIndex
tile.growLock = 0
tile.flags = 0
chunks[tileIndex].next = NIL
chunks[tileIndex].used = 0
allocator.count = initialChunkCount
allocator.overflow = 0
```

append 伪代码：

```glsl
bool tryAppend(uint chunk, ShortEdge edge) {
	uint local = atomicAdd(chunks[chunk].used, 1u);
	if (local < CHUNK_CAP) {
		chunks[chunk].edges[local] = edge;
		return true;
	}
	return false;
}

bool appendTileEdge(uint tileIndex, ShortEdge edge) {
	uint head = tiles[tileIndex].head;
	if (tryAppend(head, edge))
		return true;

	for (uint attempt = 0u; attempt < MAX_RETRY; attempt++) {
		if (atomicCompSwap(tiles[tileIndex].growLock, 0u, 1u) == 0u) {
			uint freshHead = tiles[tileIndex].head;

			if (tryAppend(freshHead, edge)) {
				atomicExchange(tiles[tileIndex].growLock, 0u);
				return true;
			}

			uint newChunk = atomicAdd(allocator.count, 1u);
			if (newChunk >= MAX_CHUNKS) {
				atomicExchange(allocator.overflow, 1u);
				atomicExchange(tiles[tileIndex].growLock, 0u);
				return false;
			}

			chunks[newChunk].used = 1u;
			chunks[newChunk].edges[0] = edge;
			chunks[newChunk].next = freshHead;

			atomicExchange(tiles[tileIndex].head, newChunk);
			atomicExchange(tiles[tileIndex].growLock, 0u);
			return true;
		}
	}

	atomicExchange(tiles[tileIndex].flags, TILE_APPEND_FAILED);
	return false;
}
```

coverage 遍历伪代码：

```glsl
for (uint c = tiles[tileIndex].head; c != NIL; c = chunks[c].next) {
	uint n = min(chunks[c].used, CHUNK_CAP);
	for (uint i = 0u; i < n; i++) {
		ShortEdge edge = chunks[c].edges[i];
		...
	}
}
```

并发规则：

- `growLock` 只保护扩容路径；普通 append 不加锁。
- 拿到 `growLock` 后必须重新读取 `freshHead` 并再次 `tryAppend`，避免刚
  释放锁后重复扩容。
- `head` 发布顺序必须是：初始化 chunk 数据和 `next`，再
  `atomicExchange(tile.head, newChunk)`，最后释放 `growLock`。
- bin pass 只 append，coverage pass 在后续 pass 读取；不支持同一 pass
  边写边遍历。
- tile 内 edge 顺序不重要，因为 CAPA coverage 对 tile-local edges 做累加。

- 这个方向的目标是控制原子竞争，而不是完全避免原子。可接受的原子包括：
  per-edge 的 task/count 分配、per-tile append cursor、per-path/tile bounds
  的 `atomicMin/Max`，以及少量 overflow chunk 分配。应避免在 pixel/sample
  内层循环中使用原子。
- `capa_prepare` 可以顺手生成 surface-space edge、edge length/unit/dxdy、
  conservative path/screen bounds 和 clip 后 tile range。bounds 可用
  `atomicMin/Max` 维护；外层普通 `if` 预检查可以减少不必要的 atomic 调用，
  但 correctness 仍依赖 atomic 本身。
- clip/bounds 只能用于缩小无关 tile work，不能随意裁掉 x 扫描线左侧会
  影响 backdrop/winding 的边。左侧边可以不进 local edge list，但必须以
  backdrop/prefix 形式贡献状态，否则 tile 会把 inside/outside 判错。
- CAPA backdrop 必须保存连续 signed area 状态，而不是最终 alpha、离散
  GRID sample 或单纯 parity。tile 内局部边贡献与 backdrop 是线性相加：

```txt
signedArea(row, pixelX)
  = rowBackdropAtTileLeft[row]
  + localSignedAreaDelta(row, tileLeft -> pixelX)
```

  最终写 atlas 前才按 fill rule 把 `signedArea` 转成 coverage。不要提前
  clamp backdrop、tile delta 或 local area。
- 对当前 pixel 的局部边贡献是面积；对右侧 tile 的 row backdrop delta 是
  这个 row 内被边跨过的有向高度。例：左侧边给当前 row 贡献 `+0.5`
  backdrop；右侧一条反向边在当前 pixel 的 `x = 0.5` 处只跨过 `0.5` 个
  y 像素，则当前 pixel 的局部面积贡献是 `-0.25`，所以
  `0.5 + (-0.25) = 0.25`；但传给更右侧区域的 backdrop 变化是 `-0.5`，
  后续 backdrop 变为 `0.0`。
- 多重 winding/螺旋形状会让 signed backdrop 持续增加或减少，例如
  `0 -> 1 -> 2 -> 3` 再回落。中间状态必须完整保存为 signed float；只有
  最终 coverage 可以 clamp/fold。
- 四种 CGAA/CAPA fill rule 都可以基于连续 signed area 统一转换：

```glsl
float capa_fill_rule_coverage(float area, uint rule) {
	if (rule == kCGAANonZero_FillRule)
		return clamp(abs(area), 0.0, 1.0);

	if (rule == kCGAAEvenOdd_FillRule) {
		float m = mod(abs(area), 2.0);
		return 1.0 - abs(m - 1.0);
	}

	if (rule == kCGAAPositive_FillRule)
		return clamp(area, 0.0, 1.0);

	if (rule == kCGAANegative_FillRule)
		return clamp(-area, 0.0, 1.0);

	return clamp(abs(area), 0.0, 1.0);
}
```

  `kCGAAEvenOdd_FillRule` 不应退回离散 parity/GRID。它可以对连续
  `abs(area)` 做 triangle-wave folding：`0.5 -> 0.5`、`1.5 -> 0.5`、
  `2.0 -> 0.0`、`2.5 -> 0.5`。
- 动态线程数可以通过 GPU counters 驱动：prepare pass 用 `atomicAdd`
  维护真实 task count；如果需要 indirect dispatch，可用完成计数器让最后
  完成的 invocation 写 threadgroup 数。这个方案比按 `maxTaskCount` 暴力
  dispatch 更干净，但是否值得要用 capture 验证。
- 后续实现必须用 Xcode GPU Capture 验证：`CAPA short-edge bin` 是否仍
  保持低占比、`CAPA tile coverage` 是否从全路径 fallback 大幅下降、
  `allocator.overflow` / `TILE_APPEND_FAILED` 是否为 0，以及多数 tile 是否
  只使用 initial chunk。

需要记住的判断：

- 解析面积积分没有发现根本错误；当前质量问题更像算法取舍和采样表现，
  而不是公式明显算错。
- 若要模拟 GRID，CAPA 能近似做出来，但简单量化面积会出现跳变/抖动；
  CGAA 的 grid 波是连续移动的，这与实现细节、采样位置和 tile/backdrop
  组织有关。
- 未来如果要根治多图元边界、clip、blend 和 AA 统一问题，仍应走
  tile scene / ordered resolve 方向，而不是 per-path mask 后直接 alpha blend。

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
