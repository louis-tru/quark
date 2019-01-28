# `qgr/action`


## create(json[,parent])

* 通过`json`数据创建动作，如果传入的`json`为[`Action`]跳过创建过程
	
	如果传入父动作，创建完成追加新创建的动作到`parent`结尾

* 如果传入的数据是一个[`Array`]创建[`KeyframeAction`]并使用这个[`Array`]创建[`Frame`]

* 如果传入的数据里有`seq`属性创建[`SequenceAction`]

* 如果传入的数据里有`spawn`属性创建[`SpawnAction`]

* 如果传入的数据里没有`seq`也没`spawn`创建[`KeyframeAction`]，

	对像的内部属性`keyframe`如果为[`Array`]，那么用这个[`Array`]创建[`Frame`]

* @arg `json` {[`Object`]}
* @arg `[parent]` {[`GroupAction`]} 
* @ret {[`Action`]}

Example:

```js
var act1 = action.create([
	{ time:0, x:0 },
	{ time:1000, x:100 },
]);
var act1 = action.create({
	delay: 1000,
	keyframe: [
		{ time:0, x:0, curve: 'linear', },
		{ time:1000, x:100 },
	]
});
// 创建SequenceAction并有两子KeyframeAction
var act2 = action.create({
	loop: -1,
	seq: [
		{
			keyframe: [
				{ time:0, x: 0 },
				{ time:1000, x: 100 },
			]
		},
		[
			{ time:0, x: 100 },
			{ time:1000, x: 0 },
		]
	]
})

```


## transition(view,style[,delay[,cb]])
## transition(view,style[,cb])

* 通过样式创建视图样式过渡动作并播放这个动作，完成后回调

Callback: cb()

* @arg `view` 	{[`View`]}
* @arg `style`  {[`Object`]}
* @arg `[delay]`  {[`uint`]} `ms`
* @arg `[cb]`     {[`Function`]}
* @ret {[`KeyframeAction`]}

Example:

```js
// 1秒后过渡完成并回调
action.transition(view, {
	time: 1000,
	y: 100, 
	x: 100,
}, ()={
	console.log('view transition end');
})
// 延时1秒后开始播放，并使用线性过渡
action.transition(view2, {
	time: 1000,
	curve: 'linear',
	y: 100, 
	x: 100,
}, 1000)
```


## `Class: Action`
* `abstract class`

* 动作基础类型，这是个抽象类型没有构造函数

### Action.play()

* 播放动作

### Action.stop()

* 停止动作

### Action.seek(time)

* 跳转到目标`time`时间，调用后会重置`loopd`

* 不包含延时时间，如果想在延时之前，可传入负数`time`

* @arg `time` {[`int`]} `ms`

### Action.seekPlay(time)

* 跳转到目标`time`时间，并开始播放，调用后会重置`loopd`

* @arg `time` {[`int`]} `ms`

### Action.seekStop(time)

* 跳转到目标`time`时间，并停止播放，调用后会重置`loopd`

* @arg `time` {[`int`]} `ms`

### Action.clear()

* 清空动作,清空动作后会立即停止动作

### Action.loop 

* 动作循环播放的次数，`-1`表示无限循环

* {[`int`]}

### Get: Action.loopd 

* 当前动作已经循环播放的次数

* {[`uint`]}

### Action.delay 

* 延时播放

* {[`uint`]} `ms`

### Get: Action.delayd 

* 延时过去的时间

* {[`uint`]} `ms`

### Action.speed 

* 播放速率，默认为`1.0`，可设置的范围在`0.1`到`10.0`之间

* {[`float`]} `0.1-10`

### Action.playing 

* 是否播放中，设置`action.playing = true`相当调用`action.play()`

* {[`bool`]}

### Get: Action.duration 

* 当前动作的时长不包括延时，对于[`SpawnAction`]取最长的子动作

* {[`uint`]} `ms`

### Get: Action.parent 

* 父动作，如果没有父动作返回`null`

* {[`GroupAction`]}


## `Class: GroupAction`
* `abstract class`
* `extends` [`Action`]

* 动作集合，这是个抽象类型没有构造函数

### Get: GroupAction.length 

* {[`uint`]}
	
### GroupAction.append(child)

* 追加子动作到结尾

* @arg `child` {[`Action`]}
 	
### GroupAction.insert(index, child)

* 插入子动作到`index`的位置

* @arg `index` {[`uint`]}	
* @arg `child` {[`Action`]}
 	
### GroupAction.removeChild(index)

* 通过`index`删除子动作

* @arg `index` {[`uint`]}
 	
### GroupAction.children(index)

* 获取子动作

* @arg `index` {[`uint`]}
* @ret {[`Action`]}


## `Class: SpawnAction`
* `extends` [`GroupAction`]

* 并行子动作实现，所有的动作并行一起播放


## `Class: SequenceAction`
* `extends` [`GroupAction`]

* 串行子动作实现，子动作一个接一个串行播放


## `Class: KeyframeAction`
* `extends` [`Action`]

* 关键帧动作, 不能包含子动作

### KeyframeAction.hasProperty(name)

* 测试当前是否添加了属性名`name`

* @arg `name` {[`PropertyName`]} 
* @ret {[`bool`]}

### KeyframeAction.matchProperty(name)

* 测试属性名称是否与当前绑定的视图匹配

* @arg `name` {[`PropertyName`]} 
* @ret {[`bool`]}

### KeyframeAction.frame(index)

* 通过索引获取关键帧

* @arg `index` {[`uint`]}
* @ret {[`Frame`]}

### KeyframeAction.add([time[,curve]])

* 通过`time`时间与曲线`curve`添加关键帧，并返回关键帧

* arg `[time=0]` {[`uint`]} 不传入`time`默认为`0`
* arg `[curve='ease']` {[`Curve`]} 不传入`curve`默认为[`EASE`]
	
	可使用 [`LINEAR`]、[`EASE`]、[`EASE_IN`]、[`EASE_OUT`]、[`EASE_IN_OUT`]

	或	 `'linear'`、`'ease'`、`'easeIn'`、`'easeOut'`、`'easeInOut'` 做为参数。

* @ret {[`Frame`]}

### KeyframeAction.add([style])

* 通过`style`对像属性添加关键帧，并返回关键帧

* arg `[style]` {[`Object`]}
* @ret {[`Frame`]}

### Get: KeyframeAction.first 

* 第一个关键帧

* {[`Frame`]}

### Get: KeyframeAction.last 

* 最后一个关键帧

* {[`Frame`]}

### Get: KeyframeAction.length 

* 关键帧数量

* {[`uint`]}

### Get: KeyframeAction.position 

* 当前关键帧的播放位置，`-1`表示还未开始播放

* {[`int`]} 

### Get: KeyframeAction.time 

* 当前播放时间`time`

* {[`uint`]} `ms`



## `Class: Frame`

* 关键帧

### Frame.fetch([view]) 

* 通过视图抓取样式属性填充到当前`frame`
	
	如果不传入视图抓取当前绑定的视图样式属性

* @arg `[view]` {[`View`]}

### Frame.flush() 

* 恢复当前关键帧样式属性为默认值

### Get: Frame.index 

* 关键帧所在的动作中的索引位置

* {[`uint`]}

### Frame.time 

* 关键帧的所在动作中的时间`time`

* {[`uint`]} `ms`

### Get: Frame.host 

* 关键帧所在的动作[`KeyframeAction`]

* {[`KeyframeAction`]}

### Frame.curve 

* 当前关键帧到下一个关键帧的过渡曲线

	可使用 [`LINEAR`]、[`EASE`]、[`EASE_IN`]、[`EASE_OUT`]、[`EASE_IN_OUT`]

	或	 `'linear'`、`'ease'`、`'easeIn'`、`'easeOut'`、`'easeInOut'` 做为值设置。

* {[`Curve`]}

### Frame.translate 

* {[`Vec2`]}

### Frame.scale 

* {[`Vec2`]}

### Frame.skew 

* {[`Vec2`]}

### Frame.origin 

* {[`Vec2`]}

### Frame.margin 

* {[`Value`]}

### Frame.border 

* {[`Border`]}

### Frame.borderWidth 

* {[`float`]}

### Frame.borderColor 

* {[`Color`]}

### Frame.borderWadius 

* {[`float`]}

### Frame.minWidth 

* {[`Value`]}

### Frame.minHeight 

* {[`Value`]}

### Frame.start 

* {[`Vec2`]}

### Frame.ratio 

* {[`Vec2`]}

### Frame.width 

* {[`Value`]|[`float`]}

### Frame.height 

* {[`Value`]|[`float`]}

### Frame.x 

* {[`float`]}

### Frame.y 

* {[`float`]}

### Frame.scaleX 

* {[`float`]}

### Frame.scaleY 

* {[`float`]}

### Frame.skewX 

* {[`float`]}

### Frame.skewY 

* {[`float`]}

### Frame.originX 

* {[`float`]}

### Frame.originY 

* {[`float`]}

### Frame.rotateZ 

* {[`float`]}

### Frame.opacity 

* {[`float`]}

### Frame.visible 

* {[`bool`]}

### Frame.marginLeft 

* {[`Value`]}

### Frame.marginTop 

* {[`Value`]}

### Frame.marginRight 

* {[`Value`]}

### Frame.marginBottom 

* {[`Value`]}

### Frame.borderLeft 

* {[`Border`]}

### Frame.borderTop 

* {[`Border`]}

### Frame.borderRight 

* {[`Border`]}

### Frame.borderBottom

* {[`Border`]}

### Frame.borderLeftWidth 

* {[`float`]}

### Frame.borderTopWidth 

* {[`float`]}

### Frame.borderRightWidth 

* {[`float`]}

### Frame.borderBottomWidth 

* {[`float`]}

### Frame.borderLeftColor 

* {[`Color`]}

### Frame.borderTopColor 

* {[`Color`]}

### Frame.borderRightColor 

* {[`Color`]}

### Frame.borderBottomColor 

* {[`Color`]}

### Frame.borderRadiusLeftTop 

* {[`float`]}

### Frame.borderRadiusRightTop 

* {[`float`]}

### Frame.borderRadiusRightBottom 

* {[`float`]}

### Frame.borderRadiusLeftBottom 

* {[`float`]}

### Frame.backgroundColor 

* {[`Color`]}

### Frame.newline 

* {[`bool`]}

### Frame.contentAlign 

* {[`ContentAlign`]}

### Frame.textAlign 

* {[`TextAlign`]}

### Frame.maxWidth 

* {[`Value`]}

### Frame.maxHeight 

* {[`Value`]}

### Frame.startX 

* {[`float`]}

### Frame.startY 

* {[`float`]}

### Frame.ratioX 

* {[`float`]}

### Frame.ratioY 

* {[`float`]}

### Frame.repeat 

* {[`Repeat`]}

### Frame.textBackgroundColor 

* {[`TextColor`]}

### Frame.textColor 

* {[`TextColor`]}

### Frame.textSize 

* {[`TextSize`]}

### Frame.textStyle 

* {[`TextStyle`]}

### Frame.textFamily 

* {[`TextFamily`]}

### Frame.textLineHeight 

* {[`TextLineHeight`]}

### Frame.textShadow 

* {[`TextShadow`]}

### Frame.textDecoration 

* {[`TextDecoration`]}

### Frame.textOverflow 

* {[`TextOverflow`]}

### Frame.textWhiteSpace 

* {[`TextWhiteSpace`]}

### Frame.alignX 

* {[`Align`]}

### Frame.alignY 

* {[`Align`]}

### Frame.shadow 

* {[`Shadow`]}

### Frame.src 

* {[`String`]}


## LINEAR
## EASE
## EASE_IN
## EASE_OUT
## EASE_IN_OUT


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

[`Action`]: action.md#class-action
[`GroupAction`]: action.md#class-groupaction
[`SpawnAction`]: action.md#class-spawnaction
[`SequenceAction`]: action.md#class-sequenceaction
[`KeyframeAction`]: action.md#class-keyframeaction
[`Frame`]: action.md#class-frame
[`View`]: qgr.md#class-view
[`Curve`]: value.md#class-curve
[`LINEAR`]: action.md#linear
[`EASE`]: action.md#ease
[`EASE_IN`]: action.md#ease_in
[`EASE_OUT`]: action.md#ease_out
[`EASE_IN_OUT`]: action.md#ease_in_out
[`PropertyName`]: css.md#enum-propertyname

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
[`Value`]: value.md#class-value
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
