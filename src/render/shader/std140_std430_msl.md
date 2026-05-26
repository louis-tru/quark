# GLSL std140 / std430 / MSL Constant Buffer Layout 总结

本文用于记录 GLSL `std140`、`std430` 与 Metal Shading Language `constant` buffer 的常见对齐规则，尤其是 `vec3/float3`、数组、结构体、矩阵的差异。

---

## 1. std140 核心理解

`std140` 不是“所有成员都强制 16 字节对齐”。

更准确地说：

- 标量仍然按 4 字节对齐。
- `vec2` 按 8 字节对齐。
- `vec3/vec4` 按 16 字节对齐。
- 数组元素 stride 会向上补到 16 字节。
- 结构体 alignment 至少向上补到 16 字节。
- 矩阵按向量数组处理，默认 column-major。

### 基础类型

| 类型 | alignment | size |
|---|---:|---:|
| `float/int/bool` | 4 | 4 |
| `vec2` | 8 | 8 |
| `vec3` | 16 | 12 |
| `vec4` | 16 | 16 |

### 最容易误解的地方：vec3 后面的 4 字节 padding 可以复用

```glsl
layout(std140) uniform test {
    float a; // offset 为 0
    float b; // offset 为 4
    vec3  f; // offset 为 16
    float s; // offset 为 28
};
```

解释：

```txt
float a: offset 0，占 4 byte
float b: offset 4，占 4 byte
vec3  f: offset 16，占 12 byte，即 16,20,24
float s: offset 28，复用 vec3 后面的 4 byte padding
```

所以 `vec3` 的起点需要 16 对齐，但它不是永远独占完整 16 字节牢房。

如果后面也是 `vec3`：

```glsl
layout(std140) uniform test {
    float a; // offset 为 0
    float b; // offset 为 4
    vec3  f; // offset 为 16
    vec3  s; // offset 为 32
};
```

因为第二个 `vec3 s` 自己也要求 16 字节对齐，所以不能放在 28，只能跳到 32。

---

## 2. std140 数组规则

数组是 `std140` 最典型的 16 字节膨胀来源。

```glsl
layout(std140) uniform test {
    float a[3]; // a[0] offset 0, a[1] offset 16, a[2] offset 32
};
```

虽然 `float` 本身只有 4 字节，但在 `std140` 数组中：

```txt
array stride = 16
```

即：

```txt
a[0] = 0
a[1] = 16
a[2] = 32
```

---

## 3. std140 结构体规则

结构体的 alignment 至少向上补到 16 字节，结构体 size 也会补齐到 16 的倍数。

```glsl
struct st {
    float b;
};

layout(std140) uniform test {
    st    c; // offset 为 0
    float d; // offset 为 16
};
```

虽然 `st` 内部只有一个 `float`，但 `st` 作为结构体成员时，整体 size 会补到 16。

---

## 4. std140 矩阵规则

矩阵本质上按向量数组处理。默认是 column-major。

例如：

```glsl
layout(std140) uniform test {
    mat3x3 mat1; // offset 为 0
    mat3x3 mat;  // offset 为 48
};
```

`mat3x3` 默认等价于 3 列 `vec3`：

```txt
mat3 = vec3[3]
```

每一列 stride 为 16，所以：

```txt
mat3 size = 3 * 16 = 48
```

---

## 5. std140 标量后接 mat4

```glsl
layout(std140) uniform test {
    float  a; // offset 为 0
    mat4x4 b; // offset 为 16
};
```

`mat4x4` 的列向量是 `vec4`，需要 16 字节对齐，所以 `b` 从 offset 16 开始。

---

## 6. std430 核心理解

`std430` 可以理解成：取消 `std140` 里数组和结构体强制按 16 字节膨胀的规则。

但基础类型对齐规则基本一样：

| 类型 | alignment | size |
|---|---:|---:|
| `float/int/bool` | 4 | 4 |
| `vec2` | 8 | 8 |
| `vec3` | 16 | 12 |
| `vec4` | 16 | 16 |

所以对于普通成员排列：

```glsl
float a;
float b;
vec3  f;
float s;
```

`std140` 和 `std430` 的结果通常一样：

```txt
a = 0
b = 4
f = 16
s = 28
```

也就是说，`std430` 中 `vec3` 后面的 padding 同样可以被后面的 4 字节标量复用。

---

## 7. std140 与 std430 的主要区别

### 数组

```glsl
float a[3];
```

| layout | offset |
|---|---|
| `std140` | `0, 16, 32` |
| `std430` | `0, 4, 8` |

### 结构体

```glsl
struct st {
    float b;
};
```

| layout | `st` alignment | `st` size |
|---|---:|---:|
| `std140` | 16 | 16 |
| `std430` | 4 | 4 |

### 矩阵

`mat3` 在 `std140` 和 `std430` 中通常都是 48 字节，因为 `vec3` 仍然要求 16 字节对齐。

但是 `mat2` 这类由 `vec2` 组成的矩阵会不同：

| 类型 | std140 | std430 |
|---|---:|---:|
| `mat2` | 32 | 16 |
| `mat3` | 48 | 48 |
| `mat4` | 64 | 64 |

---

## 8. MSL constant buffer 核心理解

Metal Shading Language 的 `constant` buffer 更接近：

```txt
C/C++ struct ABI + SIMD vector alignment
```

它不是 `std140`，也不是完全的 `std430`。

### MSL 基础类型

| 类型 | alignment | size |
|---|---:|---:|
| `float` | 4 | 4 |
| `int` | 4 | 4 |
| `float2` | 8 | 8 |
| `float3` | 16 | 16 |
| `float4` | 16 | 16 |

重点：

```txt
MSL float3 的 size 是 16，不是 12。
```

这和 GLSL 的 `vec3` 最大区别：

| 类型 | alignment | size |
|---|---:|---:|
| GLSL `vec3` in std140/std430 | 16 | 12 |
| MSL `float3` | 16 | 16 |

---

## 9. MSL float3 padding 不可复用

```metal
struct Test {
    float  a;
    float  b;
    float3 f;
    float  s;
};
```

MSL layout：

```txt
a = 0
b = 4
f = 16
s = 32
```

原因：

```txt
float3 size = 16
```

所以 `float3` 后面的 4 字节不是 GLSL `vec3` 那种可复用 padding，而是 `float3` 类型本身的一部分。

---

## 10. MSL packed_float3

MSL 提供 `packed_float3`：

| 类型 | alignment | size |
|---|---:|---:|
| `packed_float3` | 4 | 12 |

它更接近 GLSL `vec3` 的大小。

```metal
struct Test {
    float         a;
    packed_float3 f;
    float         s;
};
```

layout：

```txt
a = 0
f = 4
s = 16
```

但注意：`packed_float3` 可能不如天然 `float3/float4` 访问高效。跨 API 时通常要在“布局一致性”和“访问效率”之间做取舍。

---

## 11. MSL 数组与结构体

MSL 数组更像 C ABI / std430，不会像 `std140` 那样强制所有数组元素 stride 到 16。

```metal
constant float a[3];
```

layout：

```txt
a[0] = 0
a[1] = 4
a[2] = 8
```

MSL struct alignment 通常是成员最大 alignment，不会像 `std140` 那样强制结构体至少 16 字节对齐。

---

## 12. 总结表

| 特性 | std140 | std430 | MSL constant |
|---|---|---|---|
| `float` align | 4 | 4 | 4 |
| `vec2/float2` align | 8 | 8 | 8 |
| `vec3/float3` align | 16 | 16 | 16 |
| `vec3` size | 12 | 12 | 16 |
| `vec3` 后 padding 可复用 | 是 | 是 | 否 |
| 数组 stride 强制 16 | 是 | 否 | 否 |
| struct 强制至少 16 对齐 | 是 | 否 | 否 |
| 更像 C/C++ ABI | 否 | 部分 | 是 |

---

## 13. 跨 API 工程建议

### 不建议跨 API buffer 里直接使用 vec3/float3

最容易出问题的是：

```glsl
vec3
```

因为：

| API | size |
|---|---:|
| GLSL std140/std430 `vec3` | 12 |
| MSL `float3` | 16 |

如果 GLSL 直接翻译成 MSL `float3`，后续字段 offset 很容易错位。

### 更安全的做法

优先使用：

```glsl
vec4
```

或者显式 padding：

```cpp
struct Vec3Padded {
    float x;
    float y;
    float z;
    float _pad;
};
```

也可以在 MSL 中使用：

```metal
packed_float3
```

但要注意访问效率与代码生成一致性。

---

## 14. 一句话记忆

```txt
std140：数组和结构体按 vec4 车位停，保守但浪费。
std430：取消数组/结构体的强制 vec4 车位，更紧凑。
MSL：更像 C++ SIMD ABI，float3 是真正 16 字节对象。
```

