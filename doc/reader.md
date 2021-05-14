# `flare/reader`

这里提供的方法可以针对不协议的uri路径进行基本的读取操作

现在支持的路径类型：

* `http://` or `https://` - 可使用同步或异步方式进行读取,但不能读取目录或测试存在, 
`readdirSync()`返回空数组而`isFileSync()`永远返回`false`。

* `file://` 本地文件路径。`/var/data` or `var/data` 都可做为本地路径，并不会出错。

*	`zip://`	这是`zip`包内路径的一种表示方法，`zip:///var/data/test.zip@/a.txt` 
这个路径表示`zip:///var/data/test.zip`中的`a.txt`文件。注意这个路径一定要存在于本地文件系统中


## `Object: StreamData`

读取文件流时返回的结构类型, 这是个`Object`类型描述并没有实际存在的构造函数

### StreamData.data
* {[`Buffer`]} 当前读取到的Buffer数据

### StreamData.complete
* {[`bool`]} 读取是否完成

### StreamData.size
* {[`uint`]} 已经读取到的数据总量

### StreamData.total
* {[`uint`]} 全部数据源的总大小,有可能是`0`,为`0`表示数据大小未知,可能为无限大比如为视频直播数据流


## readStream(path[,cb])

异步读取文件流，并返回中止`id`

通过中止`id`可强制取消当前的读取任务

成功后通过回调返回[`StreamData`]类型数据

Callback: `cb(data)` (data:[`StreamData`])

* @arg `path` {[`String`]}    要读取的文件路径
* @arg `[cb]` {[`Function`]}
* @ret {[`uint`]} return abort `id`

## readFile(path[,cb])

异步读取文件，并返回中止`id`,通过中止`id`可强制取消当前的读取任务

成功后通过回调返回[`Buffer`]数据

Callback: `cb(data)` (data:[`Buffer`])

* @arg `path` {[`String`]}    	
* @arg `[cb]` {[`Function`]}
* @ret {[`uint`]} return abort `id`

Example:

```js
// async read file stream 
reader.readStream('http://www.baidu.com', function(d){ }));
reader.readStream('file:///var/data/test.txt', function(d){ }));
reader.readStream('zip:///var/data/test.zip@aa.txt', function(d){ 
	/*Success*/ 
	console.log(d.data.length, d.complete);
}));

// async read file
reader.read('http://www.baidu.com', function(d){ }));
reader.read('file:///var/data/test.txt', function(d){ }));
reader.read('zip:///var/data/test.zip@aa.txt', function(d){ 
	/*Success*/ 
	console.log(d.length);
}.catch(e=>{ /*Fail*/ }));
```

## readFileSync(path)

同步读取文件,成功返回文件`Buffer`失败会抛出异常

* @arg `path` {[`String`]}
* @ret {[`Buffer`]} return `Buffer`

## readdirSync(path)

读取目录列表信息，返回[`Dirent`]的[`Array`]

这个方法不能处理`http://`与`https://`类型的路径,如果传入这种路径立即返回一个空数组[`Array`]

这个方法也不会抛出异常，如果不能读取路径，只会返回空数组[`Array`]

* @arg `path` {[`String`]}
* @ret {[`Array`]}

## existsSync(path)

测试文件或目录是否存在，如果文件存在会返回`false`

这个方法不能处理`http://`与`https://`类型的路径,如果传入这种路径立即返回`false`

* @arg `path` {[`String`]}
* @ret {[`bool`]}

## isFileSync(path)

测试文件是否存在

* @arg `path` {[`String`]}
* @ret {[`bool`]}

## isDirectorySync(path)

测试目录是否存在

* @arg `path` {[`String`]}
* @ret {[`bool`]}

## abort(id)

通过`id`中止异步任务，与[`fs.abort`]功能相同

* @arg id {[`uint`]} 传入的中止`id`


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

[`Buffer`]: https://nodejs.org/dist/latest-v8.x/docs/api/buffer.html
[`fs.readStream`]: fs.md#readstream-path-cb-
[`StreamData`]: reader.md#object-streamdata
[`fs.abort`]: fs.md#abort-id-
[`Dirent`]: fs.md#object-dirent
