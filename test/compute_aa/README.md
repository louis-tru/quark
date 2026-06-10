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
  - coverage、清屏和纯色合成 kernel。
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

原始展平直线边。保存几何端点、Y 半开范围 `[minY, maxY)` 和 winding。

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

一个 Metal threadgroup 对应一个 `16x16` tile。

### 1. 解析 backdrop

前 `16 * sampleGrid` 个线程各自负责一个 local Y sample：

```text
扫描当前 yTile 行的 backdrop events
累加 firstTileX <= 当前 tileX 的 winding
写入 threadgroup backdrop 数组
```

完成后执行：

```metal
threadgroup_barrier(mem_flags::mem_threadgroup);
```

这确保 tile 内所有像素线程读取 backdrop 前，共享数据已经完成初始化。
所有线程必须先到达 barrier，越界线程只能在 barrier 后退出。

### 2. 计算像素 coverage

每个像素线程遍历固定 sample 网格：

1. 从 threadgroup backdrop 取得 tile 左边界 winding；
2. 遍历当前 tile 的 `ComputeAATileEdge`；
3. 只有当前 sample 位于 tile-edge 的有效 sample 范围内时，才测试原始边交点；
4. 根据 fill rule 判断 sample 是否在图形内；
5. 汇总为最终 coverage。

## 半开区间规则

Y 范围统一使用：

```text
[minY, maxY)
```

这避免共享顶点被相邻两条边重复计数。

连续 Y 坐标转换为离散 sample-grid Y 时，当前代码使用：

```cpp
int y = int(max(sampleY, 0) + 0.499999f);
```

它是在非负 sample-grid 坐标下对 `ceilf(sampleY - 0.5f)` 的快速近似。
轻微负偏置用于维持 sample 中心边界的半开区间归属，并吸收很小的正向浮点误差。

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

## 重要不变量

- CPU 与 Metal 中的 tile size、sample grid 和结构体布局必须严格一致。
- `ComputeAATileEdge` 的 sample 范围是 tile 内局部范围。
- `ComputeAABackdropEvent` 从 `firstTileX` 开始生效，包含该 tile。
- backdrop event 与 tile-edge 必须刚好覆盖同一批新跨过的 Y sample。
- coverage kernel 使用固定 `16x16` threadgroup；不要随意改为不完整边缘 threadgroup。
- barrier 前不得按像素越界提前 return。
