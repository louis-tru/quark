# CAPA PASS PROCESS

本文记录当前 `src/render/shader/capa/` 目录里的 CAPA pass 事实源。不要再按旧的
7-pass 记忆接 Metal；当前目录中有 10 个 shader pass，且多个后续 pass 的启动
group count 由 shader 写入 `CAPAEnvironment`，Metal 侧应使用 indirect dispatch。

## 当前 10 个 Shader Pass

0. CPU pass0
   - CPU 只构建 path-space flattened edges 和 path metadata。
   - CPU 负责提供预算上限：short-edge task、path tile、path tile row、
     boundary tile、short-edge chunk 等 buffer 容量。
   - CPU 可提供保守 batch bounds 用于分配 `globalTiles` 最大容量，但实际全局
     tile span 由 GPU prepare pass 写入 `CAPAEnvironment`。

1. `capa_prepare.glsl`
   - 每条 edge 一个 64-wide thread。
   - 将 path-space edge 变换到 surface-space。
   - 更新每个 path 的 clipped bounds。
   - 写 short-edge task，并累加 `env.taskCount`。

2. `capa_prepare1.glsl`
   - 每个 path 一个 32-wide thread。
   - 根据 pass1 写出的 path bounds 分配 `path.tileOffset/tileCount/tileRect`。
   - 写 `tileRows`，累加 `env.pathTileCount/env.pathTileRowCount`。
   - 合并写 `env.globalTileBounds`。

3. `capa_prepare2.glsl`
   - 单线程 pass。
   - 根据 `env.globalTileBounds/taskCount/pathTileRowCount` 写后续 indirect
     dispatch 参数：
     `orderPassGroups_Size32`、`binPassGroups_Size64`、
     `prefixPassGroups_Size16_2`、`compositePassGroups_Size16_16`。
   - 同时写 `env.globalTileSpan/globalTileCount`。

4. `capa_order.glsl`
   - 使用 `env.orderPassGroups_Size32` indirect dispatch。
   - 每个 global tile 线程按 draw order 建立 path-tile 链。
   - 初始化对应 `CAPAPathTile`，并写 `CAPAGlobalTile.head`。

5. `capa_bin.glsl`
   - 使用 `env.binPassGroups_Size64` indirect dispatch。
   - 将 short-edge task 加入 tile-local short-edge chunk list。
   - 首次命中 path tile 时分配 real `CAPABoundaryTile`，`boundaryTileIndex`
     保留值为 `0` 空、`1` 实心、`2` 锁/失败调试、`>=3` real boundary tile。
   - 累加 `env.shortEdgeChunkCount/env.boundaryTileCount`。

6. `capa_bin1.glsl`
   - 单线程 pass。
   - 根据 `env.boundaryTileCount` 写 `backdropPassGroups_Size16_2`。
   - 这个 group count 同时供 backdrop 和 coverage 两个 boundary-tile pass 使用。

7. `capa_backdrop.glsl`
   - 使用 `env.backdropPassGroups_Size16_2` indirect dispatch。
   - `local_size_x=16, local_size_y=2`，一个 workgroup 处理两个 boundary tiles。
   - 计算每个 boundary tile 的 per-row local backdrop；tileX0 特殊保存左侧
     累积值，并临时借用 `coverage[row]` 存 tileX0 local 值。

8. `capa_prefix.glsl`
   - 使用 `env.prefixPassGroups_Size16_2` indirect dispatch。
   - 按 path tile row 做横向 prefix。
   - 将空边 tile 标记为空或实心，实心使用 `boundaryTileIndex=1`。

9. `capa_coverage.glsl`
   - 使用 `env.backdropPassGroups_Size16_2` indirect dispatch。
   - 只处理 real boundary tiles，从索引 `3` 开始。
   - 将 short-edge chunk + row backdrop 积分成 16x16 packed R8 coverage page。

10. `capa_composite.glsl`
    - 使用 `env.compositePassGroups_Size16_16` indirect dispatch。
    - 每个 global tile 一个 16x16 workgroup，每个线程写一个 pixel。
    - 按 global tile 的 ordered path-tile 链读取 coverage，并在 shader 内按
      `path.blendMode` 做纯色 ordered composite，写入目标 texture。

## Metal Wiring Rules

- `MetalCanvas::drawCAPACmd()` 必须提交上面的 10 个 shader pass。
- 只有生成调度参数之前的 pass 可以用 CPU 固定 group count：
  `prepare`、`prepare1`、`prepare2`、`bin1`。
- `order`、`bin`、`backdrop`、`prefix`、`coverage`、`composite` 必须从
  `CAPAEnvironment` 对应 `uvec4` 字段做 Metal indirect dispatch。
- `CAPAEnvironment` 的 `uvec4` 字段按 16 字节对齐，Metal indirect dispatch 读取
  前三个 `uint32_t` 作为 `threadgroupsPerGrid`。
- 不要在 C++ 里用预算上限直接启动后续 pass；预算只用于分配 buffer capacity。
