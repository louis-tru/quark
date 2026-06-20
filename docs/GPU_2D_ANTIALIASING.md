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

### 浏览器 SVG / CAPA / CGAA / AASide 对比观察

`tools/capa_svg_compare.html` 用同一个星形 + 双三次曲线路径对比了浏览器 SVG transform、SVG 几何重绘、Canvas cached bitmap 和 Canvas vector redraw。当前观察结论：

- 浏览器的静态 SVG transform 很可能不是每帧重新做矢量 coverage，而是先生成一张中间纹理/layer，再对这张纹理做 transform。小图时它的锯齿最少，但边缘细节不够干净；放大后锯齿会回来，表现接近动态几何绘制。
- Canvas cached bitmap 在修正 canvas 尺寸比例后，和浏览器 SVG transform 的观感非常接近。它的“小图更平滑”更像高分辨率图像下采样 / mipmap / linear filter 抹掉了原始锯齿细节，而不是更准确的矢量 AA。
- 浏览器动态 SVG geometry 和 Canvas vector redraw 更像 GRID 4x4 或 2x2 技术。它们的锯齿波是离散采样造成的“跳格/撕裂”感，和 Qk 开启 CGAA 时的动态观感相近。
- CAPA 当前解析面积积分与 AASide 在单条直线边上的质量非常接近。这说明 AASide 的距离 ramp 对单边场景已经近似一维面积积分；CAPA 的价值主要在多边、多轮廓、backdrop、tile-local 统一 resolve，而不是让一条旋转直线边神奇超过 AASide。
- CAPA / AASide 的小角度直线旋转会出现非常均匀、柔和、连续移动的低频波纹；CGAA / GRID 也有波动，但更像离散采样锯齿在移动。两者都不是免费午餐，只是在“平滑波纹”和“离散锯齿”之间选择不同代价。
- CAPA 中用 point-sample 方式模拟 2x2 / 4x4 GRID 时，动态锯齿可能出现突然跳位；这不等同于 CGAA 当前的 tile/sample 组织。真正要模拟 CGAA，需要按它的 sample grid、tile backdrop、边穿越和覆盖 resolve 语义来做，而不是只在解析面积 pass 中替换为若干点采样。

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

### Compute area pipeline Anti-aliasing / CAPA

把路径边分配到 tile，在 compute shader 中计算 tile 内 coverage。

这是当前最值得研究的 fill 方向，但要注意：

- 它不是单个 shader 可以解决的问题，而是完整的路径编译、binning、backdrop、tile resolve 管线。
- 如果仍然每个 path 独立生成 mask，再普通 alpha blend，边界漏光仍会存在。
- 性能关键在于减少 CPU/GPU 同步、减少全局原子、减少临时 buffer、控制每 tile 工作量。

当前 `CGAA` 可以视为 compute coverage AA 的原型名称。下一代重构建议命名为 `CAPA`：Compute Area Pipeline Anti-aliasing。这个名字强调两个重点：

- `Area`：coverage 来自面积积分，而不是启发式软边。
- `Pipeline`：它不是单个 shader，而是一条包含 transform、count、scan、binning、backdrop、coverage、tile resolve 的 GPU 管线。

### Qk 下一步 CAPA 管线草案

当前讨论后的判断：Qk 不必急着把 path flatten 搬到 GPU。路径本身通常变化不频繁，变化更多来自 offset、matrix、surface scale、clip 等状态。CPU flatten 已经有良好缓存命中，计算量也不大，类似当前 AASide 缓存路线。因此下一阶段可以先保留 CPU 展平，把 GPU 工作集中在 transform、binning、backdrop、tile coverage 和 tile resolve 上。

CAPA 第一版先不追求完全均衡地展开不同长度的原始边。可以先让原始边线程循环写轻量 short-edge task，把真正重的 tile/bin/area 工作放到后续按短边或 tile 调度的 pass 中。这个取舍更接近 Pathfinder/Vello 的真实工程路线：先把动态数据流和面积积分管线跑通，再用 profiler 判断是否需要把某些 atomic/task 生成点替换成 count + scan。

CAPA v1 先按 6 个阶段设计，其中 `pass 0` 在 CPU，后面 5 个是 GPU pass：

当前仓库里的可运行原型已经进入第三个中间状态：CPU 只保留路径展平和绘制命令编码，上传 path-space edge list、path matrix/color 和 edge range；GPU 的 `capa_prepare` 负责变换到 surface-space 并生成 short-edge task，`capa_bin` 把 short-edge task 分配到固定容量 per-tile list，`capa_tile` 目前把这份 list 主要作为 boundary-tile 分类器：无本地边的 tile 走 row-area backdrop 快路径，有本地边的 boundary tile 暂时回退到 full-edge area kernel，`capa_resolve` 把 coverage atlas 写回目标。这个版本已经移除了 CPU surface-space edge transform 和 CPU tile 分桶，但还没有 count + scan tile allocation、backdrop prefix、正确的 tile-local short-edge area resolve 或 tile 内多 draw 有序 resolve；它仍然不是最终性能形态。

```text
pass 0 / CPU 编码
  输入：
    路径、样式、矩阵、绘制顺序
  输出：
    展平后的原始边 buffer
    绘制元数据 buffer
  工作：
    在 CPU 展平 path
    缓存展平后的直线边
    上传 edge list、path id、draw id、矩阵、样式、clip 元数据

pass 1 / 原始边准备与短边任务生成
  输入：
    展平后的原始边 buffer
    绘制元数据 buffer
  输出：
    变换后的边 / 短边任务 buffer
    path tile bounds
    短边任务数量
  工作：
    每个线程处理一条原始边
    用 draw matrix 变换 p0 / p1
    计算变换后的边长度
    计算需要生成多少条短边 shortEdgeCount
    计算局部 bounds
    计算 winding / backdrop 贡献
    在 GPU 上归约/修正 path 在 tile 空间中的 bounds
    使用 atomic / bump 分配短边任务区间
    循环写入 short-edge task

pass 2 / 短边分配到 tile
  输入：
    短边任务 buffer
    变换后的边 buffer
    path tile bounds
  输出：
    tile-edge 引用 buffer
    tile backdrop delta buffer
    tile-edge 数量或范围
  工作：
    每个线程处理一个 short-edge task
    构造实际短边
    计算短边影响的 tile 或 tile 范围
    写入 tile-edge 引用
    写入 tile backdrop delta

pass 3 / tile backdrop 传播
  输入：
    tile backdrop delta buffer
    path tile bounds
  输出：
    tile 初始 winding / backdrop buffer
  工作：
    在每个 path 的 tile 范围内传播 / prefix backdrop delta
    让每个 tile 得到面积积分所需的初始 winding / backdrop

pass 4 / tile 面积覆盖计算
  输入：
    tile-edge 引用 buffer
    变换后的边 buffer
    tile 初始 winding / backdrop buffer
  输出：
    tile coverage / mask buffer，或 tile-local coverage command
  工作：
    每个 workgroup 处理一个 tile 或一组 tile
    读取 tile-local edges
    做面积积分，计算 coverage

pass 5 / tile 颜色合成
  输入：
    tile coverage / mask buffer
    绘制元数据 buffer
    有序 tile draw list
  输出：
    最终目标像素
  工作：
    v1 先只处理单纯色填充
    在同一个 framebuffer tile 内按绘制顺序 resolve 多个 draw layer
    一次性计算最终 color / coverage / blend
```

其中 path tile bounds 是必须落实的细节。它用于定位当前 path 在 tile 空间中的起始位置，也用于限制 backdrop prefix 的传播范围。当前方向是让 GPU pass 1 在变换原始边时同步产生 bounds 信息，后续通过 atomic min/max 或 count + scan/reduction 得到 path tile bounds；CPU 不再根据 path bounds 和 draw matrix 估算保守 tile 范围。这样 CPU 端只保留路径展平和绘制命令上传。

backdrop 在 CAPA 中也需要重新定义。旧 CGAA 中 backdrop 更接近离散 winding 初值，方便判断纯色 tile；但 CAPA 做面积解析后，不能只依赖“离散纯色 tile”概念。CAPA 的 backdrop 应先理解为 tile 面积积分的初始 winding / 参考状态：它告诉 tile 在没有局部边贡献前的填充状态，tile 内真实 coverage 仍由 tile-local edges 的面积贡献共同决定。

Pathfinder 和 Vello 在面积覆盖上可以作为参考：

- Pathfinder 的 `fill.cs.glsl` 从 tile 的 backdrop 开始，把 fill list 中每条 line segment 的面积贡献累加进去，再按 fill rule 把 coverage 限制到 `[0, 1]`。它的 `fill_area.inc.glsl` 使用 area LUT 计算单条线段对像素的覆盖贡献。
- Vello 的 `fine.wgsl::fill_path` 更直接：`area[i] = backdrop`，随后遍历 tile-local segments，对每个像素累加有符号面积贡献，最后 even-odd 用周期折叠，non-zero 用 `min(abs(area), 1)`。
- 这两个项目都说明：backdrop 不是最终 coverage，而是面积积分的初始项；最终 coverage 必须由 backdrop 加上 tile-local edge/segment 贡献共同决定。
- 多条边影响同一个像素时，不应分别 alpha blend。应在同一个 tile pass 内先把这些边的面积贡献累加到同一个 `area`，再统一按 fill rule 得到 coverage。

本地可直接对照的源码位置：

```text
../vello/vello_shaders/src/cpu/fine.rs
  fill_path():
    area[] 先全部初始化为 fill.backdrop
    遍历 tile-local PathSegment
    对每个像素 area += y_edge + a * dy
    最后按 even-odd / non-zero 规则 resolve

../vello/vello_shaders/shader/fine.wgsl
  fill_path_ms():
    MSAA 路线更复杂
    使用 workgroup shared winding 数组
    先统计 segment 触达的采样/像素数量
    workgroup 内 scan + 二分把实际工作分配给线程
    再做 x/y prefix 得到每个采样点 winding

../vello/vello_shaders/src/cpu/path_count.rs
  对 line 做 tile 划分
  写 tile.backdrop delta
  写 SegmentCount task

../vello/vello_encoding/src/path.rs
  Tile.backdrop 表示 tile 左边界的累计 backdrop

../pathfinder/shaders/d3d11/fill.cs.glsl
  coverages = vec4(backdrop)
  coverages += accumulateCoverageForFillList(...)
  最后 abs/clamp 并写入 coverage tile

../pathfinder/shaders/fill_area.inc.glsl
  computeCoverage(from, to, areaLUT)
  对单条线段使用 area LUT 近似像素覆盖面积

../pathfinder/shaders/d3d11/bin.cs.glsl
  addFill() 把 tile-local line 写入 fill list
  adjustBackdrop() 写入 backdrop delta
```

因此 CAPA 的像素面积核心不应该是“边产生一个 alpha 然后混合一次”，而应该是：

```text
float area[16][16] = initialBackdrop

for edge in tileLocalEdges:
  for pixel touched by edge:
    area[pixel] += signedAnalyticArea(edge, pixel)

for pixel:
  coverage[pixel] = resolveFillRule(area[pixel])
```

这样同一个像素内有多条边时，所有边先合并为一个面积/winding 结果，再输出一次 coverage；这也是它能避免很多 per-edge/per-path AA 叠加错误的关键。

CAPA v1 的单色 fill 面积计算可以先参考 Vello 的解析形式：

```text
area = initialBackdrop
for edge in tileLocalEdges:
  area += signedAreaContribution(edge, pixel)
coverage = resolveFillRule(area)
color = premulSolidColor * coverage
```

后续如果解析公式在某些边界上不稳定，可以再参考 Pathfinder 的 area LUT 路线。

CAPA v1 的原则：

- pass 1 可以使用 atomic/bump 分配 short-edge task，先避免额外 count/scan pass。
- pass 1 允许原始边线程循环写轻量 task，但不要在这里做 tile coverage 重活。
- pass 2 之后尽量按 short edge、tile、tile command 调度，让昂贵工作更均衡。
- 如果 pass 1 或 pass 2 的动态分配成为瓶颈，再把对应位置替换成 count + prefix-sum。
- v1 只先支持纯色 fill；渐变、图像、复杂 clip 后放。

backdrop 也不应让每个 tile raster 时反复向左查询到 `xtile_0`。更好的结构是：

```text
bin 阶段:
  边跨 tile 时写 backdrop delta

backdrop 阶段:
  对 tile 行/列做 prefix/propagate
  每个 tile 得到自己的 initial backdrop

coverage 阶段:
  tile 只读自己的 initial backdrop + local edge list
```

这样 tile raster 才是局部工作。

多图元漏光问题则要求 tile 中不能只存边，还要保留绘制命令信息：

```text
tile command:
  draw id
  path id
  paint/style
  clip state
  blend mode
  edge list or coverage source
```

最终目标是同一个 framebuffer tile 内按 draw order 统一 resolve coverage/color，而不是 path A 独立 AA 到 framebuffer 后 path B 再独立 AA。这个结构才是解决不同颜色、多轮廓、border/fill 共享边界漏光的根本方向。

CAPA v1 对颜色合成的边界：

- 单个纯色 fill 是第一目标。
- 同色图元在同一个 tile 内比较容易合并，可以把 coverage 合并后按同一颜色输出。
- 不同颜色不能简单相加，因为还要保留绘制顺序、alpha、blend mode。
- 不同颜色共享边界的漏光问题，最终要靠 tile 内 ordered resolve 或 coverage ownership 解决；v1 先验证管线是否具备消除漏光的结构基础。
- 渐变、图像、复杂 clip 暂不进入第一版，避免 paint/clip 复杂度掩盖面积积分和 tile pipeline 本身的问题。

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
