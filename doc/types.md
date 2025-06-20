# `quark/value`


## `Class: TextAlign`
### TextAlign.value
* [`LEFT`] - 从`左到右`排列并且以`左边`对齐
* [`CENTER`] - 从`左到右`排列并且以`居中`对齐
* [`RIGHT`] - 从`左到右`排列并且以`右边`对齐
* [`LEFT_REVERSE`] - 从`右到左`排列并且以`左边`对齐
* [`CENTER_REVERSE`] - 从`右到左`排列并且以`居中`对齐
* [`RIGHT_REVERSE`] - 从`右到左`排列并且以`右边`对齐


## `Class: Align`
### Align.value
* [`LEFT`] - 水平左对齐
* [`RIGHT`] - 水平右对齐
* [`CENTER`] - 居中对齐
* [`TOP`] - 垂直顶部对齐
* [`BOTTOM`] - 垂直底部对齐
* [`NONE`] - 无


## `Class: ContentAlign`
### ContentAlign.value
* [`LEFT`] - 水平布局，从左到右排列布局，溢出往下换行
* [`RIGHT`] - 水平布局，从右到左排列布局，溢出往下换行
* [`TOP`] - 垂直布局，从上到下排列布局，溢出往右边换行
* [`BOTTOM`] - 垂直布局，从下到上排列布局，溢出往右换行


## `Class: Repeat`
### Repeat.value
* [`NONE`] - 不重复
* [`REPEAT`] - `x`轴与`y`轴都重复
* [`REPEAT_X`] - 只重复`x`轴
* [`REPEAT_Y`] - 只重复`y`轴
* [`MIRRORED_REPEAT`] - 镜像模式`x`轴与`y`轴都重复
* [`MIRRORED_REPEAT_X`] - 镜像模式只重复`x`轴
* [`MIRRORED_REPEAT_Y`] - 镜像模式只重复`y`轴


## `Class: Direction`
### Direction.value
* [`LEFT`] - 左
* [`RIGHT`] - 右
* [`TOP`] - 上
* [`BOTTOM`] - 下


## `Class: KeyboardType`
### KeyboardType.value
* [`NORMAL`] - 
* [`ASCII`] - 
* [`NUMBER`] - 
* [`URL`] - 
* [`NUMBER_PAD`] - 
* [`PHONE`] - 
* [`NAME_PHONE`] - 
* [`EMAIL`] - 
* [`DECIMAL`] - 
* [`TWITTER`] - 
* [`SEARCH`] - 
* [`ASCII_NUMBER`] - 


## `Class: KeyboardReturnType`
### KeyboardReturnType.value
* [`NORMAL`] -
* [`GO`] -
* [`JOIN`] -
* [`NEXT`] -
* [`ROUTE`] -
* [`SEARCH`] -
* [`SEND`] -
* [`DONE`] -
* [`EMERGENCY`] -
* [`CONTINUE`] -


## `Class: Border`
### Border.width
* {[`float`]}

### Border.color
* {[`Color`]}

## `Class: Shadow`
### Shadow.offsetX
* {[`float`]}

### Shadow.offsetY
* {[`float`]}

### Shadow.size
* {[`float`]}

### Shadow.color
* {[`Color`]}


## `Class: Color`
### Color.r
* {[`uint`]}

### Color.g
* {[`uint`]}

### Color.b
* {[`uint`]}

### Color.a
* {[`uint`]}

### Color.reverse()
* @ret {[`Color`]}

### Color.toRgbString()
### Color.toRgbaString()
### Color.toHex32String()


## `Class: Vec2`
### Vec2.x
* {[`float`]}

### Vec2.y
* {[`float`]}


## `Class: Vec3`
### Vec3.x
* {[`float`]}

### Vec3.y
* {[`float`]}

### Vec3.z
* {[`float`]}



## `Class: Vec4`
### Vec4.x
* {[`float`]}

### Vec4.y
* {[`float`]}

### Vec4.z
* {[`float`]}

### Vec4.w
* {[`float`]}



## `Class: Curve`
### Curve.p1X
* {[`float`]}

### Curve.p1Y
* {[`float`]}

### Curve.p2X
* {[`float`]}

### Curve.p2Y
* {[`float`]}

Example:

```js
// 'linear'、'ease'、'easeIn'、'easeOut'、'easeInOut'
var cueceLinear = new Curve(0, 0, 1, 1)
var cueceEase = new Curve(0.25, 0.1, 0.25, 1)
var cueceEaseIn = new Curve(0.42, 0, 1, 1)
var cueceEaseOut = new Curve(0.25, 0.1, 0.25, 1)
var cueceEaseInOut = new Curve(0.25, 0.1, 0.25, 1)
```


## `Class: Rect`
### Rect.x
* {[`float`]}

### Rect.y
* {[`float`]}

### Rect.width
* {[`float`]}

### Rect.height
* {[`float`]}



## `Class: Mat`
### Mat.m0
### Mat.m1
### Mat.m2
### Mat.m3
### Mat.m4
### Mat.m5


## `Class: Mat4`
### Mat4.m0
### Mat4.m1
### Mat4.m2
### Mat4.m3
### Mat4.m4
### Mat4.m5
### Mat4.m6
### Mat4.m7
### Mat4.m8
### Mat4.m9
### Mat4.m10
### Mat4.m11
### Mat4.m12
### Mat4.m13
### Mat4.m14
### Mat4.m15


## `Class: Value`
### Value.type
* [`AUTO`] - 自动
* [`FULL`] - 填满
* [`PIXEL`] - 像素
* [`PERCENT`] - 百分比
* [`MINUS`] - 减法

### Value.value
* {[`float`]}

## `Class: TextAttrsValue`
### TextColor.type
* [`INHERIT`] - 继承父视图
* [`VALUE`] - 明确数值

## `Class: TextColor`
* `extends` [`TextAttrsValue`]

### TextColor.value
* {[`Color`]}


## `Class: TextSize`
* `extends` [`TextAttrsValue`]

### TextSize.value
* {[`float`]}


## `Class: TextFamily`
* `extends` [`TextAttrsValue`]

### TextFamily.value
* {[`String`]}

## `Class: TextStyle`
* `extends` [`TextAttrsValue`]

### TextStyle.value
* [`THIN`] - 100
* [`ULTRALIGHT`] - 200
* [`LIGHT`] - 300
* [`REGULAR`] - 400 正常
* [`MEDIUM`] - 500
* [`SEMIBOLD`] - 600
* [`BOLD`] - 700
* [`HEAVY`] - 800
* [`BLACK`] - 900
* [`THIN_ITALIC`] - 100 斜体 
* [`ULTRALIGHT_ITALIC`] - 200 斜体
* [`LIGHT_ITALIC`] - 300 斜体
* [`ITALIC`] - 400 斜体
* [`MEDIUM_ITALIC`] - 500 斜体
* [`SEMIBOLD_ITALIC`] - 600 斜体
* [`BOLD_ITALIC`] - 700 斜体
* [`HEAVY_ITALIC`] - 800 斜体
* [`BLACK_ITALIC`] - 900 斜体


## `Class: TextShadow`
* `extends` [`TextAttrsValue`]

### TextShadow.value
* {[`Shadow`]}


## `Class: TextLineHeight`
* `extends` [`TextAttrsValue`]

### TextLineHeight.isAuto
* {[`bool`]}

### TextLineHeight.height
* {[`float`]}


## `Class: TextDecoration`
* `extends` [`TextAttrsValue`]

### TextDecoration.value
* [`NONE`] - 无装饰
* [`OVERLINE`] - 上划线
* [`LINE_THROUGH`] - 中划线
* [`UNDERLINE`] -	下划线


## `Class: TextOverflow`
* `extends` [`TextAttrsValue`]

### TextOverflow.value
* [`NORMAL`] - 不做处理
* [`CLIP`] - 修剪
* [`ELLIPSIS`] - 修剪并在文本末尾显示省略号
* [`CENTER_ELLIPSIS`] - 修剪并在文本中间显示省略号


## `Class: TextWhiteSpace`
* `extends` [`TextAttrsValue`]

### TextWhiteSpace.value
* [`NORMAL`] - 保留所有空白,使用自动wrap
* [`NO_WRAP`] - 合并空白序列,不使用自动wrap
* [`NO_SPACE`] - 合并空白序列,使用自动wrap
* [`PRE`] - 保留所有空白,不使用自动wrap
* [`PRE_LINE`] - 合并空白符序列,但保留换行符,使用自动wrap


## parseTextAlign(str)

* @ret {[`TextAlign`]}

## parseAlign(str)

* @ret {[`Align`]}

## parseContentAlign(str)

* @ret {[`ContentAlign`]}

## parseRepeat(str)

* @ret {[`Repeat`]}

## parseDirection(str)

* @ret {[`Direction`]}

## parseKeyboardType(str)

* @ret {[`KeyboardType`]}

## parseKeyboardReturnType(str)

* @ret {[`KeyboardReturnType`]}

## parseBorder(str)

* @ret {[`Border`]}

## parseShadow(str)

* @ret {[`Shadow`]}

## parseColor(str)

* @ret {[`Color`]}

## parseVec2(str)

* @ret {[`Vec2`]}

## parseVec3(str)

* @ret {[`Vec3`]}

## parseVec4(str)

* @ret {[`Vec4`]}

## parseCurve(str)

* @ret {[`Curve`]}

## parseRect(str)

* @ret {[`Rect`]}

## parseMat(str)

* @ret {[`Mat`]}

## parseMat4(str)

* @ret {[`Mat4`]}

## parseValue(str)

* @ret {[`Class: Value`]}

## parseTextColor(str)

* @ret {[`TextColor`]}

## parseTextSize(str)

* @ret {[`TextSize`]}

## parseTextFamily(str)

* @ret {[`TextFamily`]}

## parseTextStyle(str)

* @ret {[`TextStyle`]}

## parseTextShadow(str)

* @ret {[`TextShadow`]}

## parseTextLineHeight(str)

* @ret {[`TextLineHeight`]}

## parseTextDecoration(str)

* @ret {[`TextDecoration`]}

## parseTextOverflow(str)

* @ret {[`TextOverflow`]}

## parseTextWhiteSpace(str)

* @ret {[`TextWhiteSpace`]}



## `Enum: All`

### AUTO
### FULL
### PIXEL
### PERCENT
### MINUS
### INHERIT
### VALUE
### THIN
### ULTRALIGHT
### LIGHT
### REGULAR
### MEDIUM
### SEMIBOLD
### BOLD
### HEAVY
### BLACK
### THIN_ITALIC
### ULTRALIGHT_ITALIC
### LIGHT_ITALIC
### ITALIC
### MEDIUM_ITALIC
### SEMIBOLD_ITALIC
### BOLD_ITALIC
### HEAVY_ITALIC
### BLACK_ITALIC
### NONE
### OVERLINE
### LINE_THROUGH
### UNDERLINE
### LEFT
### CENTER
### RIGHT
### LEFT_REVERSE
### CENTER_REVERSE
### RIGHT_REVERSE
### TOP
### BOTTOM
### MIDDLE
### REPEAT
### REPEAT_X
### REPEAT_Y
### MIRRORED_REPEAT
### MIRRORED_REPEAT_X
### MIRRORED_REPEAT_Y
### NORMAL
### CLIP
### ELLIPSIS
### CENTER_ELLIPSIS
### NO_WRAP
### NO_SPACE
### PRE
### PRE_LINE

### ASCII
### NUMBER
### URL
### NUMBER_PAD
### PHONE
### NAME_PHONE
### EMAIL
### DECIMAL
### TWITTER
### SEARCH
### ASCII_NUMBER

### GO
### JOIN
### NEXT
### ROUTE
### SEND
### DONE
### EMERGENCY
### CONTINUE


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
[`float`]: native_types.md#float
[`bool`]: native_types.md#bool

[`TextAlign`]: value.md#class-textalign
[`Align`]: value.md#class-align
[`ContentAlign`]: value.md#class-contentalign
[`Repeat`]: value.md#class-repeat
[`Direction`]: value.md#class-direction
[`KeyboardType`]: value.md#class-keyboardtype
[`KeyboardReturnType`]: value.md#class-keyboardreturntype
[`Border`]: value.md#class-border
[`Shadow`]: value.md#class-shadow
[`Color`]: value.md#class-color
[`Vec2`]: value.md#class-vec2
[`Vec3`]: value.md#class-vec3
[`Vec4`]: value.md#class-vec4
[`Curve`]: value.md#class-curve
[`Rect`]: value.md#class-rect
[`Mat`]: value.md#class-mat
[`Mat4`]: value.md#class-mat4
[`Class: Value`]: value.md#class-value
[`TextColor`]: value.md#class-textcolor
[`TextSize`]: value.md#class-textsize
[`TextFamily`]: value.md#class-textfamily
[`TextStyle`]: value.md#class-textstyle
[`TextShadow`]: value.md#class-textshadow
[`TextLineHeight`]: value.md#class-textlineheight
[`TextDecoration`]: value.md#class-textdecoration
[`TextOverflow`]: value.md#class-textoverflow
[`TextWhiteSpace`]: value.md#class-textwhitespace
[`TextAttrsValue`]: value.md#class-textattrsvalue
[`AUTO`]: value.md#auto
[`FULL`]: value.md#full
[`PIXEL`]: value.md#pixel
[`PERCENT`]: value.md#percent
[`MINUS`]: value.md#minus
[`INHERIT`]: value.md#inherit
[`VALUE`]: value.md#value
[`THIN`]: value.md#thin
[`ULTRALIGHT`]: value.md#ultralight
[`LIGHT`]: value.md#light
[`REGULAR`]: value.md#regular
[`MEDIUM`]: value.md#medium
[`SEMIBOLD`]: value.md#semibold
[`BOLD`]: value.md#bold
[`HEAVY`]: value.md#heavy
[`BLACK`]: value.md#black
[`THIN_ITALIC`]: value.md#thin_italic
[`ULTRALIGHT_ITALIC`]: value.md#ultralight_italic
[`LIGHT_ITALIC`]: value.md#lightitalic
[`ITALIC`]: value.md#italic
[`MEDIUM_ITALIC`]: value.md#medium_italic
[`SEMIBOLD_ITALIC`]: value.md#semibold_italic
[`BOLD_ITALIC`]: value.md#bold_italic
[`HEAVY_ITALIC`]: value.md#heavy_italic
[`BLACK_ITALIC`]: value.md#black_italic
[`NONE`]: value.md#none
[`OVERLINE`]: value.md#overline
[`LINE_THROUGH`]: value.md#line_through
[`UNDERLINE`]: value.md#underline
[`LEFT`]: value.md#left
[`CENTER`]: value.md#center
[`RIGHT`]: value.md#right
[`LEFT_REVERSE`]: value.md#left_reverse
[`CENTER_REVERSE`]: value.md#center_reverse
[`RIGHT_REVERSE`]: value.md#right_reverse
[`TOP`]: value.md#top
[`BOTTOM`]: value.md#bottom
[`MIDDLE`]: value.md#middle
[`REPEAT`]: value.md#repeat
[`REPEAT_X`]: value.md#repeat_x
[`REPEAT_Y`]: value.md#repeat_y
[`MIRRORED_REPEAT`]: value.md#mirrored_repeat
[`MIRRORED_REPEAT_X`]: value.md#mirrored_repeat_x
[`MIRRORED_REPEAT_Y`]: value.md#mirrored_repeat_y
[`NORMAL`]: value.md#normal
[`CLIP`]: value.md#clip
[`ELLIPSIS`]: value.md#ellipsis
[`CENTER_ELLIPSIS`]: value.md#center_ellipsis
[`NO_WRAP`]: value.md#no_wrap
[`NO_SPACE`]: value.md#no_space
[`PRE`]: value.md#pre
[`PRE_LINE`]: value.md#pre_line
[`ASCII`]: value.md#ascii
[`NUMBER`]: value.md#number
[`URL`]: value.md#url
[`NUMBER_PAD`]: value.md#numberpad
[`PHONE`]: value.md#phonepad
[`NAME_PHONE`]: value.md#name_phone
[`EMAIL`]: value.md#email
[`DECIMAL`]: value.md#decimal
[`TWITTER`]: value.md#twitter
[`SEARCH`]: value.md#search
[`ASCII_NUMBER`]: value.md#ascii_number
[`GO`]: value.md#go
[`JOIN`]: value.md#join
[`NEXT`]: value.md#next
[`ROUTE`]: value.md#route
[`SEND`]: value.md#send
[`DONE`]: value.md#done
[`EMERGENCY`]: value.md#emergency
[`CONTINUE`]: value.md#continue
