# `noug/ctr`

## static ViewController::render(vdom[,parentView) 

* 通过`vdom`创建视图或视图控制器`DOM对像`

* @arg `vdom` {Object} 			`VDOM`描述数据
* @arg `[parentView]` {[`View`]} 	传入父视图时将新创建的视图加入到它的结尾
* @ret {[`View`]|[`ViewController`]}

Example:

```jsx
import { Application, ViewController, Root, Div } from 'noug'
import 'noug/http'
class MyCtr extends ViewController {
	triggerLoad(e) {
		http.get('http://192.168.1.100:1026/README.md?param=' + this.message.param, bf=>(this.modle = {bf}));
	}
	render() {
		return (
			<Div width=100 height=100 backgroundColor="#f00">
				{this.modle.bf&&this.modle.bf.toString('utf8')}
			</Div>
		)
	}
}
new Application().start(
	<Root>
		<MyCtr message={param:10} />
	</Root>
);
```

## `Class: ViewController`
* `extends` [`Notification`]

### ViewController.onLoad

* 开始第一次调用render时触发

### ViewController.onMounted

* 第一次完成render调用后触发

### ViewController.onRemove

* 调用删除remove()时触发

### ViewController.onRemoved

* 调用删除remove()后触发

### ViewController.onUpdate

* DOM内容被更新时触发

### ViewController.modle 

* 控制器视图模型

* {[`Object`]}

### Get: ViewController.owner 

* 获取父控制器

* {[`ViewController`]}

### Get: ViewController.dom 

* 获取当前控制器绑定的DOM对像

* {[`View`]|[`ViewController`]}

### ViewController.id 

* 获取或设置一个`id`，这个`id`在同一个控制器内部不能重复

* 可通过`IDs.id`在父视图控制器中查询子控制器

* {[`String`]}

### Get: ViewController.IDs

* 控制器内部所有具名id子dom的索引

* {[`Object`]}

### Get: ViewController.isLoaded

* 触发onLoad后会设置成`true`

* {[`Boolean`]}

### Get: ViewController.isMounted

* 控制器挂载后会设置在`true`

* {[`Boolean`]}

### ViewController.render(...vdoms)

* 通过`vdom`数据载入视图，这是个相当重要的方法，所有`gui`视图载入创建以及视图数据的绑定都在这个方法中完成，

	重写[`ViewController`]类与该方法来实现自定义组件。

* @arg `vdom` {[`VDOM`]}

* @ret {VDOM}

### ViewController.style
* --> [`View.style`]

### ViewController.remove()

* 删除控制器,同时会触发`onRemove`事件

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

[`View`]: noug.md#class-view
[`ViewController`]: ctr.md#class-viewcontroller
[`Notification`]: event.md#class-notification
[`View.action`]: noug.md#get-view-action
[`View.style`]: noug.md#view-style
[`View.visible`]: noug.md#view-visible
[`View.receive`]: noug.md#view-receive
[`View.class`]: noug.md#get-view-class
[`View.transition()`]: noug.md#view-transition-style-delay-cb-
[`View.show()`]: noug.md#view-show-
[`View.hide()`]: noug.md#view-hide-
[`View.addClass()`]: noug.md#view-addClass-name-
[`View.removeClass()`]: noug.md#view-removeclass-name-
[`View.toggleClass()`]: noug.md#view-toggleclass-name-
[`View.remove()`]: noug.md#view-remove-
