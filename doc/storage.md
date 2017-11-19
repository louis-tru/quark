# `ngui/storage`

在这里设置的值，退出程序后依然存在，称呼其为本地存储 localStorage 适合小容量数据的快速存取

## get(key)

通过名称获取本地值

* @arg `key` {[`String`]}
* @ret {[`String`]}

## set(key, value)

通过名称设置本地值

* @arg `key` {[`String`]}
* @arg `value` {[`String`]}

## del(key)

通过名称删除本地值

* @arg `key` {[`String`]}

## getJson(key)

通过名称获取本地值json数据

* @arg `key` {[`String`]}
* @ret {[`Object`]}

## setJson(key, value)

通过名称设置本地值json数据

* @arg `key` {[`String`]}
* @arg `value` {[`Object`]}

## delJson(key)

通过名称删除本地值json

* @arg `key` {[`String`]}

## claer()

删除所有本地值,包括普通数据与json数据


[`Object`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object
[`String`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String
