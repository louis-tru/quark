# View layout and styles

[中文](VIEW_STYLE-cn.md) · [Back to README](../../README.md) · [API index](https://quarks.cc/doc/)

Quark views describe visible UI elements and receive events from the hardware
and operating system. They form an explicit view tree; Quark does not use a
browser DOM or browser CSS layout engine.

For complete property and method documentation, see the [View API].

## View class hierarchy

The public classes declared in `libs/quark/view.ts` currently have this
inheritance structure. `MorphView`, `TextOptions`, `ScrollView`, and `Player`
are interfaces or behavior contracts, so they are shown as `implements`
annotations instead of base-class nodes:

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
		* [Agent] — abstract
			* [Sprite]
			* [Spine]
	* [Label] — implements [TextOptions]
	* [InputSink]

## Layout example

The JSX syntax can look familiar to web developers, but its elements and
properties operate on Quark's native view and layout system:

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

## CSS-like styles

Quark provides a class-driven style system inspired by CSS and designed for
GUI view hierarchies. It is not a complete browser CSS selector engine: only
class selectors, the supported hierarchy operators, and the three built-in
pseudo states participate in matching.

### Supported selector syntax

- Descendant selectors use spaces. `.a .b .c` matches `.b` and `.c` at any
  deeper descendant level in sequence.
- Direct child selectors use `>`. `.a > .b > .c` requires each following
  match to be an immediate child of the previous one.
- Continuous class selectors have no spaces. `.a.b.c` matches one View that
  has all three classes.
- The supported pseudo states are `:normal`, `:hover`, and `:active`. A pseudo
  state must be the final suffix of its continuous class segment:
  `.a.b:active .c.d` is valid, while `.a:active.b` is invalid.
- Multiple selector expressions may be grouped with commas.

For example, `.div_cls.div_cls2:active .aa.bb.cc` selects a descendant with
classes `aa`, `bb`, and `cc` under an active ancestor that has both `div_cls`
and `div_cls2`.

### Style transitions

Each style rule may specify a `time` value in milliseconds to define the
transition duration when switching into that rule. If `time` is omitted, the
style change is applied immediately. When a transition is requested, Quark
creates and plays an action internally.

### Pseudo states

The style system supports three interaction states:

1. `normal` applies when the pointer or touch is no longer over or pressing
   the view.
2. `hover` applies when the pointer enters the view or it gains focus.
3. `active` applies while the pointer or touch is pressed.

Pseudo states are resolved at runtime from view interaction events.

### Stylesheet example

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
