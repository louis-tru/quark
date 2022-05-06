# `noug/display_port`


## `Enum: Orientation`

设置屏幕方向枚举

### ORIENTATION_INVALID
### ORIENTATION_PORTRAIT
### ORIENTATION_LANDSCAPE
### ORIENTATION_REVERSE_PORTRAIT
### ORIENTATION_REVERSE_LANDSCAPE
### ORIENTATION_USER
### ORIENTATION_USER_PORTRAIT
### ORIENTATION_USER_LANDSCAPE
### ORIENTATION_USER_LOCKED


## `Enum: StatusBarStyle`

系统状态栏样式枚举

### STATUS_BAR_STYLE_WHITE 
### STATUS_BAR_STYLE_BLACK


## `Class Display`

这个类型的构造函数禁止访问

可以通过[`app.displayPort`]或[`gui.displayPort`]访问

### Display.onChange

屏幕尺寸变化时触发

### Display.onOrientation

屏幕方向改变时触发

### Display.onRender

一帧渲染完成后触发

### Display.lockSize([width[,height]])
* @arg `[width=0]` {[`float`]}
* @arg `[height=0]` {[`float`]}

* width与height都设置为`0`时自动设置一个最舒适的默认显示尺寸

* 设置锁定视口为一个固定的逻辑尺寸,这个值改变时会触发change事件

* 如果width设置为零表示不锁定宽度,系统会自动根据height值设置一个同等比例的宽度

	如果设置为非零表示锁定宽度,不管[`Display`]尺寸怎么变化对于编程者来说,这个值永远保持不变

* 如果height设置为零表示不锁定,系统会自动根据width值设置一个同等比例的高度

	如果设置为非零表示锁定高度,不管[`Display`]尺寸怎么变化对于编程者来说,这个值永远保持不变

### Display.nextFrame(cb)
* @arg `cb` {[`Function`]}

### Get: Display.width 

* {[`float`]} 

### Get: Display.height 

* {[`float`]} 

### Get: Display.phyWidth 

屏幕的实际物理像素宽度

* {[`float`]} 

### Get: Display.phyHeight 

屏幕的实际物理像素高度

* {[`float`]} 

### Get: Display.bestScale 

系统建议的屏幕缩放比

* {[`float`]} 

### Get: Display.scale 

当前屏幕缩放比

* {[`float`]} 

### Get: Display.scaleValue 

* {[`Vec2`]} 

### Get: Display.rootMatrix 

* {[`Mat4`]} 

### Get: Display.atomPixel

屏幕原子像素尺寸

* {[`float`]} 

### keepScreen(keep)

保持屏幕，不自动进入休眠状态

* @arg `keep` {[`bool`]}

### statusBarHeight()

获取系统状态栏高度，在非显示状态会返回`0`

* @ret {[`float`]}

### setVisibleStatusBar(visible)

设置系统状态栏是否显示

* @arg visible {[`bool`]}

### setStatusBarStyle(style)

设置系统状态栏文本颜色

* @arg color {[`StatusBarStyle`]}

### requestFullscreen(fullscreen)

请求进入全屏或退出全屏状态

* @arg `fullscreen` {[`bool`]}

### orientation()

返回当前屏幕方向

* Returns:
*  `ORIENTATION_PORTRAIT`
*  `ORIENTATION_LANDSCAPE`
*  `ORIENTATION_REVERSE_PORTRAIT`
*  `ORIENTATION_REVERSE_LANDSCAPE`
* @ret {[`Orientation`]} return direction angle

### setOrientation(orientation)

设置屏幕旋转方向，应用初始化时为`ORIENTATION_USER`按当前设备方向自动旋转

* @arg `orientation` {[`Orientation`]}

### fsp()

返回当前刷新率

* @ret {[`uint`]}

## Get: defaultAtomPixel

默认屏幕原子像素尺寸

* {[`float`]}

## Get: defaultStatusBarHeight

默认屏幕bar高度

* {[`float`]}

## Get: atomPixel

屏幕原子像素尺寸

* {[`float`]}

## Get: current 

获取当前`Display`实例

* {[`Display`]}

## nextFrame(cb)

渲染下一帧画面后执行回调

Callback: cb()

* @arg `cb` {[`Function`]}


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

[`Display`]: display_port.md#class-displayport
[`Mat4`]: value.md#class-mat4
[`Vec2`]: value.md#class-vec2
[`Color`]: value.md#class-color

[`app.displayPort`]: app.md#get-guiapplication-displayport
[`gui.displayPort`]: noug.md#get-displayport

[`Orientation`]: display_port.md#enum-orientation
[`StatusBarStyle`]: display_port.md#enum-statusbarstyle
