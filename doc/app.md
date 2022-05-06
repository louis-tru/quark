# `noug/app`


## `Class: Application`
* `extends` [`Notification`]

### Application.onLoad
### Application.onUnload
### Application.onBackground
### Application.onForeground
### Application.onPause
### Application.onResume
### Application.onMemoryWarning

内存不足时触发，触发后会自动调用[`clear()`]进行资源清理并执行`js`垃圾收集

### Application.constructor([options])
* @arg `[options]` {[`Options`]}

Example:

```jsx
import Application from 'noug/app'
import Root from 'noug'

var opts = { anisotropic: true, mipmap: true, multisample: 2 };

// 注意: jsx语法只能出现在.jsx文件中
new Application(opts).start(<Root>Hello</Root>).onLoad = function () {
	// my code ...
	console.log('Hello!');
};
```

### Application.start(vx)

通过`vx`视图xml数据启动应用程序

* @arg `vx` {[`Object`]}
* @ret {[`Application`]}

### Application.clear() 

清理释放一些不常用到的资源

### Application.openUrl(url)

调用后会打开系统浏览器并跳转到`url`

* @arg `url` {[`String`]}

### Application.sendEmail(recipient,subject[,cc[,bcc[,body]]])

调用后会打开系统邮件邮件客户端的发送界面,并填充传入的参数。

多个接收人使用逗号分割。

* @arg `recipient` {[`String`]}
* @arg `subject` {[`String`]}
* @arg `[cc]` {[`String`]}
* @arg `[bcc]` {[`String`]}
* @arg `[body]` {[`String`]}


### Application.maxTextureMemoryLimit()

获取纹理数据的最大内存限制值

* @ret {[`uint64`]}

### Application.setMaxTextureMemoryLimit(limit)

设置纹理数据的最大内存限制值，系统初始化默认为`512MB`，当纹理数据内存占用超过这个值，会根据数据的使用频率进行清理。

* @arg `limit` {[`uint64`]}

### Application.usedTextureMemory()

返回纹理数据使用的内存大小，这包括字体纹理数据与图像纹理数据

* @ret {[`uint64`]}

### Get: Application.isLoad 

是否已载入完成

* {[`bool`]}

### Get: Application.displayPort 

* {[`Display`]}

### Get: Application.root 

* {[`Root`]}

### Get: Application.focusView 

获取第一响应者，即当前焦点

* {[`View`]}

### Application.defaultTextBackgroundColor 

默认文本背景颜色

* {[`TextColor`]}

### Application.defaultTextColor 

默认文本颜色

* {[`TextColor`]}

### Application.defaultTextSize 

默认文本尺寸

* {[`TextSize`]}

### Application.defaultTextStyle 

默认文本样式

* {[`TextStyle`]}

### Application.defaultTextFamily 

默认文本字体家族

* {[`TextFamily`]}

### Application.defaultTextShadow 

默认文本阴影

* {[`TextShadow`]}

### Application.defaultTextLineHeight 

默认文本行高

* {[`TextLineHeight`]}

### Application.defaultTextDecoration 

默认文本装饰

* {[`TextDecoration`]}

### Application.defaultTextOverflow 

默认文本溢出选项

* {[`TextOverflow`]}

### Application.defaultTextWhiteSpace 

默认文本处理空格方式

* {[`TextWhiteSpace`]}


## `Object: Options`

* 创建`Application`的选项，这是个`Object`类型描述并没有实际存在的构造函数

### multisample

* 0-4 Level 启用多重采样抗锯齿 `0`不启用，`4`为最大

* 启用后有非常明显的抗锯齿效果，但会消耗非常多的绘图性能

* {[`uint`]} 


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

[`Notification`]: event.md#class-notification
[`Application`]: app.md#class-guiapplication
[`Options`]: app.md#object-options
[`TextColor`]: value.md#class-textcolor
[`TextSize`]: value.md#class-textsize
[`TextStyle`]: value.md#class-textstyle
[`TextFamily`]: value.md#class-textfamily
[`TextShadow`]: value.md#class-textshadow
[`TextLineHeight`]: value.md#class-textlineheight
[`TextDecoration`]: value.md#class-textdecoration
[`TextOverflow`]: value.md#class-textoverflow
[`TextWhiteSpace`]: value.md#class-textwhitespace
[`Display`]: display_port.md#class-displayport
[`Root`]: noug.md#class-root
[`View`]: noug.md#class-view
[`clear()`]: app.md#guiapplication-clear
