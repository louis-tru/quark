# `langou/path`


## executable()

获取当前应用程序的二进制执行文件路径

* @ret {[`String`]}

Example:

```js
// Prints:
// file:///var/containers/Bundle/Application/4F1BD659-601D-4932-8484-D0D1F978F0BE/test.app/test
console.log(path.executable());
```

## documents([appendPath])

获取当前应用程序的文档存储路径

* @arg `[appendPath='']` {[`String`]} 追加到文档路径后面
* @ret {[`String`]}

Example:

```js
// Prints:
// file:///var/mobile/Containers/Data/Application/89A576FE-7BB9-4F26-A456-E9D7F8AD053D/Documents
console.log(path.documents());
// Prints 设置追加路径参数的结果:
// file:///var/mobile/Containers/Data/Application/89A576FE-7BB9-4F26-A456-E9D7F8AD053D/Documents/aa.jpeg
console.log(path.documents('aa.jpeg'));
```

## temp([appendPath])

获取应用程序临时目录

* @arg `[appendPath='']` {[`String`]}
* @ret {[`String`]}

## resources([appendPath])

获取应用程序资源目录

* @arg `[appendPath='']` {[`String`]}
* @ret {[`String`]}

## fallback(path)

恢复路径为操作系统可以识别的路径,一般不需要使用该函数,除非直接调用非`Langou`提供的Native/C/C++函数

* @arg `path` {[`String`]}
* @ret {[`String`]}

Example:

```js
// Prints: /var/data/test.js
console.log(path.fallback('file:///var/data/test.js'));
```

## cwd()

获取当前工作目录

* @ret {[`String`]}

## chdir(path)

设置当前工作目录,成功后返回`true`

* @arg `path` {[`String`]}
* @ret {[`bool`]} 


## isAbsolute(path)

测试路径是否为一个绝对路径

* @arg `path` {[`String`]}
* @ret {[`bool`]}

Example:

```js
// Prints:
// true
// true
// false
console.log(path.isAbsolute('/var/kk'));
console.log(path.isAbsolute('http://quickgr.org/'));
console.log(path.isAbsolute('index.jsx'));
```

## resolve(path,[...partPaths])

格式化传入的路径为标准绝对路径

* @arg `path` {[`String`]} 传入路径
* @arg `[...partPaths]` {[`String`]} 可选的分部路径
* @ret {[`String`]}

Example:

```js
// Prints: http://quickgr.org/A/C/test.js
console.log(path.resolve('http://quickgr.org/home', "..", "A", "B", "..", "C", "test.js"));
// Prints: 
// true
// file:///var/data/aaa/cc/ddd/kk.jpg
console.log(path.chdir('/var/data'));
console.log(path.resolve('aaa/bbb/../cc/.///ddd/kk.jpg'));
```

## `Class: URL`

url与path处理类

### URL.constructor([path])

构造函数，如果传入非法路径会抛出异常

* @arg `[path='']` {[`String`]} 字符串路径,传入相对路径或决对路径

Example:

```js
// cwd: file:///var/data
// Prints: file:///var/data/index.js
var uri = new URL('index.js');
console.log(uri.href);
// Prints: http://quickgr.org/index.html?args=0
var uri2 = new URL('http://quickgr.org/home/../index.html?args=0')
console.log(uri2.href);
// Prints: 
// Error: Parse uri error, Illegal URL
new URL('http://quickgr.org:').href
```

### Get: URL.href

获取uri完整的href,包括参数

* @ret {[`String`]}

Example:

```js
// Prints: http://quickgr.org/
console.log(new URL('http://quickgr.org/').href);
```

### Get: URL.filename

获取文件名称

* @ret {[`String`]}

```js
// Prints: /aaa/bbbb/ccc/test.js
console.log(new URL('http://quickgr.org/aaa/bbbb/ccc/test.js').filename);
```

### Get: URL.path

获取路径包含参数部分

* @ret {[`String`]}

```js
// Prints: /aaa/bbbb/ccc/test.js?asas=asas
console.log(new URL('http://quickgr.org/aaa/bbbb/ccc/test.js?asas=asas').path);
```

### Get: URL.dirname

获取目录名称

Example:

```js
// Prints: /aaa/bbbb/ccc
console.log(new URL('http://quickgr.org/aaa/bbbb/ccc/test.js').dirname);
```

### Get: URL.search

获取uri查询参数

* @ret {[`String`]}

Example:

```js
// Prints: ?a=A&b=B
console.log(new URL('http://quickgr.org/?a=A&b=B').search);
```

### Get: URL.hash

获取hash参数

* @ret {[`String`]}

Example:

```js
// Prints: #c=C&d=D
console.log(new URL('http://quickgr.org/?a=A&b=B#c=C&d=D').hash);
```

### Get: URL.host

获取主机,返回一个带端口号的主机名称

* @ret {[`String`]}

Example:

```js
// Prints: quickgr.org:80
console.log(new URL('http://quickgr.org:81/').host);
```

### Get: URL.hostname

获取主机名称,不会返回端口号

* @ret {[`String`]}

Example:

```js
// Prints: quickgr.org
console.log(new URL('http://quickgr.org:81/').host);
```

### Get: URL.origin

获取uri起源,protocol+host

* @ret {[`String`]}

Example:

```js
// Prints: http://quickgr.org:81
console.log(new URL('http://quickgr.org:81/host/index.html').host);
// Prints: file://
console.log(new URL('file:///var/data/index.html').host);
```

### Get: URL.basename

获取基础文件名称

* @ret {[`String`]}

Example:

```js
// Prints: index.html
console.log(new URL('file:///var/data/index.html').basename);
```

### Get: URL.extname

获取文件扩展名称

* @ret {[`String`]}

Example:

```js
// Prints: .html
console.log(new URL('file:///var/data/index.html').extname);
```

### Get: URL.port

获取主机端口号,如果URL中没有定义端口号返回一个空字符串

Example:

* @ret {[`String`]}

```js
// Prints: 81
console.log(new URL('http://quickgr.org:81').port);
// Prints 没有端口号会返回空字符串: ""
console.log(new URL('http://quickgr.org').port);
```

### Get: URL.protocol

获取URL的协议类型字符串, 例如: `'http:'`|`'https'`|`'ftp:'`

### Get: URL.params

以对像方式返回查询参数集合

* @ret {[`String`]}

Example:

```js
// Prints:
// {
//   a: "100",
//   b: "test"
// }
console.log(new URL('http://quickgr.org/?a=100&b=test').params);
```

### Get: URL.hashParams

以对像方式返回Hash参数集合

* @ret {[`Object`]}

Example:

```js
// Prints:
// {
//   a: "200",
//   b: "300"
// }
console.log(new URL('http://quickgr.org/#a=200&b=300').hashParams);
```

### URL.getParam(name)

通过名称获取uri参数值

* @arg `name` {[`String`]} 
* @ret {[`String`]} 

Example:

```js
// Prints: ok
console.log(new URL('http://quickgr.org/?args=ok').getParam('args'));
```

### URL.setParam(name, value)

设置URL查询参数键值对,并返回自己

* @arg `name` {[`String`]}
* @arg `value` {[`String`]}
* @ret {URL} 返回自己

### URL.deleteParam(name)

通过名称删除URL查询参数

* @arg `name` {[`String`]}
* @ret {URL}

### URL.clearParam()

删除URL中的所有查询参数

* @ret {URL}

### URL.getHash(name)

### URL.setHash(name, value)

### URL.deleteHash(name)

### URL.clearHash()

### URL.relative(target)

返回与`target`的相对路径

* @arg `target` {[`String`]} 字符串类型的目标路径
* @ret {[`String`]} 

Example:

```js
// Prints: ../A/B/C/test.js
var uri = new URL('http://quickgr.org/home/');
console.log(uri.relative('http://quickgr.org/A/B/C/test.js'));
// Prints: file:///var/data/A/B/C/test.js
var uri2 = new URL('http://quickgr.org/home/');
console.log(uri2.relative('file:///var/data/A/B/C/test.js'));

```

## `下面为URL快捷函数,其中第一个参数都为创建URL对像用到的路径`

## filename(path)
## path(path)
## dirname(path)
## search(path)
## hash(path)
## host(path)
## hostname(path)
## origin(path)
## basename(path)
## extname(path)
## port(path)
## protocol(path)
## params(path)
## hashParams(path)
## getParam(path, name)
## setParam(path, name, value)
## delParam(path, name)
## clearParam(path)
## getHash(path, name)
## setHash(path, name, value)
## deleteHash(path, name)
## clearHash(path)
## relative(path, target)


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

[`Encodings`]: buffer.md#buffers-and-character-encodings
[`Buffer`]: buffer.md#class-buffer
[`FileType`]: fileStat.md#enum-filetype
[`Dirent`]: fileStat.md#class-dirent
[`FileOpenMode`]: #enum-fileopenmode
[`FileStat`]: fileStat.md#class-filestat
