# Compute AA 原型说明

本目录是 Quark 的 tile-based Compute AA 研究原型，目前用于验证：

- CPU 将路径曲线展平为有方向的直线边；
- CPU 使用 tile 级扫描线算法建立紧凑边列表和 backdrop 数据；
- Metal Compute Shader 使用固定子采样网格计算 winding coverage；
- coverage texture 最终合成到颜色纹理。

它目前仍是独立实验代码，但核心算法已经验证完成，可作为下一阶段接入 Quark
正式渲染模型的实现基线。

## 原型完成总结

当前 Compute GRID AA 原型已经完成从路径数据构建到 Metal 输出的完整闭环：

- CPU 将曲线展平为有方向边，并在一次 X-tile 扫描中建立局部边引用、backdrop
  event 与 boundary/uniform tile 分类；
- GPU 只对约 `11%` 的 boundary tile 执行昂贵 GRID coverage，uniform tile
  直接写入全 0 或全 1；
- 每条 Y sample 线程对每条有效边只求一次交点；
- 离散交点直接累加到 `crossingDelta[64]`，通过 `crossingMask + ctz()` 只遍历
  真实存在的 X 交点位置，并按区间一次生成完整 inside mask；
- 同一 X 的多个交点直接累计 winding，不需要排序、不需要清零完整 delta 表，
  也不存在交点事件数组溢出；
- 线程组通过共享 `insideMask` 和一次 barrier，将 `4x4` samples 合并成像素
  coverage；
- Uniform、Boundary Coverage 与 Composite 的语义和性能均已完成验证。

当前算法复杂度已经接近此 GRID/tile 结构下的最优形式：

```text
CPU：边实际跨越的 tile/sample 范围 + 最终输出记录数量
GPU Boundary 行：O(edgeCount + uniqueCrossingX)，uniqueCrossingX <= 64
```

已知性能结论：

- boundary/uniform 分类将稳定总 GPU 时间从 `0.733-0.773ms` 降至约
  `0.32-0.40ms`；
- 稀疏交点区间方向将 Boundary Coverage 占比从代表性的 `19.41%` 降至约
  `9%`；
- Composite 已占约 `78%`，成为绝对瓶颈；
- Compute AA 仍慢于约 `0.20ms` 的 AASide，但覆盖语义更通用、复杂路径质量
  更稳定，适合成为 Quark 的高质量 AA 路径；
- 不应继续把主要时间投入 Boundary kernel 微调，后续收益应来自正式架构接入、
  资源复用、批处理，以及减少完整 R8 coverage 写入/读取和独立 Composite。

下一步首先将 compute shader stage 接入现有 GLSL Native 构建脚本：

- 扩展 `tools/gen_glsl_natives.js`，当前脚本只识别 `#vert/#frag` 并生成
  vertex/fragment AST、GLSL 与 MSL native source；
- 为 compute shader 定义源码分段、stage 解析、入口命名和生成产物；
- 扩展 Metal native shader/pipeline 描述，使其可以表达 compute pipeline，
  而不仅是现有 vertex + fragment render pipeline；
- 将 Compute AA shader 从独立 `.metal` 实验文件迁移到正式 shader 源码与
  生成流程后，再开始接入 Quark 渲染命令、资源生命周期与批处理模型。

## 文件

- `mtl_compute_aa_prototype.h`
  - CPU / Metal 共享数据结构的 C++ 定义。
- `mtl_compute_aa_prototype.mm`
  - 路径展平、tile 数据构建、Metal buffer 上传与 compute 编码。
- `compute_aa_prototype.metal`
  - coverage compute kernel，以及普通 render composite vertex/fragment shader。
- `test_compute_aa.mm`
  - macOS Metal 动态测试窗口。
- `compute_aa_webgpu.html`
  - 较早的 WebGPU / CPU 对照实验，不保证与当前 Metal 数据结构完全同步。

## 核心直觉

普通扫描线填充会沿每条水平扫描线计算边的 X 交点，再根据 winding
判断采样点是否在图形内部。

本原型把这个思路放大到 tile：

- 一个 tile 是 `16x16` 像素；
- 每个像素在 Y 方向有 `kComputeAASampleGrid` 个 sample；
- CPU 不逐像素 rasterize，也不逐 sample 重新求边交点；
- CPU 沿 X tile 列推进一条边，只观察它是否跨过了新的离散 Y sample。

伪代码：

```text
for each edge:
    将左右端点按 X 排序
    lastSampleGridY = 左端点所在的离散 Y sample

    for each crossed X tile:
        sampleGridY = 边到达 tile 右边界时所在的离散 Y sample

        if sampleGridY == lastSampleGridY:
            当前 X tile 没有跨过任何 Y sample，什么都不做
        else:
            记录当前 tile 内需要精确测试的原始边与 sample 范围
            记录从下一 X tile 开始生效的 backdrop span
            lastSampleGridY = sampleGridY
```

因此，CPU 构建阶段处理的是“边跨过离散 Y sample 的变化”，不是生成最终
像素 coverage。

## 示例

为了容易观察，假设：

```text
tileSize = 10
sampleGrid = 2
edge = (0, 0) -> (15, 20)
slope = dy / dx = 20 / 15
```

边从 X=0 推进到第一个 tile 右边界 X=10：

```text
Y = 10 * 20 / 15 = 13.333...
```

转换到 Y sample-grid 后，比较：

```text
lastSampleGridY = edge 在 X=0 时的离散 sample
sampleGridY     = edge 在 X=10 时的离散 sample
```

两者之间的新 sample 范围会：

1. 在当前 `xTile=0` 实际跨过的 `yTile` 中添加 `ComputeAATileEdge`；
2. 添加一个从 `xTile=1` 开始生效的 `ComputeAABackdropEvent`。

然后继续推进到边终点 X=15。若某次推进后离散 `sampleGridY` 没有变化，
本次迭代不生成任何数据。

## 数据结构语义

### `ComputeAAEdge`

原始展平直线边。保存几何端点、CPU 预计算的 `dx/dy` 和 winding。

水平边不进入列表，因为水平扫描线算法不需要它们。

### `ComputeAATileEdge`

表示某个 tile 内仍需精确求交的原始边引用：

```text
edgeIndex
local sample 范围 [sampleBegin, sampleEnd)
```

同一条原始边可以被多个 X tile 引用，但每个引用只影响当前推进步骤新跨过的
Y sample。这样不会和已进入 backdrop 的 sample 重复计算 winding。

### `ComputeAABackdropEvent`

它不是 Metal 同步事件，只是一条紧凑的 backdrop span 数据：

```text
从 firstTileX 开始，
当前 yTile 内 [sampleBegin, sampleEnd) 的 backdrop 增加 winding。
```

CPU 只追加这条记录，不展开写入右侧所有 tile。

### `ComputeAABackdropRow`

描述一个 `yTile` 行拥有的 backdrop events 在连续数组中的切片。

### `ComputeAATile`

描述当前 tile 的局部边引用切片和像素原点。它不再保存完整 backdrop 数组。

## CPU 构建流程

`build_tile_edges()` 主要分为三部分：

1. **沿 X tile 扫描每条边**
   - 每条非竖边只计算一次 sample-grid slope；
   - 内层通过 `nextSampleY += tileSampleYStep` 推进；
   - 没有离散 Y sample 变化时不生成数据。

2. **记录局部边引用和 backdrop span**
   - `ComputeAATileEdge` 写入对应 tile 的临时桶；
   - `ComputeAABackdropEvent` 写入对应 yTile 行的临时桶；
   - 不在 CPU 展开 backdrop。

3. **压平临时桶**
   - 将 tile-edge 与 backdrop events 写入连续数组，便于直接上传 GPU。

当前 CPU 算法复杂度主要取决于：

```text
边实际跨越的 X tile 数
+ 发生离散 Y sample 变化时跨越的 yTile 数
+ 最终输出记录数量
```

## GPU Coverage 流程

一个 Metal threadgroup 对应一个 `16x16` tile，并使用
`16 * sampleGrid` 个线程。

### 1. 生成 sample inside mask

`16 * sampleGrid` 个线程各自负责一个 local Y sample 行：

```text
扫描当前 yTile 行的 backdrop events
累加 firstTileX <= 当前 tileX 的 winding
每条局部边只计算一次与当前 Y sample 的 X 交点
将交点 winding 累加到离散 X bucket，并用 64-bit crossing mask 标记非空位置
通过 ctz() 只遍历真实交点位置，按区间生成 inside mask
将 inside 结果压成一个 64-bit mask
```

所有线程写完自己的独占 mask 后执行一次：

```metal
threadgroup_barrier(mem_flags::mem_threadgroup);
```

这确保第二阶段读取其他 Y sample 行的 inside mask 时，所有 mask 都已经完成。
第一阶段中每个线程只写自己的私有 crossing delta 与独占 mask，不需要原子
操作或额外同步。

### 2. 合并像素 coverage

同一批线程以 `16 * sampleGrid` 为步长分担 tile 内全部 `256` 个像素。
在 `sampleGrid = 4` 时映射等价于：

```text
thread 0..15  -> pixel row 0..3,  每线程独占一个 pixel X
thread 16..31 -> pixel row 4..7
thread 32..47 -> pixel row 8..11
thread 48..63 -> pixel row 12..15
```

每个线程从 inside mask 中读取四个独立像素的 `4x4` samples，合并并写入
coverage texture。每个像素只由一个线程读取和写入，因此这一阶段不需要
原子操作、shuffle 或第二次 barrier。

`ComputeAATileEdge::sampleBegin/sampleEnd` 已经将边限制到当前 tile 内实际
跨过的离散 Y sample 范围，因此 coverage kernel 命中该范围后无需再次检查
原始边的 `[minY, maxY)`。边的 `dx/dy` 也由 CPU 每条边预计算一次，GPU
通过一次 FMA 求交，不再逐 sample 执行除法。

## 半开区间规则

Y 范围统一使用：

```text
[minY, maxY)
```

这避免共享顶点被相邻两条边重复计数。

连续 Y 坐标转换为离散 sample-grid Y 时，当前代码使用：

```cpp
int y = ceilf(sampleY - 0.5f);
```

它将连续位置映射到以半整数为中心的离散 sample 行，并维持共享顶点的半开
区间归属。

如果修改此规则，必须重新验证：

- 正好落在 sample 中心的端点；
- 正好落在 tile 边界的端点；
- 正负斜率；
- 共享路径顶点；
- 非零、奇偶、正 winding 和负 winding fill rule。

## 当前性能状态

相较最早的“边包围盒铺满 tile + 每 tile 重复求交 + CPU backdrop 前缀和”
版本，当前算法已经删除：

- 边 X/Y 包围盒的 tile 笛卡尔积写入；
- CPU `edge_crossing_at()` 重复求交；
- CPU 向右侧所有 tile 展开 backdrop；
- CPU backdrop X 前缀和；
- X tile 内层的除法、逐步乘法和 `ceilf/floorf`。

当前 CPU 实测热点主要是临时桶中的大量小 `Array` 分配、扩容和析构。
后续可考虑：

- frame arena / 线性内存池；
- 两遍计数后一次性分配连续数组；
- 缓存不变路径的构建结果；
- 复用 Metal buffer 和 coverage texture；
- 统计每个 yTile 行的 backdrop event 数，评估 GPU event 扫描成本。

### 实验分支总表

目前远端保存了 4 个 Compute AA 实验分支，都是后续选择方案时需要比较的
候选结构。

| 分支 | HEAD | Coverage / backdrop 结构 | 实测总 GPU 时间 | 状态 |
| --- | --- | --- | --- | --- |
| `experiment/compute-aa-row-mask` | `9bf955c3e` | GPU backdrop events；每 tile 64线程，每线程负责一条 Y sample；私有 `windingDelta[64]`；共享 `insideMask[64]` + 一次 barrier | `0.733-0.773ms` | 64线程 GPU-backdrop 基线 |
| `experiment/compute-aa-cpu-backdrop` | `8e8e2682a` | CPU 二维差分与前缀和生成完整 backdrop；每 tile 16线程，两个 tile 配成 SIMD32；每线程负责一行像素；无共享 mask/barrier | 约 `0.60ms` | 当前最快，但增加 CPU 构建与上传 |
| `experiment/compute-aa-gpu-backdrop-private-delta` | `db5b53d29` | 与 CPU-backdrop 分支相同的 16线程/双 tile 行累计结构，但每条 Y sample 在 GPU 扫描 backdrop events | `0.730-0.772ms` | 用于隔离 GPU backdrop 扫描成本 |
| `experiment/compute-aa-boundary-tiles` | 当前工作分支 | CPU 分类边界/纯色 tile；独立 Compute pass 写满纯色 0/1；64线程 GRID kernel 只 dispatch 边界 tile | `0.32-0.40ms` | 当前重大优化实验 |

`experiment/compute-aa-row-mask` 原来保存过全共享/全 compute pass 版本，现已
用更有价值的 64线程/private-delta 基线覆盖。全共享公平对比分支实测
`1.194-1.274ms` 后已经删除，结果继续保留在下方性能结论中。

当前选择结论：

- GPU 时间最快的是 CPU 完整 backdrop 分支，但它把压力转移到了 CPU 构建和
  完整 backdrop buffer 上传。
- 64线程/shared-mask 与 16线程/GPU-event 两个版本基本持平，说明只改变线程
  组织、shared mask 和 barrier 并不能突破当前瓶颈。
- 全共享 `windingDelta[64][64]` 路线已经确认不通过。
- 最终保留哪个方案仍未决定；后续实验结束后再从四个候选中选择。

### 已验证的 GPU 性能结论

目前测量均来自同一测试画面，具体数值会随机器和 capture 状态变化，但相对
趋势已经比较稳定：

- 最早使用独立 compute Clear / Coverage / Composite 的完整帧约
  `1.4-1.6ms`，其中 Coverage 通常占约 `60%-70%`；
- CPU 完整计算 backdrop 可以降低一部分 GPU 时间，但会增加 CPU 构建和上传
  成本，不能单独解决主要瓶颈；
- CPU backdrop + 线程私有 `windingDelta[64]` + render clear/composite 的实验
  总时间约 `0.60ms`；
- 相同画面的 AASide 总时间约 `0.20ms`，说明全 atlas 的固定 coverage 与
  composite 工作量仍然明显偏大。

已经验证的关键现象：

- 注释边交点计算后 Coverage 仍然很重，说明瓶颈不只来自边遍历；
- 初始化共享 `windingDelta[64][64]`、沿 X 做 winding 前缀和、生成和读取
  `insideMask` 都有显著成本；
- 完全删除 delta、反复扫描边寻找下一个交点，会把复杂度变成近似
  `edgeCount * crossingCount`，实测反而退化到约 `2.0-2.4ms`；
- 每 tile 仅使用 16 个线程会浪费 SIMD32 的一半执行槽；将两个 tile 配成
  32 个线程后有改善，但不能消除算法本身的固定工作量；
- 将 Clear 合并进普通 Composite render pass 是有效优化，删除了一个完整
  compute pass，但 Coverage 仍是主要成本。

当前最值得验证的架构方向不是继续微调一次 FMA 或一个分支，而是减少进入
Compute Coverage 的 tile 数量：只对边界 tile 做高精度 coverage，完全内部
区域由普通硬件光栅化或其他低成本路径填充，并尽量避免全 atlas composite。

共享 row-mask 公平对比分支的最新实测为：

```text
Compute AA GPU average: 1.194-1.274ms
Coverage: 72.25%
Composite: 27.75%
```

据此估算 Coverage 约为 `0.86-0.92ms`，Composite 约为 `0.33-0.35ms`。
共享 row-mask 的 Coverage 单项已经明显慢于 CPU backdrop + 私有 delta 分支
约 `0.60ms` 的完整帧。当时只能确认整个共享 row-mask 结构存在问题，不能
区分共享 delta、共享 mask、barrier 和 GPU backdrop 各自的成本。

随后只将共享 `windingDelta[64][64]` 改成每线程私有
`windingDelta[64]`，其余 GPU backdrop、共享 `insideMask[64]`、barrier 和
Composite 均保持不变，实测为：

```text
Compute AA GPU average: 0.733-0.773ms
Coverage: 65.61%
Composite: 34.39%
```

据此估算 Coverage 约为 `0.48-0.51ms`，比共享 delta 版本降低约 `44%-45%`；
完整帧降低约 `39%`。这确认共享二维 delta 是旧 Coverage 的主要瓶颈，而
GPU backdrop events 本身可以暂时保留。与 CPU backdrop + 私有 delta 的
约 `0.60ms` 相比，GPU backdrop 版本仍慢约 `0.13-0.17ms`，但避免了对应的
CPU 构建和上传成本。

随后尝试将共享 `insideMask[64]` 和 barrier 替换为线程私有 mask +
simdgroup shuffle，并保留全部 64 个 lane 参与第二阶段。实测完整帧退化到：

```text
Compute AA GPU average: 0.858-0.882ms
```

相比共享 mask + barrier 基线的 `0.733-0.773ms` 慢约 `14%-20%`。因此已撤回
shuffle 版本；不要仅为删除这块共享 mask 和 barrier 再次采用该结构。

## 下一阶段：解析面积 Coverage

当前固定 GRID 算法的计算量和 coverage 级数都随 `sampleGrid²` 增长：

```text
4x4   -> 16 samples  -> 17 个 coverage 值
16x16 -> 256 samples -> 257 个 coverage 值
```

因此继续提高 GRID 无法高效获得完整 8-bit 灰度。下一条主要质量研究路线是
解析面积 coverage：边穿过像素时计算连续的有符号梯形面积和 cover delta，
再沿 X 扫描得到 coverage。它不是 SDF，不计算到最近边的距离。

现有的扁平有向边、tile 分桶和扫描线 fill-rule 语义仍可复用；需要替换的是
离散 Y sample backdrop、`insideMask` 和固定子像素网格。解析版本的 incoming
backdrop/cover 将是连续行状态，边在一行内跨过多个 X cell 时会为每个 cell
产生局部面积贡献。

这条路线的目标是让高质量计算量主要取决于边实际穿过的像素/cell，而不是
全 atlas 像素数乘以 `sampleGrid²`。详细设计方向记录在
`docs/GPU_2D_ANTIALIASING.md` 的 “Analytic Area Compute Coverage Direction”。

## 重大优化方向：边界 Tile 与纯色 Tile

对当前 GRID 和未来解析面积 coverage 都成立的关键结论：

```text
没有有效边界穿过的连续 tile，必然整体为空或整体为实心。
```

不可能存在“没有边界穿过，但 tile 内一半 inside、一半 outside”的第三种
状态；inside/outside 的任何变化都必然需要跨过边界。

CPU 构建阶段需要按当前 coverage 算法的语义分类：

```text
boundary tile -> 需要精确 coverage
uniform outside tile -> R8 全部写 0
uniform inside tile  -> R8 全部写 1
```

R8 coverage texture 必须始终完整。不能让 outside tile 保持未初始化，也不能
让后续 Composite shader 额外理解“哪些 tile 有效”。后续可能有很多 paint /
composite shader 依赖这张 R8，完整的 0..1 coverage 语义必须保持简单稳定。

第一版正确执行模型是继续使用一个规则 Coverage compute pass：

```text
纯色 tile:
    快速写满 0 或 1

边界 tile:
    执行昂贵 GRID coverage
    或未来执行解析面积 coverage
```

纯色 tile 仍然写入 256 个 R8 texel，但不再遍历边、不求交、不扫描 GRID
samples，也不需要额外 clear pass、稀疏 Composite、普通 render shader 或
tile span 合并。只有边界 tile 执行昂贵算法。

这可能是当前 GRID 路线剩余最大的性能突破，也是解析面积 coverage 的重要
基础：未来连续面积计算同样只应运行于边界 tile，其他 tile 继续快速写完整
的 0/1 coverage。

### 当前边界 Tile 实验分支

分支：

```text
experiment/compute-aa-boundary-tiles
```

它从 64线程/private-delta/shared-mask 基线创建，当前实现：

- boundary 标记合并进原有 CPU X-tile 扫描，不再建立第二套几何裁剪/DDA
  数据流程；
- 水平边使用零 winding，只参与 boundary 标记，不生成 coverage edge 或
  backdrop；
- CPU 只输出紧凑的 boundary tile 数据，不再输出全量 tile 数组和二次索引列表；
- CPU 只使用第一个 Y sample 的 backdrop 为无边界 tile 计算整数 winding；
- 独立 `Compute AA Uniform Tiles` pass 为无边界 tile 写满 0 或 1；
- `Compute AA Boundary Coverage` pass 只 dispatch boundary tile，内部继续使用
  原 64线程 GRID coverage；
- Composite 仍读取语义完整的整张 R8 coverage texture。

窗口标题会显示 `boundary` / `uniform` tile 数量。当前大尺寸测试通常约为：

```text
edges:324
boundary:501-506
uniform:3983-3987
```

即只有约 `11%` 的 tile 执行昂贵 GRID coverage。禁用 uniform pass 后，可直接
看到沿路径轮廓连续分布的 boundary tile 带；这验证了昂贵工作已经从“整个
atlas 面积”转为主要跟随“路径轮廓”。

注意：不要在 CPU 性能测试期间每帧更新原生窗口标题。旋转路径会改变上述统计
数字，`NSWindow.title` 会触发 AppKit 标题栏布局、绘制和 Window Server 通信，
其累计成本可能远高于 `buildDrawData()`，并在 Instruments 中显示为
`-[NSWindow _dosetTitle:andDefeatWrap:]`。不同 boundary 算法的计数稳定性不同，
因此这个 UI 副作用还会制造假的 A/B 性能差异。正式测量时应禁用标题更新，
把统计保存在内存中并在测量结束后只输出一次。

两类 boundary 标记数据不完全相同：

- 按离散 GRID sample 变化标记时，只有影响当前采样点的 tile 才算 boundary；
  浅斜边和 sample 阈值变化会让数量随旋转频繁跳动。
- `mark_boundary_range` 按连续几何边界经过的 tile 标记，包括水平边和没有改变
  离散 sample 行的边，因此更保守，数量通常也更稳定。

### 边界 Tile 里程碑测量

原始 64线程/all-tile GPU backdrop 基线：

```text
Compute AA GPU average: 0.733-0.773ms
```

boundary/uniform 分流后的稳定测量：

```text
Compute AA GPU average: 0.32-0.40ms
```

启动或短暂运行时曾测得 `0.23-0.24ms`，但会回到上述稳定区间，因此不把低值
作为最终性能结论。即便按稳定结果计算，该优化仍将完整帧 GPU 时间降低约一半。

GPU Capture 的代表性 `4x4 GRID` 占比：

```text
Uniform Tiles:     9.79%
Boundary Coverage: 19.41%
Composite:         70.80%
```

改为 `1x1 GRID` 后总时间仍接近，说明当前主要瓶颈已经不是 boundary GRID
算术，而是完整 R8 atlas 的写入、Composite 采样和最终颜色写入。Xcode 会把
两个 compute pass 中先执行的 pass 标为 `Unused Texture`；这是因为它只看到
后续 pass 再次写同一张 texture，无法识别两者写入的是互不重叠的 tile。

### 稀疏交点分桶 / 区间填充实验

`experiment/compute-aa-sorted-crossings` 保持 CPU 数据、Uniform pass、线程组
结构和 Composite 不变，只替换 Boundary Coverage 内部生成行 mask 的算法：

- 每条 Y sample 线程仍扫描当前 tile 的活跃边并计算离散 X 交点；
- 不再立即写入 `windingDelta[x]`；
- 交点直接累加到每线程私有的 `short crossingDelta[64]` 对应 X bucket；
- `ulong crossingMask` 标记非空 bucket，同一 X 的多个交点直接累计 winding；
- 通过 `ctz(crossingMask)` 按 X 升序访问真实存在的 bucket；
- 根据相邻交点形成的区间一次生成 64-bit inside mask。

这个版本无需清零整个 delta 表、无需固定扫描 64 个 X sample，也无需排序或
交点数量上限。复杂度为 `O(edgeCount + uniqueCrossingX)`，其中
`uniqueCrossingX <= 64`。私有数组仍为 128B。

实测交点区间版本收益明显，`Compute AA Boundary Coverage` 时间约砍掉一半。
插入排序版本的代表性 GPU Capture 占比：

```text
Uniform Tiles:     12.32%
Boundary Coverage:  9.44%
Composite:         78.24%
```

当前稀疏分桶版本实测 Boundary Coverage 约为 `9.19%`，与插入排序版本基本
相当，但它没有排序成本和交点列表溢出风险，因此选作正式接入基线。此前代表性
Boundary Coverage 占比为 `19.41%`。这表明当前测试中，大多数 boundary
Y-sample 行的有效交点位置远少于固定的 64 个 X sample；按实际非空位置填充
区间，比每行清零完整 delta 表并执行固定长度前缀扫描更便宜。

该实验确认 Compute GRID AA 已具备加入 Quark 渲染模型的技术基础，但它仍以
更高 CPU/GPU 成本换取比 AASide 更稳定的高质量 coverage。下一项最重要的
性能实验不是继续微调 GRID，而是让 boundary/uniform coverage 在最终绘制中
直接生效，尝试删除完整 R8 coverage 写入与独立 Composite。

## 重要不变量

- CPU 与 Metal 中的 tile size、sample grid 和结构体布局必须严格一致。
- `ComputeAATileEdge` 的 sample 范围是 tile 内局部范围。
- `ComputeAABackdropEvent` 从 `firstTileX` 开始生效，包含该 tile。
- backdrop event 与 tile-edge 必须刚好覆盖同一批新跨过的 Y sample。
- coverage kernel 使用固定 `16 * sampleGrid` 个线程；每个 threadgroup 对应一个完整 `16x16` tile。
- barrier 前不得按像素越界提前 return。
