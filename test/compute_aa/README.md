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
  - coverage compute kernel，以及 coverage 纹理合成用的普通 vertex/fragment shader。
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
2. 在对应 yTile 行记录一个从 `xTile=1` 开始生效的 backdrop 二维差分。

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

### `backdrops`

CPU 为每个 tile 的每条 local Y sample 生成一个最终 backdrop winding。
数据按 tile-major 顺序连续存放，GPU 每个线程只需读取一个值。

### `ComputeAATile`

描述当前 tile 的局部边引用切片和像素原点。

## CPU 构建流程

`build_tile_edges()` 主要分为三部分：

1. **沿 X tile 扫描每条边**
   - 每条非竖边只计算一次 sample-grid slope；
   - 内层通过 `nextSampleY += tileSampleYStep` 推进；
   - 没有离散 Y sample 变化时不生成数据。

2. **记录局部边引用和 backdrop 二维差分**
   - `ComputeAATileEdge` 写入对应 tile 的临时桶；
   - backdrop range 只在 `firstTileX` 写入 Y range 的两个差分端点。

3. **CPU backdrop 前缀和与压平**
   - 每个 yTile 行先沿 local Y、再沿 X 对二维差分做前缀和；
   - 将每个 tile 的最终 backdrop 和 tile-edge 写入连续数组。

当前 CPU 算法复杂度主要取决于：

```text
边实际跨越的 X tile 数
+ 发生离散 Y sample 变化时跨越的 yTile 数
+ 最终输出记录数量
```

## GPU Coverage 流程

一个 Metal threadgroup 使用 `32` 个线程，同时处理横向相邻的两个
`16x16` tile。前后各 `16` 个线程分别处理一个 tile。

### 1. 计算整行 coverage

每个线程负责 tile 内一整行的 16 个像素：

```text
依次处理该像素行的 sampleGrid 条 Y sample 行
从 CPU 预计算 backdrop 开始
使用线程私有 delta 记录当前 Y sample 行的交点
沿 X 做 winding 前缀
直接累计 16 个像素的 coverage
```

该版本不使用 threadgroup `windingDelta`、`insideMask`、原子操作或 barrier。
每个线程只保留当前 Y sample 行的私有 delta，处理下一条 sample 行时复用。

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

相较最早的“边包围盒铺满 tile + 每 tile 重复求交”
版本，当前算法已经删除：

- 边 X/Y 包围盒的 tile 笛卡尔积写入；
- CPU `edge_crossing_at()` 重复求交；
- 每条边向右侧所有 tile 逐项展开 backdrop；
- X tile 内层的除法、逐步乘法和 `ceilf/floorf`。

当前 CPU 实测热点主要是临时桶中的大量小 `Array` 分配、扩容和析构。
后续可考虑：

- frame arena / 线性内存池；
- 两遍计数后一次性分配连续数组；
- 缓存不变路径的构建结果；
- 复用 Metal buffer 和 coverage texture；
- 对比 CPU backdrop 展开增加的上传量与 GPU event 扫描成本。

### 当前实验与测量结论

当前未提交实验位于 `experiment/compute-aa-cpu-backdrop`，基于已提交并推送的：

```text
experiment/compute-aa-row-mask
commit 706ec42bc
```

测试场景约为 `1039x1085` atlas、`4420` tiles、`4x4` samples。实测结论：

- GPU 扫描 backdrop events 的早期版本总时间约 `1.4-1.6ms`。
- CPU 完整预计算 backdrop 后约 `1.11-1.30ms`，但 CPU 时间和上传量增加。
- 删除 delta 后反复扫描全部 edges 寻找下一个 X 交点，退化到约
  `2.0-2.4ms`，原因是复杂度接近 `edgeCount * crossingCount`。
- 每 tile 只有 16 个线程会浪费半个 SIMD32；一个 SIMD32 同时处理两个相邻
  tile 后，时间明显改善。
- 当前线程私有 delta 版本不需要 threadgroup 数组、`insideMask` 或 barrier。
- compute Clear 与 compute Composite 已删除。当前 Composite render pass 使用
  `MTLLoadActionClear`，同时完成清屏、coverage 采样和硬件 alpha blending。
- 最新总 GPU 时间约 `0.60ms`，Xcode 中 Coverage 约占 `62%`，Composite
  约占 `38%`。
- 同场景 AASide 约 `0.20ms`。全 atlas Compute AA 即使继续微调，也很难追上
  直接硬件光栅化。

### 已否定或低收益方向

- 为 Even-Odd 单独写快速路径：不是主要使用场景，不优先。
- CPU 为每条 Y sample 生成并排序 X 交点：会接近 CPU 扫描线 rasterizer，
  且不能解决 coverage atlas 写入与合成成本。
- 仅优化边交点 FMA：交点计算不是主要热点。
- 删除所有 delta 数据：会迫使 GPU 重复扫描 edges 查找交点，性能更差。

### 下个会话优先方向

不要继续围绕全 atlas kernel 做小幅微调。优先验证：

1. CPU/GPU 将 tile 分类为 exterior、solid interior、edge tile；
2. 只 dispatch edge tiles 执行精确 `4x4` Compute AA；
3. exterior tile 完全跳过；
4. solid interior 使用普通硬件填充，而不是写入再采样 coverage atlas；
5. 评估 coverage texture/buffer 复用与静态路径缓存。

## 重要不变量

- CPU 与 Metal 中的 tile size、sample grid 和结构体布局必须严格一致。
- `ComputeAATileEdge` 的 sample 范围是 tile 内局部范围。
- CPU backdrop 二维差分从 `firstTileX` 开始生效，包含该 tile。
- backdrop 差分与 tile-edge 必须刚好覆盖同一批新跨过的 Y sample。
- coverage kernel 使用固定 `32` 个线程；每个 threadgroup 对应横向相邻的两个 `16x16` tile。
