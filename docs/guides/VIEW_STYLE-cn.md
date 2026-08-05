# 视图布局与样式

[English](VIEW_STYLE.md) · [返回 README](../../README-cn.md) · [API 索引](https://quarks.cc/doc/)

Quark 的 View 描述屏幕上的可见 UI 元素，并响应硬件和操作系统产生的事件。
View 组成显式的视图树；Quark 不使用浏览器 DOM 或浏览器 CSS 布局引擎。

完整属性和方法请查阅 [View API]。

## View 类继承关系

`libs/quark/view.ts` 当前公开类的继承关系如下。`MorphView`、`TextOptions`、
`ScrollView` 和 `Player` 是接口或行为约定，因此以 `implements` 标注，而不作为
基类节点列出：

* [View]
	* [Br]
	* [Box]
		* [Flex]
			* [Flow]
		* [Free]
		* [Image]
			* [Video] — implements `Player`
		* [Morph] — implements [MorphView]
			* [World]
			* [Root]
		* [Text] — implements [TextOptions]
			* [Button]
		* [Input] — implements [TextOptions]
			* [Textarea] — implements [ScrollView]
		* [Scroll] — implements [ScrollView]
	* [Entity] — implements [MorphView]
		* [Agent] — 抽象类
			* [Sprite]
			* [Spine]
	* [Label] — implements [TextOptions]
	* [InputSink]

## 布局示例

JSX 语法对 Web 开发者可能比较熟悉，但其中的元素和属性实际作用于 Quark
自己的原生 View 与布局系统：

```tsx
import { Jsx, Application, Window } from 'quark'

new Application()
new Window().render(
	<flex width="100%" height="50%" itemsAlign="centerCenter">
		<button
			minWidth="10%"
			maxWidth="40%"
			height="100%"
			paddingLeft={5}
			lineHeight={1}
			fontSize={18}
			fontFamily="iconfont"
			backgroundColor="#f00"
			whiteSpace="noWrap"
			textAlign="center"
		>
			<label
				fontFamily="default"
				fontSize={16}
				textOverflow="ellipsis"
				value="ABCDEFGHIJKMLNOPQ"
			/>
		</button>
		<text
			weight={[0, 1]}
			height="100%"
			textColor="#00f"
			lineHeight={1}
			fontSize={16}
			whiteSpace="noWrap"
			fontWeight="bold"
			textOverflow="ellipsisCenter"
			textAlign="center"
			value="Title"
			backgroundColor="#0f0"
		/>
		<text
			minWidth="10%"
			maxWidth="40%"
			height="100%"
			textColor="#f0f"
			lineHeight={1}
			backgroundColor="#0ff"
			textAlign="center"
			value="A"
			opacity={0.5}
		/>
	</flex>
)
```

## CSS-like 样式

Quark 提供了一个受 CSS 启发、面向 GUI 视图层级的 class 驱动样式系统。
它不是完整的浏览器 CSS 选择器引擎；只有 class、受支持的层级操作符和三种
内置伪状态参与匹配。

### 支持的选择器语法

- 后代选择器使用空格。`.a .b .c` 按顺序匹配任意更深层级的 `.b` 和 `.c`。
- 直接子级选择器使用 `>`。`.a > .b > .c` 要求后一个匹配项必须是前一个
  匹配项的直接子 View。
- 连续 class 选择器之间没有空格。`.a.b.c` 匹配同时拥有这三个 class 的
  同一个 View。
- 支持 `:normal`、`:hover`、`:active` 三种伪状态。伪状态必须是当前连续
  class 段的最后后缀：`.a.b:active .c.d` 合法，`.a:active.b` 非法。
- 多个选择器表达式可以使用逗号组合。

例如，`.div_cls.div_cls2:active .aa.bb.cc` 会匹配一个同时具有 `aa`、`bb`、
`cc` 三个 class 的后代；它的祖先同时具有 `div_cls`、`div_cls2`，且处于
`active` 状态。

### 样式过渡

每个样式规则都可以指定一个以毫秒为单位的 `time`，表示切换到该规则时的
过渡时长。未指定 `time` 时，样式立即生效。需要过渡时，Quark 会在内部创建
并播放一个 Action。

### 伪状态

样式系统支持三种交互状态：

1. `normal`：指针或触摸不再停留或按压 View 时应用。
2. `hover`：指针进入 View 或 View 获得焦点时应用。
3. `active`：指针或触摸按下期间应用。

伪状态会在运行时根据 View 的交互事件解析。

### 样式表示例

```tsx
import { Jsx, createCss } from 'quark'

createCss({
	'.a': {
		width: 'match',
		lineHeight: 45,
		whiteSpace: 'pre',
		fontSize: 16,
	},
	'.a:normal': {
		textColor: '#0f0',
	},
	'.a:hover': {
		textColor: '#f0f',
	},
	'.a:active': {
		textColor: '#f00',
	},
	'.a .b': {
		fontSize: 20,
	},
	'.a > .c': {
		width: 100,
	},
	'.a.b:active .c.d': {
		opacity: 0.8,
	},
	'.a:normal .b': {
		time: 500,
		textColor: '#000',
	},
	'.a:hover .b': {
		time: 500,
		textColor: '#f00',
	},
})

const view = (
	<text class="a">
		<label value="Hello A!" />
		<label class="b" value="Hello B!" />
	</text>
)
```

[View API]: https://quarks.cc/doc/view.html
[View]: https://quarks.cc/doc/view.html#class-view
[Br]: https://quarks.cc/doc/view.html#class-br
[Box]: https://quarks.cc/doc/view.html#class-box
[Flex]: https://quarks.cc/doc/view.html#class-flex
[Flow]: https://quarks.cc/doc/view.html#class-flow
[Free]: https://quarks.cc/doc/view.html#class-free
[Image]: https://quarks.cc/doc/view.html#class-image
[Video]: https://quarks.cc/doc/view.html#class-video
[Morph]: https://quarks.cc/doc/view.html#class-morph
[MorphView]: https://quarks.cc/doc/view.html#interface-morphview
[World]: https://quarks.cc/doc/view.html#class-world
[Root]: https://quarks.cc/doc/view.html#class-root
[Text]: https://quarks.cc/doc/view.html#class-text
[TextOptions]: https://quarks.cc/doc/view.html#interface-textoptions
[Button]: https://quarks.cc/doc/view.html#class-button
[Input]: https://quarks.cc/doc/view.html#class-input
[Textarea]: https://quarks.cc/doc/view.html#class-textarea
[ScrollView]: https://quarks.cc/doc/view.html#interface-scrollview
[Scroll]: https://quarks.cc/doc/view.html#class-scroll
[Entity]: https://quarks.cc/doc/view.html#class-entity
[Agent]: https://quarks.cc/doc/view.html#class-agent
[Sprite]: https://quarks.cc/doc/view.html#class-sprite
[Spine]: https://quarks.cc/doc/view.html#class-spine
[Label]: https://quarks.cc/doc/view.html#class-label
[InputSink]: https://quarks.cc/doc/view.html#class-inputsink
