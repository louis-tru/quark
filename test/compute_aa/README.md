# Compute AA 原型说明

本目录是 Quark 的 tile-based Compute AA 研究原型，目前用于验证：

- CPU 将路径曲线展平为有方向的直线边；
- CPU 使用 tile 级扫描线算法建立紧凑边列表和 backdrop 数据；
- Metal Compute Shader 使用固定子采样网格计算 winding coverage；
- coverage texture 最终合成到颜色纹理。

它不是生产渲染器的一部分，数据结构和算法仍可能继续变化。

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

一个 Metal threadgroup 使用 32 个线程，同时处理横向相邻的两个 `16x16`
tile。每个 tile 使用 16 个线程，每线程负责一整行像素。

### 1. 累计像素行 coverage

每个线程依次处理当前像素行的 `sampleGrid` 条 local Y sample：

```text
for each local Y sample:
    扫描当前 yTile 行的 backdrop events，得到初始 winding
    遍历 tile edges，生成线程私有 winding delta
    沿 64 个 X samples 做 winding 前缀和
    将 inside sample 累加到当前像素行的 16 个 coverage 计数
```

横向相邻的两个 tile 合并为一个 SIMD32 threadgroup，避免单个 tile 的
16 线程浪费一半 SIMD32 执行槽：

```text
thread 0..15  -> 左侧 tile 的 pixel row 0..15
thread 16..31 -> 右侧 tile 的 pixel row 0..15
```

delta 与 coverage 计数均为线程私有数据，不使用 threadgroup memory、
barrier、原子操作或 shuffle。与约 `0.60ms` 的 CPU-backdrop 快速版本相比，
当前实验只将每条 Y sample 的初始 winding 改为扫描 GPU backdrop events。

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

### 当前公平对比分支

旧共享 row-mask 公平对比分支：

```text
experiment/compute-aa-row-mask-render-composite
```

它用于重新测量共享 row-mask Coverage kernel 的纯成本：

- `sampleGrid = 4`；
- GPU 扫描 backdrop events；
- 每个 Y sample 线程遍历一次 tile edges；
- 使用 threadgroup `windingDelta[64][64]`；
- 使用 `insideMask[64]` 和一次 barrier；
- 不使用 CPU 完整 backdrop；
- 不使用线程私有 delta；
- 删除 compute Clear 与 compute Composite；
- 普通 Composite render pass 使用 `MTLLoadActionClear`，在同一个 pass 中
  清屏、采样 coverage，并通过固定功能 premultiplied-alpha blending 合成。

因此该分支与 `experiment/compute-aa-cpu-backdrop` 的总 GPU 时间可以公平比较，
差异主要来自 Coverage/backdrop/delta 结构，而不是额外 Clear/Composite pass。

相关保存点与当前实验：

```text
experiment/compute-aa-row-mask    已更新为 private-delta/shared-mask 基线
experiment/compute-aa-cpu-backdrop 8e8e2682a
experiment/compute-aa-gpu-backdrop-private-delta 当前工作树为 16线程/双tile + GPU backdrop
```

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

当前版本采用与 CPU-backdrop 快速版本相同的 16线程/双 tile 行累计
结构，但继续由 GPU backdrop events 计算初始 winding。它用于判断
`0.733-0.773ms` 基线与约 `0.60ms` CPU-backdrop 版本之间的差距，有多少来自
row mask/barrier，又有多少来自 GPU backdrop 扫描。

该版本实测为：

```text
Compute AA GPU average: 0.730-0.772ms
```

与 64线程/shared-mask 基线的 `0.733-0.773ms` 没有明显差异。16线程版本删除
了 shared mask 和 barrier，但每个线程串行处理 4 条 Y sample 行，两部分
成本基本互相抵消。与约 `0.60ms` CPU-backdrop 版本仍存在的差距，进一步
说明 GPU backdrop event 扫描是主要差异候选。

## 重要不变量

- CPU 与 Metal 中的 tile size、sample grid 和结构体布局必须严格一致。
- `ComputeAATileEdge` 的 sample 范围是 tile 内局部范围。
- `ComputeAABackdropEvent` 从 `firstTileX` 开始生效，包含该 tile。
- backdrop event 与 tile-edge 必须刚好覆盖同一批新跨过的 Y sample。
- coverage kernel 使用固定 32 个线程；每个 threadgroup 对应横向相邻的两个 `16x16` tile。
- 奇数 tile 列的最后一个 threadgroup 中，右侧 tile 的 16 个线程会提前 return。
