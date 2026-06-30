# CAPA PASS PROCESS

本文记录当前 `src/render/shader/capa/` 目录里的 CAPA pass 事实源。不要再按旧的
7-pass 记忆接 Metal；当前目录中有 12 个 shader pass，且多个后续 pass 的启动
group count 由 shader 写入 `CAPAEnvironment`，Metal 侧应使用 indirect dispatch。

## 当前 12 个 Shader Pass

0. CPU pass0
   - CPU 只构建 path-space flattened edges 和 path metadata。
   - CPU 负责提供预算上限：short-edge task、small tile、path tile row、
     boundary tile、ordered path tile、short-edge node 等 buffer 容量。
   - CPU 可提供保守 batch bounds 用于分配 `globalTiles` 最大容量，但实际全局
     tile span 由 GPU prepare pass 写入 `CAPAEnvironment`。

1. `capa_prepare.glsl`
   - 每条 edge 一个 64-wide thread。
   - 将 path-space edge 变换到 surface-space。
   - 更新每个 path 的 clipped bounds。
   - 写 short-edge task，并累加 `env.taskCount`。

2. `capa_prepare_tiles.glsl`
   - 每个 path 一个 32-wide thread。
   - 根据 `capa_prepare.glsl` 写出的 path bounds 分配线性 `CAPASmallTile`
     区间和 tile row 区间；此处不初始化最终 composite 用的 `CAPAPathTile`。
   - 只有该 path 的 tile rows 全部分配成功时，才写
     `path.tileOffset/tileRowOffset/tileRect/tileEnd`。
   - 写 `CAPAPathTileRow(pathIndex, pathTileIndex, CAPA_NIL, 0)`，累加
     `env.pathTileCount/env.pathTileRowCount`。
   - 合并写 `env.globalTileBounds`。

3. `capa_prepare_dispatch.glsl`
   - 单线程 pass。
   - 根据 `env.globalTileBounds/taskCount/pathTileRowCount` 写后续 indirect
     dispatch 参数：
     `tilePassGroups_Size32`、`orderPassGroups_Size32`、
     `binPassGroups_Size32`、`classifyPassGroups_Size32`、
     `prefixPassGroups_Size16_2`、`compositePassGroups_Size16_16`。
   - 同时写 `env.globalTileSpan/globalTileCount`。

4. `capa_tile.glsl`
   - 使用 `env.tilePassGroups_Size32` indirect dispatch。
   - `local_size_x=32`，每个线程清一个线性 `CAPASmallTile`。
   - 只做 small-tile 空数据初始化：`CAPASmallTile.value=CAPA_NIL`；不按 path
     遍历，不建立 `CAPAGlobalTile` 链。

5. `capa_bin.glsl`
   - 使用 `env.binPassGroups_Size32` indirect dispatch。
   - 将 short-edge task 加入 `CAPASmallTile.value` 指向的 tile-local
     short-edge node list，不在本 pass 分配 `CAPABoundaryTile`。
   - 水平边也写入一个无面积贡献的远 y 轴 marker node，用来表示该 tile 需要
     后续 boundary tile 分配。
   - DANGER: 不要用 `boundaryTileIndex` 或其它 SSBO 字段实现 shader 自旋锁。
     GPU 不保证拿到锁的 invocation 会先运行到解锁路径；跨 invocation 等待可能
     死锁。`capa_bin.glsl` 只能使用不等待其它 invocation 前进的原子
     append/overflow 设计。
   - IMPORTANT: 不要再使用“多个 invocation 共享一个小 short-edge chunk，
     `atomicAdd(count)` 抢槽，满了后同 pass 动态挂 overflow chunk”的协议。
     实测链表本身可达但会丢 short-edge slot。更安全的方向是每个 emitted
     short edge 自身作为链表节点，或拆成 count/prefix/fill 多 pass。
   - 最终短边存储方向：`CAPAShortEdge` 自身就是链表节点，带 `next` 字段。
     因为 `capa_prepare.glsl` 保证一个 short-edge task 最多触碰 3 个 tile，所以分配
     `maxShortEdgeCount * 3` 个节点，每个 task 独占 `taskIndex * 3 + 0..2`。
     bin pass 只把已经写好的节点用 `atomicExchange(smallTile.value,
     nodeIndex)` 挂到 tile 链表上，不再需要 scan、共享 chunk 槽位或 overflow
     修复。
   - HARD RULE: 同一个 pass 内不要做复杂的 atomic 写后读依赖状态机。不要让
     invocation A 写出的 atomic 状态被 invocation B 在同一 pass 读出后继续驱动
     共享容器扩容、所有权转移或链表修复；这种逻辑必须拆 pass 或改成每个
     invocation 只写自己独占的节点/槽位。

6. `capa_boundary.glsl`
   - 使用 `env.classifyPassGroups_Size32` indirect dispatch，每个线程处理一条
     `CAPAPathTileRow`。
   - 扫描该 row 的 `CAPASmallTile`，为非空 small tiles 连续分配
     `CAPABoundaryTile`，写入 `pathIndex`、`shortEdgeHead`、`tileCoord`。
   - 将 `CAPASmallTile.value` 从 short-edge head 改写为真实 boundary tile
     index；分配失败时写回 `CAPA_NIL`。
   - 最后完成的 row 线程根据 `env.boundaryTileCount` 写
     `backdropPassGroups_Size16_2`，这个 group count 同时供 backdrop 和
     coverage 两个 boundary-tile pass 使用。

7. `capa_backdrop.glsl`
   - 使用 `env.backdropPassGroups_Size16_2` indirect dispatch。
   - `local_size_x=16, local_size_y=2`，一个 workgroup 处理两个 boundary tiles。
   - 计算每个 boundary tile 的 per-row local backdrop。tileX0 的左侧初始
     前缀临时借用 `coverage[row]` 保存，本 tile 的 local row 值写入
     `backdrop[row]`。

8. `capa_classify.glsl`
   - 使用 `env.classifyPassGroups_Size32` indirect dispatch。
   - `local_size_x=32`，每个线程处理一条 path tile row。
   - 扫描该 row 的 `CAPASmallTile`，此时输入只应是真实 boundary tile index
     或 `CAPA_NIL`。用 boundary tile 的 row0 prefix/backdrop 做 full/empty
     分类，只在需要 full 标记时把
     `CAPASmallTile.value` 写成 `CAPA_FULL_TILE` (`CAPA_NIL - 1`)。

9. `capa_prefix.glsl`
   - 使用 `env.prefixPassGroups_Size16_2` indirect dispatch。
   - 沿 `CAPAPathTileRow.boundaryTileIndex/count` 指向的连续 boundary tile
     区间做真正 per-row prefix。
   - 读取 `backdrop[row]` 的 local row 值，并把 `backdrop[row]` 改写成
     coverage pass 需要的 tile-left row prefix。

10. `capa_coverage.glsl`
   - 使用 `env.backdropPassGroups_Size16_2` indirect dispatch。
   - 只处理 real boundary tiles；真实 boundary tile index 从 `0` 开始。
   - 开头先从 `backdrop[row]` 读出本行 tile-left prefix 到私有变量，然后
     将 short-edge node + row prefix 积分成 16x16 packed R8 coverage page。

11. `capa_order.glsl`
    - 使用 `env.orderPassGroups_Size32` indirect dispatch。
    - coverage/prefix 完成后，每个 global tile 线程只按 path `tileRect` 统计
      当前 tile 命中的 path-tile 层数，不在统计阶段读取 coverage/small-tile
      语义，也不做 full-tile 预混合。
    - 常见路径先把最多 16 个 pathIndex 放入线程本地临时数组；超过临时容量时
      记录第一个溢出 pathIndex，写入阶段从该位置继续遍历。
    - 用 `env.orderedPathTileCount` 一次性分配连续 `CAPAPathTile` span，再从
      `CAPASmallTile.value` 把 boundary tile index 写入新的 `CAPAPathTile`；
      `CAPA_NIL` 空 tile 不写入最终 span。
    - `CAPAGlobalTile.head/count` 表示这个 global tile 的连续 path-tile span，
      供 composite 线性访问。
    - 写入阶段会恢复连续 `SrcOver` full tile 预混合。分配使用 raw hit
      count，实际写入可能更少；`CAPAGlobalTile.head/count` 指向分配区尾部
      已写好的连续节点。

12. `capa_composite.glsl`
    - 使用 `env.compositePassGroups_Size16_16` indirect dispatch。
    - 每个 global tile 一个 16x16 workgroup，每个线程写一个 pixel。
    - 按 global tile 的 ordered path-tile 连续 span 读取 coverage；`CAPA_NIL`
      视为 0 coverage，`CAPA_FULL_TILE` 视为 1 coverage，其它值读取真实
      boundary coverage，并在 shader 内按 `path.blendMode` 做纯色 ordered
      composite，写入目标 texture。

## Metal Wiring Rules

- `MetalCanvas::drawCAPACmd()` 必须提交上面的 12 个 shader pass。
- 只有生成调度参数之前的 pass 可以用 CPU 固定 group count：
  `prepare`、`prepare_tiles`、`prepare_dispatch`。
- `tile`、`order`、`bin`、`boundary`、`backdrop`、`classify`、`prefix`、
  `coverage`、`composite` 必须从 `CAPAEnvironment` 对应 `uvec4` 字段做
  Metal indirect dispatch。
- `CAPAEnvironment` 的 `uvec4` 字段按 16 字节对齐，Metal indirect dispatch 读取
  前三个 `uint32_t` 作为 `threadgroupsPerGrid`。
- 不要在 C++ 里用预算上限直接启动后续 pass；预算只用于分配 buffer capacity。
