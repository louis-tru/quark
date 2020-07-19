# `pkg`

## Get: mainPackage


## Get: main

主启动运行文件路径


## Get: pkgs 

获取package名称列表


## hasPackage(name)

是否有这个pkg

* @arg `name` {[`String`]}


## getPackage(name) 

通过名称获取pkg实体

* @arg `name` {[`String`]}


## getPackageWithAbsolutePath(path)

通过绝对路径获取pkg内部相对路径,没有找到返回null

* @arg `path` {[`String`]}


## addNodePath(node_modules) 

* @arg `node_modules` {[`String`]}


## addPackage(packagePath)

* @arg `packagePath` {[`String`]}


## setOrigin(path[,origin])

* @arg `path` {[`String`]}
* @arg `[origin]` {[`String`]}


## disableOrigin(path)

* @arg `path` {[`String`]}


## load(packageNames, cb)

* @arg `packageNames` {[`String`]}
* @arg `cb` {[`Function`]}

Asynchronous mode load packages info and ready


## Get: options

启动参数


## extendEntries(obj, extd)

* @arg `obj` {[`Object`]}
* @arg `extd` {[`Object`]}
* @ret {[`Object`]}

## resolve(...args)

* @arg `...args` {[`String`]}
* @ret {[`String`]}


## isAbsolute(path)

* @arg `path` {[`String`]}
* @ret {[`String`]}


## isLocal(path)

* @arg `path` {[`String`]}
* @ret {[`String`]}


## isLocalZip(path)

* @arg `path` {[`String`]}
* @ret {[`String`]}

* @arg `path` {[`String`]}
* @ret {[`String`]}


## isNetwork(path)

* @arg `path` {[`String`]}
* @ret {[`String`]}


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
