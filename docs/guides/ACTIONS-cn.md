# Action 与关键帧

[English](ACTIONS.md) · [全部教程](README-cn.md) · [API 索引](https://quarks.cc/doc/)

Quark 的 Action 系统独立驱动动画，不依赖具体布局或渲染后端。Action 随时间
改变绑定 View 所支持的属性，View 接收这些值，再由正常的布局和渲染流程显示
结果。

关键帧过渡支持三次贝塞尔曲线，以及 `linear`、`ease`、`easeIn`、`easeOut`、
`easeInOut` 等内置曲线。

## 基础动画

下面的示例让 `Box` 在一秒内从 `x = 0` 移动到 `x = 100`：

```ts
import { Application, Window, Morph, KeyframeAction } from 'quark'

const app = new Application()
const win = new Window()
const view = new Morph(win)
const action = new KeyframeAction(win)

action.add({ x: 0, time: 0 })
action.add({ x: 100, time: 1000 })

view.width = 50
view.height = 50
view.backgroundColor = '#f00'
view.action = action
view.appendTo(win.root)

action.loop = 1000
action.play()
```

## Action 类型

公开的 Action 类继承关系如下：

* [Action] — 抽象基类
	* [SpawnAction] — 并行执行子 Action
	* [SequenceAction] — 串行执行子 Action
	* [KeyframeAction] — 使用关键帧改变 View 样式属性

`Action` 提供 `play()`、`stop()`、`seek()`、`seekPlay()`、`seekStop()`、
`append()`、`clear()` 等播放与组合操作。

### SpawnAction

`SpawnAction` 并行执行所有子 Action，并以最长子 Action 的时长作为自身时长。
较短的子 Action 完成后会保持完成状态，直到整个组合结束或进入下一次循环。

### SequenceAction

`SequenceAction` 按顺序逐个执行子 Action。最后一个子 Action 完成后，序列结束
或按配置开始下一次循环。

## KeyframeAction 与 Keyframe

`KeyframeAction` 包含多个 [Keyframe]。`Keyframe` 继承 `StyleSheets`，因此
属性名和值类型与 View 样式一致。属性只有在绑定的 View 支持时才有效：例如
`x` 适用于 `Morph` 等支持变换的 View，`width` 适用于 `Box` 及其子类。

```ts
// 有效：Box 支持 width 和 height。
const boxAction = new KeyframeAction(win)
const box = new Box(win)
box.backgroundColor = '#f00'
boxAction.add({ width: 10, height: 10 })
boxAction.add({ width: 100, height: 100, time: 1000 })
box.action = boxAction
boxAction.play()

// View 不提供 width 或 height，因此这些属性不会产生效果。
const viewAction = new KeyframeAction(win)
const view = new View(win)
viewAction.add({ width: 10, height: 10 })
viewAction.add({ width: 100, height: 100, time: 1000 })
view.action = viewAction
viewAction.play()
```

如果需要声明式创建，[createAction] 可以接收关键帧数组，也可以接收包含
`keyframe`、`spawn` 或 `seq` 子项的对象。

## Action 事件

View 提供两个 Action 事件：

- `View.onActionKeyframe` 在 Action 到达关键帧时触发。事件在实际渲染帧上派发，
  因此可能比目标时间略早或略晚；`ActionEvent.delay` 保存与目标时间的差值。
- `View.onActionLoop` 在新循环开始时触发，首次执行不会触发。事件同样提供
  `delay` 和当前 `looped` 次数。

事件中的 `action` 和 `frame` 属性用于识别触发事件的 Action 与关键帧。

[Action]: https://quarks.cc/doc/action.html#class-action
[SpawnAction]: https://quarks.cc/doc/action.html#class-spawnaction
[SequenceAction]: https://quarks.cc/doc/action.html#class-sequenceaction
[KeyframeAction]: https://quarks.cc/doc/action.html#class-keyframeaction
[Keyframe]: https://quarks.cc/doc/action.html#class-keyframe
[createAction]: https://quarks.cc/doc/action.html#createaction-win-arg-parent-
