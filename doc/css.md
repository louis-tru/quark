# `ftr/css`

* `css`样式表类似于html `css`样式表，支持使用多级样式表，但只支持`class`类

* 支持`3`种伪类型`normal`、`hover`、`down`

	对应[`View.onHighlighted`]事件中的 [`HIGHLIGHTED_NORMAL`] 、[`HIGHLIGHTED_HOVER`]、[`HIGHLIGHTED_DOWN`]

* 这里的样式表没有权重的概念，使用直接赋值的方式与使用样式表权重都相同，

	但需要注意先后顺序小心直接赋值会被样式表覆盖，

	因为每次修改的样式表不会立即生效都会在渲染前才应用到视图。

* 样式表级数越多越复杂也表示需要更多的时间查询与处理。

## create(sheets) 

* 创建样式表

* @arg `sheets` {[`Object`]}

## check(cssName)

* 检查样式名称是否有效

* @arg `cssName` {[`String`]}
* @ret {[`bool`]}

## CSS(sheets)

* 创建样式表，在调式状态时会检查样式名称是否有效

* @arg `sheets` {[`Object`]}

Example:

```js
import CSS from 'ftr/css'
import { GUIApplication, Root, Div } from 'ftr'
import 'ftr/dialog'
// 样式表都是全局的
CSS({
	'.test': {
		width: '50%',
		height: '50%',
		backgroundColor: '#00f',
	},
	'.test .a': {
		width: 50,
		height: 50,
	},
	'.test .a.b': { // 这种选择器会优先级会更高
		height: 60,
	},
	// 应用这些伪类到目标，要使用它们对目标生效，需目标视图能够接收事件
	'.test:normal .a': {
		backgroundColor: '#0000',
	},
	'.test:hover': {
		backgroundColor: '#f0f',
	},
	'.test:hover .a': {
		backgroundColor: '#f00',
	},
})
new GUIApplication().start(
	// 需要注意jsx语法只能存在于.jsx文件中
	<Root>
		<Div class="test"onClick=(e=>{ dialog.alert('Hello!') })>
			<Div class="a b" />
		</Div>
	</Root>
)
```


## `Enum PropertyName`

* 以下是所有支持的样式表属性

* 这也是[`KeyframeAction`]中[`Frame`]所支持的所有动作属性

### PROPERTY_X
### PROPERTY_Y
### PROPERTY_SCALE_X
### PROPERTY_SCALE_Y
### PROPERTY_SKEW_X
### PROPERTY_SKEW_Y
### PROPERTY_ROTATE_Z
### PROPERTY_ORIGIN_X
### PROPERTY_ORIGIN_Y
### PROPERTY_OPACITY
### PROPERTY_VISIBLE
### PROPERTY_WIDTH
### PROPERTY_HEIGHT
### PROPERTY_MARGIN_LEFT
### PROPERTY_MARGIN_TOP
### PROPERTY_MARGIN_RIGHT
### PROPERTY_MARGIN_BOTTOM
### PROPERTY_BORDER_LEFT
### PROPERTY_BORDER_TOP
### PROPERTY_BORDER_RIGHT
### PROPERTY_BORDER_BOTTOM
### PROPERTY_BORDER_LEFT_WIDTH
### PROPERTY_BORDER_TOP_WIDTH
### PROPERTY_BORDER_RIGHT_WIDTH
### PROPERTY_BORDER_BOTTOM_WIDTH
### PROPERTY_BORDER_LEFT_COLOR
### PROPERTY_BORDER_TOP_COLOR
### PROPERTY_BORDER_RIGHT_COLOR
### PROPERTY_BORDER_BOTTOM_COLOR
### PROPERTY_BORDER_RADIUS_LEFT_TOP
### PROPERTY_BORDER_RADIUS_RIGHT_TOP
### PROPERTY_BORDER_RADIUS_RIGHT_BOTTOM
### PROPERTY_BORDER_RADIUS_LEFT_BOTTOM
### PROPERTY_BACKGROUND_COLOR
### PROPERTY_NEWLINE
### PROPERTY_CONTENT_ALIGN
### PROPERTY_TEXT_ALIGN
### PROPERTY_MAX_WIDTH
### PROPERTY_MAX_HEIGHT
### PROPERTY_START_X
### PROPERTY_START_Y
### PROPERTY_RATIO_X
### PROPERTY_RATIO_Y
### PROPERTY_REPEAT
### PROPERTY_TEXT_BACKGROUND_COLOR
### PROPERTY_TEXT_COLOR
### PROPERTY_TEXT_SIZE
### PROPERTY_TEXT_STYLE
### PROPERTY_TEXT_FAMILY
### PROPERTY_TEXT_LINE_HEIGHT
### PROPERTY_TEXT_SHADOW
### PROPERTY_TEXT_DECORATION
### PROPERTY_TEXT_OVERFLOW
### PROPERTY_TEXT_WHITE_SPACE
### PROPERTY_ALIGN_X
### PROPERTY_ALIGN_Y
### PROPERTY_SHADOW
### PROPERTY_SRC
### PROPERTY_BACKGROUND_IMAGE


[`Class`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Classes
[`Object`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object
[`Array`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array
[`Function`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Function
[`Date`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date
[`RegExp`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/RegExp
[`ArrayBuffer`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer
[`TypedArray`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/TypedArray
[`String`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String
[`Number`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number
[`Boolean`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Boolean
[`null`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/null
[`undefined`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/undefined

[`int`]: native_types.md#int
[`uint`]: native_types.md#uint
[`int16`]: native_types.md#int16
[`uint16`]: native_types.md#uint16
[`int64`]: native_types.md#int64
[`uint64`]: native_types.md#uint64
[`float`]: native_types.md#float
[`double`]: native_types.md#double
[`bool`]: native_types.md#bool

[`KeyframeAction`]: action.md#class-keyframeaction
[`Frame`]: action.md#class-frame
[`HIGHLIGHTED_NORMAL`]: ftr.md#highlighted_normal
[`HIGHLIGHTED_HOVER`]: ftr.md#highlighted_hover
[`HIGHLIGHTED_DOWN`]: ftr.md#highlighted_down
[`View.onHighlighted`]: ftr.md#view-onhighlighted
