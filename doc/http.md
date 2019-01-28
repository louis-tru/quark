# `qgr/http`

提供`http`与`https`协议的客服端支持

可以使用`get`方式访问服务端数据，并可很方便的将这些数据保存到本地

也可使用`post`方式发送数据或上传文件到服务器

## `Enum: HttpMethod`
发送http的方法

### HTTP_METHOD_GET
### HTTP_METHOD_POST
### HTTP_METHOD_HEAD
### HTTP_METHOD_DELETE
### HTTP_METHOD_PUT

## `Enum: HttpReadyState`
`HttpClientRequest`请求状态，状态的变化触发`onreadystateChange`

### HTTP_READY_STATE_INITIAL
未初始化，未调用`send()`或一个请求完成后

### HTTP_READY_STATE_READY
准备发送状态，调用完`send()`后的正在建立连接或打开本地缓存

### HTTP_READY_STATE_SENDING

连接已经建立开始发送请求与数据，如果使用本地缓存，这个状态不会被触发

触发这个状态后会连续触发`onwrite`事件直到数据发送完成

### HTTP_READY_STATE_RESPONSE

请求发送完成，开始接收响应数据

触发这个状态后会连续触发`ondata`事件直到响应完成

### HTTP_READY_STATE_COMPLETED

完成请求，如果是一个没有异常的请求，会触发`onend`事件



## `Class: HttpClientRequest`

Http客服端请求类

### HttpClientRequest.onError

请求出现异常时触发,触发异常一般会接着触发`onabort`.

返回的事件数据为[`Error`]类型

### HttpClientRequest.onWrite

发送数据时连续触发,直到数据发送完成

### HttpClientRequest.onHeader

接收响应头完成触发

### HttpClientRequest.onData

接收响应数据时连续触发,直到数据接收完成,事件数据为主体响应片段[`Buffer`]

### HttpClientRequest.onEnd

请求完成触发

### HttpClientRequest.onReadystateChange

状态变化时触发

### HttpClientRequest.onTimeout

超时时触发，[`HttpClientRequest.setTimeout()`] 设置超时时间

### HttpClientRequest.onAbort

请求被中止时触发

Example:

```js
var cl = new HttpClientRequest()
// Prints: Error: Network error
cl.onError = function(ev) { console.log(ev.data) }
// Prints: <Buffer 00 aa cf 67>
cl.onData = function(ev) { console.log(ev.data) }
// Prints: 4 200
cl.onReadystateChange = function() { console.log(this.readyState, this.statusCode) }
```

### HttpClientRequest.constructor()

### HttpClientRequest.setMethod(method)

设置请求方法默认为[`HTTP_METHOD_GET`]方法

* @arg `method` {[`HttpMethod`]}

### HttpClientRequest.setUrl(url)
* @arg `url` {[`String`]}

### HttpClientRequest.setSavePath(path)

设置一个路径将数据保存到本地

* @arg `path` {[`String`]}

### HttpClientRequest.setUsername(username)

http请求头`Authorization`用户名

* @arg `username` {[`String`]}

### HttpClientRequest.setPassword(password)

http请求头`Authorization`密码

* @arg `password` {[`String`]}

### HttpClientRequest.disableCache(disable)

禁用缓存,既不保存下载的数据也不读取本地缓存数据

* @arg `disable` {[`bool`]}

### HttpClientRequest.disableCookie(disable)

完全禁用cookie,不保存也不读取

* @arg `disable` {[`bool`]}

### HttpClientRequest.disableSendCookie(disable)

只是不发送本地保存的cookie,但保存服务器设置的cookie

* @arg `disable` {[`bool`]}

### HttpClientRequest.disableSslVerify(disable)

禁用ssl认证,如果站点没有合法的证书,默认请求会发送失败,但可以设置禁用忽略认证

* @arg `disable` {[`bool`]}

### HttpClientRequest.setRequestHeader(headerName, value)
* @arg `headerName` {[`String`]} ascii string
* @arg `value` {[`String`]}

### HttpClientRequest.setForm(formName, value)

设置表单数据

待发送请求时会自动添加请求头 `Content-Type: application/x-www-form-urlencoded; charset=utf-8`

必须使用[`HTTP_METHOD_POST`]方法发送请求否则会忽略表单

* @arg `formName` {[`String`]}
* @arg `value` {[`String`]}

### HttpClientRequest.setUploadFile(formName, localPath)

上传本地文件设置以`multipart/form-data`形式, 并在发送请求时自动添加请求头

`Content-Type: multipart/form-data; boundary=----QgrFormBoundaryrGKCBY7qhFd3TrwA`

必须使用[`HTTP_METHOD_POST`]方法发送请求否则会忽略表单

* @arg `formName` {[`String`]}
* @arg `localPath` {[`String`]}

### HttpClientRequest.clearRequestHeader()

清空原先设置的请示头

### HttpClientRequest.clearFormData()

清空原先设置的表单

### HttpClientRequest.getResponseHeader(headerName)
* @arg `headerName` {[`String`]}
* @ret {[`String`]}

### HttpClientRequest.getAllResponseHeaders()
* @ret {[`Object`]}

### HttpClientRequest.setKeepAlive(keepAlive)

设置为`true`保持这个http连接,如果下次有这个服务器的请求会直接使用需无需再重新建立socket连接

当然这个功能需要服务器支持,如果服务完成请求后立即断开连接,那就是无效的。

**默认为`true`**

* @arg `keepAlive` {[`bool`]}

### HttpClientRequest.setTimeout(time)

设置一个超时时间,达到超时时间还未完成请求客户端会立即主动中止这个请求,并触发`onTimeout`与`onAbort`事件

默认为`0`表示永远不超时,单位为毫秒`ms`

* @arg `time` {[`uint`]} ms

### HttpClientRequest.send([data])

发送请求与数据,如果要发送数据这个请求必须为[`HTTP_METHOD_POST`]方式,否则会忽略这些数据

如果在这里设置数据，那么之前设置的表单数据将被完全忽略

* @arg `[data]` {[`String`]|[`ArrayBuffer`]|[`Buffer`]}

### HttpClientRequest.pause()

暂停请求

### HttpClientRequest.resume()

恢复暂停的请求

### HttpClientRequest.abort()

中止请求

### Get: HttpClientRequest.uploadTotal
* {[`uint`]}

### Get: HttpClientRequest.uploadSize
* {[`uint`]}

### Get: HttpClientRequest.downloadTotal
* {[`uint`]}

### Get: HttpClientRequest.downloadSize
* {[`uint`]}

### Get: HttpClientRequest.readyState
* {[`HttpReadyState`]}

### Get: HttpClientRequest.statusCode
* {[`int`]}

### Get: HttpClientRequest.url
* {[`String`]}

Example:

```js
var cl = new http.HttpClientRequest()
cl.setUrl('https://www.baidu.com/')
cl2.setSavePath(path.documents('baidu.html'));
// Prints: 
// <Buffer 3c 68 74 6d 6c 3e 0d ... >
// <Buffer 3c 21 44 4f 43 54 59 ... > 
// ...
cl.onData = function(ev) {
	console.log(ev.data);
}
cl.onEnd = function() {
	// Prints:
	// true
	// 4 200
	console.log(fs.existsSync(path.documents('baidu.html')))
	console.log(this.readyState, this.statusCode)
}
cl.send();

var cl2 = new http.HttpClientRequest()
cl2.setUrl('http://192.168.1.100:1026/Tools/uploadFile')
cl2.setMethod(http.HTTP_METHOD_POST);
cl2.setUploadFile('uploadFile', path.resources('util/http.js'))
cl2.onEnd = function() {
	// Prints: complete
	console.log('complete')
}
cl2.send();
```


## `Object: RequestOptions`

调用`request()`or`requestSync()`时使用的选项数据, 这是个`Object`类型描述并没有实际存在的构造函数

### RequestOptions.url
* {[`String`]}

### RequestOptions.method
* {[`HttpMethod`]}

### RequestOptions.headers
* {[`Object`]}

### RequestOptions.postData
* {[`Buffer`]|[`ArrayBuffer`]|[`String`]}

### RequestOptions.save
* {[`String`]}

### RequestOptions.upload
* {[`String`]}

### RequestOptions.disableSslVerify
* {[`bool`]}

### RequestOptions.disableCache
* {[`bool`]}

### RequestOptions.disableCookie
* {[`bool`]}

Example:

```js
// uploat file and save body data
var opts = {
	url: 'http://192.168.1.100:1026/Tools/uploadFile',
	method: http.HTTP_METHOD_POST,
	headers: { test: 'test' },
	// postData: 'a=A',
	save: path.documents('uploadFile.html'),
	upload: path.resources('util/http.js'),
	disableSslVerify: false,
	disableCache: true,
	disableCookie: false,
};
http.request(opts, function(buff){ 
	// Prints: <Buffer ...>
	console.log(buff)
}.catch(e=>{ /*Fail*/ }))
```

## request(options[,cb])

发送http请通过[`RequestOptions`]参数，并返回中止`id`失败抛出异常

成功通过回调返回[`Buffer`]

Callback: `cb(buff)` (buff:[`Buffer`])

* @arg `options` {[`RequestOptions`]}
* @arg `[cb]` {[`Function`]}
* @ret {[`uint`]} return req id

Example:

```js
var abortid = http.request({
	url: 'http://192.168.1.100:1026/',
}, function(buff){ /*Success*/ }.catch(e=>{ /*Fail*/ }))

```

## requestStream(options[,cb])

发送http请通过[`RequestOptions`]参数，并返回中止`id`失败抛出异常

成功通过回调返回[`StreamData`]

Callback: `cb(data)` (data:[`StreamData`])

* @arg `options` {[`RequestOptions`]}
* @arg `[cb]` {[`Function`]}
* @ret {[`uint`]} return req id

Example:

```js
var abortid = http.requestStream({ 
	url: 'http://192.168.1.100:1026/' 
}, function(d){ 
	// Prints: <Buffer ...>
	console.log(d.data) 
}.catch(e=>{ /*Fail*/ }));
```

## requestSync(options)

同步发送http请求通过[`RequestOptions`]参数，失败抛出异常

成功返回主体[`Buffer`]数据

* @arg `options` {[`Object`]}
* @ret {[`Buffer`]}

Example:

```js
// Prints: <Buffer ...>
try {
	console.log(http.requestSync({ url: 'http://192.168.1.100:1026/' }));
} catch(e) { /*Fail*/ }
```

## download(url,save[,cb])

下载并保存文件，返回中止`id`失败抛出异常

Callback: `cb()`

* @arg `url` {[`String`]}			请求的`url`
* @arg `save` {[`String`]}		本地保存路径
* @arg `[cb]` {[`Function`]}
* @ret {[`uint`]} return req id

## downloadSync(url,save)

同步下载并保存文件，失败抛出异常

* @arg `url` {[`String`]}			请求的`url`
* @arg `save` {[`String`]}		本地保存路径

## upload(url,localPath[,cb])

上传本地文件到服务器，返回中止`id`失败抛出异常

Callback: `cb()`

* @arg `url` {[`String`]}		请求有`url`
* @arg `localPath` {[`String`]}	要上传的本地文件路径
* @arg `[cb]` {[`Function`]}
* @ret {[`uint`]} return req id

## uploadSync(url,local_path)

同步上传文件，失败抛出异常

* @arg url {String}
* @arg local_path {String}

## get(url[,cb])

发送[`HTTP_METHOD_GET`]请求，返回中止`id`失败抛出异常

成功通过回调返回响应数据

Callback `cb(buff)` cb(buff:[`Buffer`])

* @arg `url` {[`String`]}
* @arg `[cb]` {[`Function`]}
* @ret {[`uint`]} return req id

## post(url,data[,cb])

发送[`HTTP_METHOD_POST`]请求，返回中止`id`失败抛出异常

成功通过回调返回响应数据

Callback `cb(buff)` cb(buff:[`Buffer`])

* @arg `url` {[`String`]}
* @arg `data` {[`String`]|[`ArrayBuffer`]|[`Buffer`]}
* @arg `[cb]` {[`Function`]}
* @ret {[`uint`]} return req id

## getSync(url)

同步发送[`HTTP_METHOD_GET`]请求，成功返回数据[`Buffer`]，失败抛出异常

* @arg `url` {[`String`]}
* @ret {[`Buffer`]}

## postSync(url,data)

同步发送[`HTTP_METHOD_POST`]请求，成功返回数据[`Buffer`]，失败抛出异常

* @arg `url` {[`String`]}
* @arg `data` {[`String`]|[`ArrayBuffer`]|[`Buffer`]}
* @ret {[`Buffer`]}

## abort(id)

通过传入中止`id`强制中止异步任务与之相似的方法有[`fs.abort()`] or [`reader.abort()`]

* @arg `id` {[`uint`]} abort id

Example:

```js
var id0 = http.download('http://192.168.1.100:1026/libs/util/http.js');
var id1 = http.upload('http://192.168.1.100:1026/Tools/uploadFile', path.resources('util/http.js'));
// force abort task
http.abort(id0);
http.abort(id1);
```

## userAgent()

返回 **User Agent**

* @ret {[`String`]}

## setUserAgent(userAgent)

设置 **User Agent**

* @arg userAgent {[`String`]}

## cachePath()

返回缓存路径

* @ret {[`String`]}

## setCachePath(path)

设置缓存路径

* @arg `path` {[`String`]}

## clearCache()

清空缓存数据文件

## clearCookie()

清空cookie


[`Object`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object
[`Date`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date
[`RegExp`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/RegExp
[`Function`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Function
[`ArrayBuffer`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer
[`TypedArray`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/TypedArray
[`String`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String
[`Number`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number
[`Boolean`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Boolean
[`null`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/null
[`undefined`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/undefined
[`Error`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Error

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
[`HttpClientRequest.setTimeout()`]: http.md#httpclientrequest-settimeout-time-
[`HttpMethod`]: http.md#enum-httpmethod
[`HttpReadyState`]: http.md#enum-httpreadystate
[`HTTP_METHOD_GET`]: http.md#http_method_get
[`HTTP_METHOD_POST`]: http.md#http_method_post
[`RequestOptions`]: http.md#object-requestoptions
[`StreamData`]: reader.md#object-streamdata
[`fs.abort()`]: fs.md#abort-id-
[`reader.abort()`]: reader.md#abort-id-
