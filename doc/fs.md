# `ngui/fs`

这里提供的是nodejs文件的扩展函数

## DEFAULT_MODE

创建与设置文件的默认`mode`值,这与文件的权限相关,这是一个`int`整数类型值

## `Enum: FileType`

文件的类型,枚举类型都为`int`整数

### FILE_UNKNOWN
### FILE_FILE
### FILE_DIR
### FILE_LINK
### FILE_FIFO
### FILE_SOCKET
### FILE_CHAR
### FILE_BLOCK

## `Object: Dirent`

读取目录时返回的结构类型, 这是个`Object`类型描述并没有实际存在的构造函数

### Dirent.name
* {[`String`]} 文件名称

### Dirent.pathname
* {[`String`]} 文件路径

### Dirent.type
* {[`FileType`]} 文件类型枚举值 `FileType`

## abort(id)

通过`id`强制中止运行中的异步任务

如果传入无意义的`id`或`id`所属的任务已经完成，不做任何处理

* @arg `id` {[`uint`]}

Example:

```js
var id0 = fs.chmodR(mypath, 755);
var id1 = fs.chownR(mypath, 501, 501);
var id2 = fs.cp(mypath, newpath);
// force abort task
fs.abort(id0);
fs.abort(id1);
fs.abort(id2);
```

## chmodR(path[,mode[,cb]])
## chmodR(path[,cb])

递归设置文件或目录`mode`属性,并返回中止`id`,可通过这个`id`强制中止任务

Callback: `cb()`

* @arg `path` {[`String`]}
* @arg `[mode=DEFAULT_MODE]` {[`uint`]}
* @arg `[cb]` {[`Function`]}
* @ret {[`uint`]} return abort `id`

Example:

```js
// `mypath`为文件路径,可以为文件也可以为目录
fs.chmodR(mypath, 0755, function(err){
	if (err)
		console.log('Fail');
	else
		console.log('Success');
});
var id = fs.chmodR(mydir, 0775);
fs.abort(id);
```

## chmodSyncR(path[,mode])

同步设置文件的mode属性,设置成功返回`true`

递归`chmodSyncR()`

* @arg `path` {[`String`]}
* @arg `[mode=DEFAULT_MODE]` {[`uint`]} **[`DEFAULT_MODE`]**
* @ret {[`bool`]}

Example:

```js
// Prints: true
console.log(fs.chmodSyncR(mypath, 0755));
```

## chownR(path, owner, group[,cb])

异步递归设置文件或目录`owner`与`group`属性。并返回中止`id`,可通过这个`id`强制中止任务

Callback:`cb()`

* @arg `path` {[`String`]}
* @arg `owner` {[`uint`]}
* @arg `group` {[`uint`]}
* @arg `[cb]` {[`Function`]}
* @ret {[`uint`]} return abort `id`

Example:

```js
var id = chownR(mypath, 501, 501, (err)=>{ });
fs.abort(id); // force abort task 
```

## chownSyncR(path, owner, group)

同步设置文件owner与group属性。成功返回`true`

递归`chownSyncR()`

* @arg `path` {[`String`]}
* @arg `owner` {[`uint`]} 操作系统用户id	
* @arg `group` {[`uint`]} 操作系统组id
* @ret {[`bool`]}

## mkdirP(path[,mode[,cb]])
## mkdirP(path[,cb])

创建目录，这个方法会依次创建目录树,目录存在也不会抛出异常

Callback:`cb()`

* @arg `path` {[`String`]}
* @arg `[mode=DEFAULT_MODE]` {[`uint`]} **[`DEFAULT_MODE`]**
* @arg `[cb]` {[`Function`]}

Example:
```js
fs.mkdirP(mypath, function(err){ 
	if (err) {
		/*Success*/ 
	} else {
		/*Fail*/ 
	}
});
```

## mkdirSyncP(path[,mode])

* @arg `path` {[`String`]}
* @arg `[mode=DEFAULT_MODE]` {[`uint`]} **[`DEFAULT_MODE`]**
* @ret {[`bool`]} Success return `true`

## removeR(path[,cb])

递归删除文件与目录,并返回一个中止id,如果不成功会抛出异常

使用这个id可以强制中止任务 `abort(id)`

Callback: `cb()`

* @arg `path` {[`String`]}
* @arg `[cb]` {[`Function`]}
* @ret {[`uint`]} return `id`

Example:
```js
var id = fs.removeR(mypath, function(err) { 
	if (err) {
		/*Success*/ 
	} else {
		/*Fail*/ 
	}
});
// 通过id可中止删除任务
fs.abort(id);
```

## removeSyncR(path)

同步递归删除目录或文件,在javascript中谨慎使用这个方法,有可能会造成线程长时间被柱塞

* @arg `path` {[`String`]}
* @ret {[`bool`]} Success return `true`

## copy(path,target[,cb])
## copyR(path, target[,cb])

拷贝文件,并返回一个中止id,如果不成功会抛出异常

使用这个id可以强制中止拷贝任务 `abort(id)`

递归拷贝文件与目录`copyR()` 这个方法与`copy()`区别在于，`copy()`只能拷贝单个文件

Callback: `cb()`

* @arg `path` {[`String`]}
* @arg `target` {[`String`]}
* @arg `[cb]` {[`Function`]}
* @ret {[`uint`]} return `id`

Example:
```js
var id = fs.copy(source, target, function(err) { 
	if (err) {
		/*Success*/ 
	} else {
		/*Fail*/ 
	}
});
// 通过id可中止任务
fs.abort(id);
```

## copySync(path, target)
## copySyncR(path, target)

同步拷贝文件,在javascript中谨慎使用这个方法,有可能会造成线程长时间被柱塞

递归拷贝文件与目录`copySyncR()` 

* @arg `path` {[`String`]}
* @arg `target` {[`String`]}
* @ret {[`bool`]} Success return `true`

## readdir(path[,cb])

读取目录列表信息，失败抛出异常,成功返回[`Dirent`]的[`Array`]

Callback: `cb(dirents)`

* @arg `path` {[`String`]}
* @arg `[cb]` {[`Function`]}

Example:

```js
// Prints:
// {
//   name: "cp.txt",
//   pathname: "file:///var/mobile/Containers/Data/Application/64DAC3FC-A4FD-4274-A2E7-B834EE4930B4/Documents/test/cp.txt",
//   type: 1
// }
fs.readdir(mydir, function(err, dirents) {
	if (err) {
		/*Fail*/
	} else {
		for (var dirent of dirents) {
			// TODO...
			console.log(dirent);
		}
	}
	console.log(dirent);
});
```

## readdirSync(path)

* @arg `path` {[`String`]}
* @ret {[`Array`]} return Array<Dirent>	

## isFile(path[,cb])

测试是否为一个文件，成功返回[`bool`]类型值

Callback: `cb(ok)` (ok:[`bool`])

* @arg path {[`String`]}
* @arg [cb] {[`Function`]}

## isFileSync(path)
* @arg `path` {[`String`]}
* @ret {[`bool`]}

## isDirectory(path[,cb])

测试是否为一个目录，成功返回[`bool`]类型值

Callback: `cb(ok)` (ok:[`bool`])

* @arg `path` {[`String`]}
* @arg `[cb]` {[`Function`]}

## isDirectorySync(path)
* @arg `path` {[`String`]}
* @ret {[`bool`]}


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

[`int`]: nativeTypes.md#int
[`uint`]: nativeTypes.md#uint
[`int16`]: nativeTypes.md#int16
[`uint16`]: nativeTypes.md#uint16
[`int64`]: nativeTypes.md#int64
[`uint64`]: nativeTypes.md#uint64
[`float`]: nativeTypes.md#float
[`double`]: nativeTypes.md#double
[`bool`]: nativeTypes.md#bool

[`FileType`]: fs.md#enum-filetype
[`Dirent`]: fs.md#class-dirent
[`DEFAULT_MODE`]: fs.md#default_mode
