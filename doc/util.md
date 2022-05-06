# `noug/util`

## version()

获取`Noug`版本号字符串

* @ret {[`String`]}

## hashCode(string)

获取字符串的哈希代码值

* @arg `string` {[`String`]}
* @ret {[`int`]}

## hash(string)

获取字符串的哈希值，与`hashCode()`的区别在于，这个方法会将哈希代码转换为字符串

* @arg `string` {[`String`]}
* @ret {[`String`]}

## id()

获取自增`id`每次调用都会将这个`id`加`1`并返回

## runScript(source,name[,sandbox])

编译运行一段javascript代码并返回运行结果,与`eval`函数类似,但这个方法可以指定一个名称与一个运行上下文对像

如果要执行Noug `js` or `jsx` 代码需先使用 `util.transformJs()` or `util.transformJsx()` 进行转换

名称在调试代码或程序发生异常时非常有用

如果不传入这个沙盒`sandbox`上下文默认使用`global`对像

* @arg `source` {[`String`]} javascript源代码字符串
* @arg `name` {[`String`]} 必需要指定这个名称
* @arg `[sandbox]` {[`Object`]}
* @ret {[`Object`]} 返回执行结果

## nextTick(cb)

## transformJsx(source,name)

转换Noug `jsx`代码为普通的可运行的`js`代码

转换失败会抛出异常,成功则返回新的代码

* @arg `source` {[`String`]}
* @arg `name` {[`String`]}
* @ret {[`String`]}

## transformJs(source,name)

转换Noug `js`代码为普通的可运行的`js`代码

转换失败会抛出异常,成功则返回新的代码

* @arg `source` {[`String`]}
* @arg `name` {[`String`]}
* @ret {[`String`]}

## noop()

空操作,调用后什么也不会做

## extendObject(obj, extd)

递归`extd`对像属性并扩展到`obj`对像上

* @arg `obj` {[`Object`]}
* @arg `extd` {[`Object`]}
* @ret {[`Object`]} 返回`obj`对像

## assign(obj, ...extd)

把`extd`是的属性扩展到`obj`对像上

* @arg `obj` {[`Object`]}
* @arg `...extd` {[`Object`]}
* @ret {[`Object`]} 返回`obj`对像

## extend(obj, ...extd)

把`extd`是的属性扩展到`obj`对像上,assign相同,但这个函数能扩展属性访问器

* @arg `obj` {[`Object`]}
* @arg `...extd` {[`Object`]}
* @ret {[`Object`]} 返回`obj`对像

## update(obj, extd)

把`extd`是的属性更新到`obj`对像上

会忽略`obj`对像上不存在的属性或类型不相同的属性

* @arg `obj` {[`Object`]}
* @arg `extd` {[`Object`]}
* @ret {[`Object`]} 返回`obj`对像

## err(err)

创建一个`Error`异常对像,可以通过异常字符串创建也可通过`Object`对像创建

如果传入参数已经是一个`Error`对像,不做任何处理立即返回

* @arg `err` {[`String`]|[`Object`]|[`Error`]}
* @ret {[`Error`]}

Example:

```js
var e = util.err('Err')
var e2 = util.err({message:'Err'})
var e3 = util.err(e);
throw e;
```

## throw(err[,cb])

抛出一个异常,如果传入`cb`会抛出一个回调异常

* @arg `err` {[`String`]|[`Object`]|[`Error`]}
* @arg `[cb]` {[`Function`]}

## cb([cb])

返回回调,不传入`cb`会返回一个空函数，如果传入了`cb`参数会立即返回并不会判断其类型

* @arg `[cb]` {[`Function`]}
* @ret {[`Function`]}

## isDefaultThrow(func)

测试回调函数的异常处理函数是否为默认

* @arg `cb` {[`Function`]}
* @ret {[`bool`]}

Example:

```js
// Prints: true
console.log(util.isDefaultThrow(function(){ }));
// Prints: false
console.log(util.isDefaultThrow(function(){ }.catch(e=>{ }));
```

## get(name[,that])

通过点记名称获取对像属性

* @arg `name` {[`String`]]	名称
* @arg `[that=global]` {[`Object`]}  不传入此参数**默认为**`global`

## set(name,value[,that])

通过点记名称设置属性值，并返回被更改的对像

* @arg `name` {[`String`]}
* @arg `value` {[`Object`]}
* @arg `[that=global]` {[`Object`]}
* @ret {[`Object`]}

## del(name[,that])

通过点记名称删除对像属性

* @arg `name` {[`String`]}
* @arg `[that=global]` {[`Object`]}

Example:

```js
var that = { a:{ b: { c: 100 } } };
// Prints: 100
console.log(util.get('a.b.c', that))
// Prints: 
// {
//	 c: 100,
//	 c1: 200	
// }
// {
//	 c: 100,
//	 c1: 200	
// }
console.log(util.set('a.b.c1', 200, that))
console.log(that.a.b);
// Prints: 
// {
//	 c: 100
// }
util.del('a.b.c1', that)
console.log(that.a.b);
```

## random([start,[end]])

获取在`start`到`end`之间的随机数

* @arg `[start=0]` {[`int`]}
* @arg `[end=1e8]` {[`int`]}
* @ret {[`int`]}

## fixRandom(chance[,...chances])

通过概率随机获取从`0`到传入概率数量`arguments.length`的随机数

传入的概率之和不能为`0`

* @arg `chance` {[`float`]}
* @arg `[...chances]` {[`float`]}
* @arg {[`uint`]}

Example:

```js
// Prints: 3 5 9
console.log(util.random(0, 10))
console.log(util.random(0, 10))
console.log(util.random(0, 10))
// Prints 0 3 2
console.log(util.fixRandom(10, 20, 30, 40))
console.log(util.fixRandom(10, 20, 30, 40))
console.log(util.fixRandom(10, 20, 30, 40))
```

## clone(obj)

深度克隆`obj`对像

* @arg `obj` {[`Object`]}
* @ret {[`Object`]}

## wrap(obj)

包裹`obj`对像与`Object.create()`功能相同

* @arg `obj` {[`Object`]}
* @ret {[`Object`]}

## extendClass(cls, extd)

扩展`Class`

* @arg `cls` {[`Class`]}
* @arg `extd` {[`Class`]|[`Object`]}

Example:

```js
class A { test() { } }
class B { test2() { } }
util.extendClass(B, A)
util.extendClass(B, { test3: function() { } })
```

## equalsClass(baseclass,subclass)

测试两个`subclass`是否为`baseclass`的子类或相同

* @arg `subclass` {[`Class`]}
* @arg `baseclass` {[`Class`]}
* @ret {[`bool`]}

## select(default,value)

选取值,如果`value`的类型与`default`相同选择`value`否则选择`default`

* @arg `default` {[`Object`]}
* @arg `value` {[`Object`]}
* @ret {[`Object`]}

## filter(obj,filters[,non])

通过名称列表过滤`obj`对像属性,并返回过滤后的新[`Object`]

`non=true`表示反选

* @arg `obj` {[`Object`]}
* @arg `filters` {[`Array`]}
* @arg `[non=false]` {[`bool`]}
* @ret {[`Object`]}

Example:

```js
// Prints:
//{
//  a: "a",
//  c: "c"
//} 
//{
//  b: "b",
//  d: "d"
//}
var obj = { a:'a',b:'b',c:'c', d:'d' };
console.log(util.filter(obj, ['a', 'c'])
console.log(util.filter(obj, ['a', 'c'], true)
```

* {[`Array`]}

## options

解析后的启动参数[`Object`]

* {[`Object`]}

## dev

是否为开发状态

* {[`bool`]}

## config

获取配置文件值

* {[`Object`]}

## timezone

获取当前时区

* {[`int`]}


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

[`Buffer`]: buffer.md#class-buffer
