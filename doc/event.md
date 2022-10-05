# `quark/event`


## `Enum: HighlightedStatus`

`View.onHighlighted`事件数据的状态枚举类型

### HIGHLIGHTED_NORMAL
### HIGHLIGHTED_HOVER
### HIGHLIGHTED_DOWN


## `Enum: ReturnValueMask`

GUI事件数据返回值掩码，如果返回值被设置成`0`，表示同时取消默认动作与事件冒泡

### RETURN_VALUE_MASK_DEFAULT = (1<<0)
### RETURN_VALUE_MASK_BUBBLE = (1<<1)
### RETURN_VALUE_MASK_ALL


## Emun: KeyboardKeyName

键盘按键对应的按键代码

### KEYCODE_UNKNOWN          = 0
### KEYCODE_MOUSE_LEFT       = 1
### KEYCODE_MOUSE_CENTER     = 2
### KEYCODE_MOUSE_RIGHT      = 3
### KEYCODE_BACK_SPACE       = 8
### KEYCODE_TAB              = 9
### KEYCODE_CLEAR            = 12
### KEYCODE_ENTER            = 13
### KEYCODE_SHIFT            = 16
### KEYCODE_CTRL             = 17
### KEYCODE_ALT              = 18
### KEYCODE_CAPS_LOCK        = 20
### KEYCODE_ESC              = 27
### KEYCODE_SPACE            = 32
### KEYCODE_COMMAND          = 91
### KEYCODE_LEFT             = 37
### KEYCODE_UP               = 38
### KEYCODE_RIGHT            = 39
### KEYCODE_DOWN             = 40
### KEYCODE_INSERT           = 45
### KEYCODE_DELETE           = 46
### KEYCODE_PAGE_UP          = 33
### KEYCODE_PAGE_DOWN        = 34
### KEYCODE_MOVE_END         = 35
### KEYCODE_MOVE_HOME        = 36
### KEYCODE_SCROLL_LOCK      = 145
### KEYCODE_BREAK            = 19
### KEYCODE_SYSRQ            = 124
### KEYCODE_0                = 48
### KEYCODE_1                = 49
### KEYCODE_2                = 50
### KEYCODE_3                = 51
### KEYCODE_4                = 52
### KEYCODE_5                = 53
### KEYCODE_6                = 54
### KEYCODE_7                = 55
### KEYCODE_8                = 56
### KEYCODE_9                = 57
### KEYCODE_A                = 65
### KEYCODE_B                = 66
### KEYCODE_C                = 67
### KEYCODE_D                = 68
### KEYCODE_E                = 69
### KEYCODE_F                = 70
### KEYCODE_G                = 71
### KEYCODE_H                = 72
### KEYCODE_I                = 73
### KEYCODE_J                = 74
### KEYCODE_K                = 75
### KEYCODE_L                = 76
### KEYCODE_M                = 77
### KEYCODE_N                = 78
### KEYCODE_O                = 79
### KEYCODE_P                = 80
### KEYCODE_Q                = 81
### KEYCODE_R                = 82
### KEYCODE_S                = 83
### KEYCODE_T                = 84
### KEYCODE_U                = 85
### KEYCODE_V                = 86
### KEYCODE_W                = 87
### KEYCODE_X                = 88
### KEYCODE_Y                = 89
### KEYCODE_Z                = 90
### KEYCODE_NUM_LOCK         = 144
### KEYCODE_NUMPAD_0         = 96
### KEYCODE_NUMPAD_1         = 97
### KEYCODE_NUMPAD_2         = 98
### KEYCODE_NUMPAD_3         = 99
### KEYCODE_NUMPAD_4         = 100
### KEYCODE_NUMPAD_5         = 101
### KEYCODE_NUMPAD_6         = 102
### KEYCODE_NUMPAD_7         = 103
### KEYCODE_NUMPAD_8         = 104
### KEYCODE_NUMPAD_9         = 105
### KEYCODE_NUMPAD_DIVIDE    = 111
### KEYCODE_NUMPAD_MULTIPLY  = 106
### KEYCODE_NUMPAD_SUBTRACT  = 109
### KEYCODE_NUMPAD_ADD       = 107
### KEYCODE_NUMPAD_DOT       = 110
### KEYCODE_NUMPAD_ENTER     = 108
### KEYCODE_F1               = 112
### KEYCODE_F2               = 113
### KEYCODE_F3               = 114
### KEYCODE_F4               = 115
### KEYCODE_F5               = 116
### KEYCODE_F6               = 117
### KEYCODE_F7               = 118
### KEYCODE_F8               = 119
### KEYCODE_F9               = 120
### KEYCODE_F10              = 121
### KEYCODE_F11              = 122
### KEYCODE_F12              = 123
### KEYCODE_SEMICOLON        = 186
### KEYCODE_EQUALS           = 187
### KEYCODE_MINUS            = 189
### KEYCODE_COMMA            = 188
### KEYCODE_PERIOD           = 190
### KEYCODE_SLASH            = 191
### KEYCODE_GRAVE            = 192
### KEYCODE_LEFT_BRACKET     = 219
### KEYCODE_BACK_SLASH       = 220
### KEYCODE_RIGHT_BRACKET    = 221
### KEYCODE_APOSTROPHE       = 222
### KEYCODE_HOME             = 300
### KEYCODE_BACK             = 301
### KEYCODE_CALL             = 302
### KEYCODE_ENDCALL          = 303
### KEYCODE_STAR             = 304
### KEYCODE_POUND            = 305
### KEYCODE_CENTER           = 306
### KEYCODE_VOLUME_UP        = 307
### KEYCODE_VOLUME_DOWN      = 308
### KEYCODE_POWER            = 309
### KEYCODE_CAMERA           = 310
### KEYCODE_FOCUS            = 311
### KEYCODE_MENU             = 312
### KEYCODE_SEARCH           = 313
### KEYCODE_MEDIA_PLAY_PAU   = 314
### KEYCODE_MEDIA_STOP       = 315
### KEYCODE_MEDIA_NEXT       = 316
### KEYCODE_MEDIA_PREVIOUS   = 317
### KEYCODE_MEDIA_REWIND     = 318
### KEYCODE_MEDIA_FAST_FORWARD= 319
### KEYCODE_MUTE             = 320
### KEYCODE_CHANNEL_UP       = 321
### KEYCODE_CHANNEL_DOWN     = 322
### KEYCODE_MEDIA_PLAY       = 323
### KEYCODE_MEDIA_PAUSE      = 324
### KEYCODE_MEDIA_CLOSE      = 325
### KEYCODE_MEDIA_EJECT      = 326
### KEYCODE_MEDIA_RECORD     = 327
### KEYCODE_VOLUME_MUTE      = 328
### KEYCODE_MUSIC            = 329
### KEYCODE_EXPLORER         = 330
### KEYCODE_ENVELOPE         = 331
### KEYCODE_BOOKMARK         = 332
### KEYCODE_ZOOM_IN          = 333
### KEYCODE_ZOOM_OUT         = 334
### KEYCODE_HELP             = 335


## `Object: GUITouch`

触摸事件返回的数据结构, 这是个`Object`类型描述并没有实际存在的构造函数

### GUITouch.id 
* {[`uint`]}

### GUITouch.startX 
* {[`float`]}

### GUITouch.startY 
* {[`float`]}

### GUITouch.x 
* {[`float`]}

### GUITouch.y 
* {[`float`]}

### GUITouch.force 
* {[`float`]}

### GUITouch.view 
* {[`View`]}



## `Class: Notification`

这是事件`EventNoticer`的集合，事件触发与响应中心

继承于它的派生类型可以使用`event`关键字来声明成员事件

### Notification.trigger(name[,data])

通过事件名称触发事件 --> [`EventNoticer.trigger(data)`]

### Notification.triggerWithEvent(name,event)

通过名称与[`Event`]触发事件 --> [`EventNoticer.triggerWithEvent(event)`]

### allNoticers(notification) 

获取`notification`上的所有[`EventNoticer`]

* @arg `notification` {[`Notification`]}
* @ret {[`Array`]} 返回内容为[`EventNoticer`]的[`Array`]

### removeEventListenerWithScope(notification,scope)

卸载`notification`上所有与`scope`相关的侦听器

实际上遍历调用了[`EventNoticer.off(scope)`]方法

* @arg `notification` {[`Notification`]}
* @arg `scope` {[`Object`]}




Example:

```js
class TestNotification extends Notification {
	eventonChange; // 这里必须以on开始以;分号结束
}

var notification = new TestNotification();
// Prints: responseonChange 0 100
notification.onChange = function(ev) { // add default listener
	console.log('responseonChange 0', ev.data)
}
notification.triggerChange(100);

// Prints: 
// responseonChange 0 200
// responseonChange 1
notification.onChange.on(function(ev) {
	console.log('responseonChange 1')	
})
notification.triggerWithEvent('change', new Event(200));

var noticer = notification.onChange;
noticer.off(0) // delete default listener
// Prints: responseonChange 1
notification.triggerChange();

```

## `Class: EventNoticer`

事件通知者,事件监听器添加与删除、触发与通知都由它实现

### EventNoticer.constructor(name,sender)

* @arg `name` {[`String`]}
* @arg `sender` {[`Object`]}

### EventNoticer.enable

事件是否启用,`noticer.enable=false`可禁用事件的发送,**Default** `true`

* {[`bool`]}

### Get: EventNoticer.name

事件的名称

* {[`String`]}

### Get: EventNoticer.sender

返回初始化时的`sender`对像

* {[`Object`]}

### Get: EventNoticer.length

监听器数量

* {[`uint`]}

### EventNoticer.on(listen[,scope[,id]])

添加监听器函数,并指定scope`this`

* @arg `func` {[`Function`]}		监听器函数
* @arg `[scope]` {[`Object`]}		范围`this`对像
* @arg `[id]` {[`int`]}					指定一个`id`,如果不指定自动生成一个,如果指定的`id`已经存在会替换之前的监听器
* @ret {[`int`]} return `id`		返回传入的`id`或自动生成的`id`

Example:

```js
var scope = { a:100 }
var id = display_port.onChange.on(function(ev) {
// Prints: 100
	console.log(this.a)
}, scope)
// 替换监听器
display_port.onChange.on(function(ev) {
// Prints: replace 100
	console.log('replace', this.a)
}, scope, id)
```

### EventNoticer.once(listen[,scope[,id]])

添加监听器函数,并指定scope`this`,只监听一次

### EventNoticer.on2(listen[,scope[,id]])

添加监听器函数,并指定scope`this`,与`on()`仅回调参数不相同

Example:

```js
var scope = { a:100 }
var id = display_port.onChange.on2(function(scope, ev) {
// Prints: 100
	console.log(scope.a)
}, scope)
```

### EventNoticer.once2(listen[,scope[,id]])

添加监听器函数,并指定scope`this`,只监听一次

### EventNoticer.trigger([data])

触发事件数据,并发送数据

* @arg `[data]` {[`Object`]}
* @ret {[`int`]} return [`Event.returnValue`]

### EventNoticer.triggerWithEvent(event)

通过[`Event`]触发事件

* @arg `event` {[`Event`]}
* @ret {[`int`]} return [`Event.returnValue`]

### EventNoticer.off()

删除所有监听器

### EventNoticer.off(id)

通过`id`删除监听器

* @arg `id`  {[`int`]}

### EventNoticer.off(scope)

通过`scope`删除一个监听器

* @arg `scope`  {[`Object`]}

### EventNoticer.off(listen[,scope])

通过`listen`函数与`scope`删除一个监听器

* @arg `listen` {[`Function`]}
* @arg [`scope`]  {[`Object`]}


## `Class: Event`

事件数据上下文

### Event.constructor([data])

* @arg `[data]` {[`Object`]}

### Get: Event.name

* {[`String`]}

### Get: Event.data

* {[`Object`]}

### Get: Event.sender

* {[`Object`]}

### Get: Event.noticer

* {[`EventNoticer`]}

### Event.returnValue

* {[`int`]}




## `Class: GUIEvent`
* `extends` [`Event`]

GUI事件数据上下文，构造函数为私有不能访问

### Get: GUIEvent.origin

*  {[`View`]}

### Get: GUIEvent.timestamp

*  {[`uint64`]}

### GUIEvent.cancelDefault()

取消默认动作

`returnValue &= ~RETURN_VALUE_MASK_DEFAULT`

### GUIEvent.cancelBubble()

取消事件冒泡

`returnValue &= ~RETURN_VALUE_MASK_BUBBLE`

### Get: GUIEvent.isDefault

`returnValue & RETURN_VALUE_MASK_DEFAULT`

* {[`bool`]}

### Get: GUIEvent.isBubble

`returnValue & RETURN_VALUE_MASK_BUBBLE`

* {[`bool`]}



## `Class: GUIActionEvent`
* `extends` {[`GUIEvent`]}

### Get: GUIActionEvent.action 

* {[`Action`]}

### Get: GUIActionEvent.delay 

* {[`uint64`]}

### Get: GUIActionEvent.frame 

* {[`uint`]}

### Get: GUIActionEvent.loop 

* {[`uint`]}


## `Class: GUIKeyEvent`
* `extends` {[`GUIEvent`]}

### Get: GUIKeyEvent.keycode 

* {[`KeyboardKeyName`]}

### Get: GUIKeyEvent.repeat 

* {[`int`]}

### Get: GUIKeyEvent.shift 

* {[`bool`]}

### Get: GUIKeyEvent.ctrl 

* {[`bool`]}

### Get: GUIKeyEvent.alt 

* {[`bool`]}

### Get: GUIKeyEvent.command 

* {[`bool`]}

### Get: GUIKeyEvent.capsLock 

* {[`bool`]}

### Get: GUIKeyEvent.device 

* {[`int`]}

### Get: GUIKeyEvent.source 

* {[`int`]}

### GUIKeyEvent.focusMove 

* {[`View`]}


## `Class: GUIClickEvent`
* `extends` {[`GUIEvent`]}

### Get: GUIClickEvent.x 

* {[`float`]}

### Get: GUIClickEvent.y 

* {[`float`]}

### Get: GUIClickEvent.count 

* {[`uint`]}

### Get: GUIClickEvent.mode 

触发方式`1=TOUCH, 2=KEYBOARD, 3=MOUSE`

* {[`int`]}


## `Class: GUIHighlightedEvent`
* `extends` {[`GUIEvent`]}

### Get: status 

* {[`HighlightedStatus`]}


## `Class:  GUITouchEvent`
* `extends` {[`GUIEvent`]}

### Get: changedTouches 

* {[`Array`]<[`GUITouch`]>}


## `Class: GUIFocusMoveEvent`
* `extends` {[`GUIEvent`]}

### Get: focus 

变化前的焦点视图

* {[`View`]}

### Get: focusMove 

变化后的焦点视图

* {[`View`]}




[`Class`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Classes
[`Date`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date
[`Error`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Error
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

[`EventNoticer.on`]: event.md#eventnoticer-on-listen-scope-id-
[`EventNoticer.once`]: event.md#eventnoticer-once-listen-scope-id-
[`EventNoticer.on2`]: event.md#eventnoticer-on-listen-scope-id-
[`EventNoticer.once2`]: event.md#eventnoticer-once-listen-scope-id-
[`Notification`]:	event.md#class-notification
[`EventNoticer`]:	event.md#class-eventnoticer
[`Event`]: event.md#class-event
[`Event.returnValue`]: event.md#event-returnvalue
[`EventNoticer.off()`]: event.md#eventnoticer-off-
[`EventNoticer.off(id)`]: event.md#eventnoticer-off-id-
[`EventNoticer.off(scope)`]: event.md#eventnoticer-off-scope-
[`EventNoticer.off(listen,scope)`]: event.md#eventnoticer-off-listen-scope-
[`EventNoticer.trigger(data)`]: event.md#eventnoticer-trigger-data-
[`EventNoticer.triggerWithEvent(event)`]: event.md#eventnoticer-triggerwithevent-event-
[`HighlightedStatus`]: event.md#enum-highlightedstatus
[`KeyboardKeyName`]: event.md#enum-keyboardkeyname
[`Action`]: action.md#class-action
[`GUIEvent`]: event.md#class-guievent
[`View`]: quark.md#class-view
[`GUITouch`]: event.md#object-guitouch

