# Vulkan Minimal

这是一个与 Quark renderer 完全解耦的 Android Vulkan 最小测试。它只建立：

```text
NativeActivity window
  -> VkInstance / VkSurfaceKHR
  -> VkPhysicalDevice / VkDevice / graphics+present queue
  -> VkSwapchainKHR
  -> clear-only render pass
  -> queue submit / present
```

运行后窗口会显示不断变化的纯色。这里没有 shader、graphics pipeline、vertex
buffer、Quark 类型或 `src/render/vulkan` 依赖。后续可以从这个闭环开始逐层加入
pipeline、buffer、texture 和 Quark 的 Canvas 语义。
