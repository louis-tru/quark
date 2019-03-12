# `qgr/ctr`

## empty

* 这是一个常量空`view xml`，描述一个无子视图的[`View`]

## isEmpty(vx)

* `vx`是否等于`empty`空视图数据

## New(vx[,parent[,...args]]) 
## New(vx[,...args])

* 通过`vx`描述数据创建视图或视图控制器

* @arg `vx` {Object} 			`view xml`描述数据
* @arg `[parent]` {View} 	传入父视图时将新创建的视图加入到它的结尾
* @arg `[...args]`				视图的构造参数
* @ret {[`View`]|[`ViewController`]}

Example:

```jsx
import { GUIApplication, ViewController, Root, Div } from 'qgr'
import 'qgr/http'
class MyCtr extends ViewController {
	loadView() {
		http.get('http://192.168.1.100:1026/README.md?param=' + this.message.param, bf=>{
			super.loadView(
				<Div width=100 height=100 backgroundColor="#f00">
					${bf.toString('utf8')}
				</Div>
			)
		})
	}
}
new GUIApplication().start(
	<Root>
		<ViewController vdata={ divWidth: 200, divCon: 'Hello' }>
			<Div width=%{vd.divWidth} height=100 backgroundColor="#000">
				%{vd.divCon}
			</Div>
		</ViewController>
		<MyCtr message={param:10} />
	</Root>
);

```

## `Class: ViewController`
* `extends` [`Notification`]

视图控制器，与视图绑定后视图变成关键视图，当前控制器有可称为成员视图控制器，

关键视图下面所有后代视图以及子视图控制器都属于这个作用域中的成员，

成员视图的`top`属性都指向当前绑定的关键视图,成员视图的`owner`以及子视图控制器的`parent`都指向当前视图控制器，

如果这些成员有具名的`id`,可以通过当前视图控制器`find(id)`找到这些成员

### ViewController.onLoadView

* 载入视图完成时触发

### ViewController.onViewData

* 视图数据变化时触发，如果有视图关注这个数据，那么它也会发生改变

### ViewController.onRemoveView

* 绑定的视图从父视图删除时触发

### ViewController.constructor([msg])

* 构造函数

* @arg `[msg]` {[`Object`]} 传入可选的消息对像

### ViewController.find(id)

* 通过`id`查找成员视图或成员控制器

* @arg `id` {[`String`]}
* @ret {[`View`]|[`ViewController`])

### ViewController.message 

* 控制器消息对像

* {[`Object`]}

### ViewController.vdata 

* 控制器视图数据

* {[`Object`]}

### Get: ViewController.parent 

* 获取父控制器

* {[`ViewController`]}

### ViewController.view 

* 获取或设置当前控制器绑定的视图

* {[`View`]}

### ViewController.id 

* 获取或设置一个`id`，这个`id`在同一个作用域中不能重复

* 可通过`id`在父视图控制器中查询子控制器

* {[`uint`]}

### ViewController.loadView(vx)

* 通过`vx`数据载入视图，这是个相当重要的方法，所有`gui`视图载入创建以及视图数据的绑定都在这个方法中完成，

	重写[`ViewController`]类与该方法来实现自定义组件。

* 这个方法调用完成会触发`onloadView`事件

* @arg `vx` {[`Object`]}

### `View proxy events`

* 这些事件都为代理视图的快捷方式，需要绑定的视图支持这些事件

#### ViewController.onBack
#### ViewController.onClick
#### ViewController.onTouchStart
#### ViewController.onTouchMove
#### ViewController.onTouchEnd
#### ViewController.onTouchCancel
#### ViewController.onKeyDown
#### ViewController.onKeyPress
#### ViewController.onKeyUp
#### ViewController.onKeyEnter
#### ViewController.onFocus
#### ViewController.onBlur
#### ViewController.onHighlighted
#### ViewController.onFocusMove
#### ViewController.onScroll
#### ViewController.onActionKeyframe
#### ViewController.onActionLoop
#### ViewController.onWaitBuffer
#### ViewController.onReady
#### ViewController.onStartPlay
#### ViewController.onError
#### ViewController.onSourceEof
#### ViewController.onPause
#### ViewController.onResume
#### ViewController.onStop
#### ViewController.onSeek

### `View proxy methods and properties`

* 代理视图的快捷方法与属性

#### ViewController.action
* --> [`View.action`]

#### ViewController.style
* --> [`View.style`]

#### ViewController.visible
* --> [`View.visible`]

#### ViewController.receive
* --> [`View.receive`]

#### ViewController.class
* --> [`View.class`]

#### ViewController.transition(style[,delay[,cb]])
#### ViewController.transition(style[,cb])
* --> [`View.transition()`]

#### ViewController.show()
* --> [`View.show()`]

#### ViewController.hide()
* --> [`View.hide()`]

#### ViewController.addClass(name)
* --> [`View.addClass()`]

#### ViewController.removeClass(name)
* --> [`View.removeClass()`]

#### ViewController.toggleClass(name)
* --> [`View.toggleClass()`]

#### ViewController.remove()
* --> [`View.remove()`]


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

[`View`]: qgr.md#class-view
[`ViewController`]: ctr.md#class-viewcontroller
[`Notification`]: event.md#class-notification
[`View.action`]: qgr.md#get-view-action
[`View.style`]: qgr.md#view-style
[`View.visible`]: qgr.md#view-visible
[`View.receive`]: qgr.md#view-receive
[`View.class`]: qgr.md#get-view-class
[`View.transition()`]: qgr.md#view-transition-style-delay-cb-
[`View.show()`]: qgr.md#view-show-
[`View.hide()`]: qgr.md#view-hide-
[`View.addClass()`]: qgr.md#view-addClass-name-
[`View.removeClass()`]: qgr.md#view-removeclass-name-
[`View.toggleClass()`]: qgr.md#view-toggleclass-name-
[`View.remove()`]: qgr.md#view-remove-
