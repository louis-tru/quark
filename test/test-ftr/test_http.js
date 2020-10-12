
import { P, M, AM, VM, VP, LOG, CA } from './test'
import 'ftr/http'
import { Buffer } from 'buffer'
import 'ftr/url'
import 'fs'

LOG('\nhttp:\n');

P(http, 'HTTP_METHOD_GET');
P(http, 'HTTP_METHOD_POST');
P(http, 'HTTP_METHOD_HEAD');
P(http, 'HTTP_METHOD_DELETE');
P(http, 'HTTP_METHOD_PUT');
P(http, 'HTTP_READY_STATE_INITIAL');
P(http, 'HTTP_READY_STATE_READY');
P(http, 'HTTP_READY_STATE_SENDING');
P(http, 'HTTP_READY_STATE_RESPONSE');
P(http, 'HTTP_READY_STATE_COMPLETED');

LOG('\nHttpClientRequest:\n')

var save = url.documents('baidu.html');
var cl = new http.HttpClientRequest();

cl.onError = function(ev) {
	LOG('http onerror:', ev.data.message) 
	P(this, 'readyState')
	P(this, 'statusCode') 
}
cl.onWrite = function(ev) {
	LOG('http onwrite:') 
	P(this, 'uploadTotal')
	P(this, 'uploadSize')
}
cl.onHeader = function(ev) { 
	LOG('http onheader:') 
	M(this, 'getResponseHeader', ['expires']);
	M(this, 'getAllResponseHeaders');
}
cl.onData = function(ev){ 
	//LOG('http ondata:', ev.data.toString());
	LOG('http ondata:', ev.data.length);
	P(this, 'downloadSize')
	P(this, 'downloadTotal')
}
cl.onEnd = function(ev){ 
	LOG('http onend:') 
	M(this, 'pause');
	M(this, 'resume');
	M(this, 'abort');
	P(this, 'uploadTotal')
	P(this, 'uploadSize')
	P(this, 'downloadTotal')
	P(this, 'downloadSize')
	P(this, 'readyState')
	P(this, 'statusCode')
	P(this, 'url')
}
cl.onReadystateChange = function(ev){ 
	LOG('http onReadystateChange:', this.readyState, this.statusCode) 
}
cl.onTimeout = function(ev){ 
	LOG('http ontimeout:')
};
cl.onAbort = function(ev){ 
	LOG('http onabort:') 
};

M(cl, 'setMethod', [http.HTTP_METHOD_GET]);
M(cl, 'setUrl', ['https://www.baidu.com/']);
M(cl, 'setSavePath', [save]);
M(cl, 'setUsername', ['louis']);
M(cl, 'setPassword', ['Alsk106612']);
M(cl, 'clearRequestHeader')
M(cl, 'clearFormData')
M(cl, 'setRequestHeader', ['test_set_request_header', 'test'])
M(cl, 'getResponseHeader', ['expires'])
M(cl, 'getAllResponseHeaders')
M(cl, 'setKeepAlive', [false])
M(cl, 'setTimeout', [10000])
M(cl, 'pause')
M(cl, 'resume')
M(cl, 'abort')
P(cl, 'uploadTotal')
P(cl, 'uploadSize')
P(cl, 'downloadTotal')
P(cl, 'downloadSize')
P(cl, 'readyState')
P(cl, 'statusCode')
P(cl, 'url')
M(cl.onEnd, 'on', [function(ev){ 
	M(fs, 'readFileSync', [save]);
	test_2();
}, 1]);
M(cl, 'send')


function test_2() {
	var file = url.documents('test_upload.txt');
	var file2 = url.documents('test_upload2.txt');

	LOG('\nTest Upload File:')
	LOG('Test disable_cookie:\n')

	M(fs, 'writeFileSync', [file, 'ABCDEFG'])
	M(fs, 'writeFileSync', [file2, '你好吗？升级不安全请求'])

	M(cl, 'clearRequestHeader')
	M(cl, 'clearFormData')
	M(cl, 'setUrl', ['http://192.168.1.100:1026/Tools/upload_file'])
	M(cl, 'setMethod', [http.HTTP_METHOD_POST])
	M(cl, 'setSavePath', ['']);
	M(cl, 'setKeepAlive', [true])
	//M(cl, 'disable_cookie', [true])
	M(cl, 'disableSendCookie', [true])
	M(cl, 'setForm', ['data', 'The test file upload'])
	M(cl, 'setUploadFile', ['upload_file', file])
	M(cl, 'setUploadFile', ['upload_file2', file2])
	M(cl.onEnd, 'on', [test_3, 1]);
	M(cl, 'send')
}

function test_3() {
	LOG('\nTest disable_cache:\n')

	M(cl, 'disableCookie', [false])
	M(cl, 'disableSendCookie', [false])
	M(cl, 'setMethod', [http.HTTP_METHOD_GET])
	M(cl, 'setUrl', ['http://192.168.1.100:1026/doc/index.md'])
	M(cl, 'disableCache', [true]);
	M(cl.onEnd, 'on', [test_4, 1]);
	M(cl, 'send')
}

function test_4() {
	LOG('\nTest disable_ssl_verify:\n')

	M(cl, 'setUrl', ['https://kyfw.12306.cn/otn/regist/init']) // 12306的证书一直都是失效的
	M(cl, 'disableSslVerify', [true]);
	M(cl, 'disableCache', [false]);
	M(cl.onEnd, 'on', [test_5, 1]);
	M(cl, 'send')
}

function test_5() {
	CA(async_test_helper)
}

async function async_test_helper() {
	LOG('\nTest HttpHelper:\n')

	M(http, 'abort', [http.get('https://www.baidu.com/',()=>{ LOG('** abort read_stream url fail **') })]);
	await AM(http, 'request', [{ 
		url: 'http://192.168.1.100:1026/Tools/upload_file',
		method: http.HTTP_METHOD_POST,
		headers: { 'test': 'test' },
		//postData: 'a=AA',
		save: url.documents('test_request_save.html'),
		upload: url.resources('ftr/base.js'),
	}, d=>1])
	VM(fs, 'existsSync', [url.documents('test_request_save.html')], true);
	VM(http, 'getSync', ['http://192.168.1.100:1026/out/temp/util.js'], d=>d instanceof Buffer);
	await AM(http, 'request', [{ 
		url: 'http://192.168.1.100:1026/Tools/upload_file',
		method: http.HTTP_METHOD_POST,
		postData: 'a=AA',
	}, d=>1])
	await AM(http, 'request', [{ 
		url: 'https://www.baidu.com/',
		method: http.HTTP_METHOD_GET,
		disableSslVerify: true,
		disableCache: true,
		disableCookie: true,
	}, d=>1])
	await AM(http, 'requestStream', [{ url: 'https://www.baidu.com/' }, d=>d.complete]);
	
	M(http, 'requestSync', [{ url: 'https://www.baidu.com/' }]);
	await AM(http, 'download', ['https://www.baidu.com/', url.documents('down.html'), d=>1]);
	VM(fs, 'existsSync', [url.documents('down.html')], true);
	//M(http, 'downloadSync', ['https://www.baidu.com/', url.documents('down2.html')]);
	//VM(fs, 'existsSync', [url.documents('down2.html')], true);
	await AM(http, 'upload', [
		'http://192.168.1.100:1026/Tools/upload_file', url.resources('ftr/http.js'), d=>1]);
	VM(http, 'requestSync', 
		[{url:'http://192.168.1.100:1026/out/temp/http.js',disable_cache:1}], d=>d instanceof Buffer);
	//M(http, 'uploadSync', [
	//	'http://192.168.1.100:1026/Tools/upload_file', url.resources('ftr/url.js')]);
	//VM(http, 'requestSync', 
	//	[{url:'http://192.168.1.100:1026/out/temp/path.js',disable_cache:1}], d=>d instanceof Buffer);
	await AM(http, 'get', ['http://192.168.1.100:1026/out/temp/http.js', d=>1]);
	//await AM(http, 'get_stream', ['https://www.baidu.com/', d=>d.complete]);
	await AM(http, 'post', ['http://192.168.1.100:1026/Tools/upload_file', 'b=B', d=>1]);
	await AM(http, 'post', ['http://192.168.1.100:1026/Tools/upload_file', new Buffer('c=C'), d=>1]);
	M(http, 'getSync', ['http://192.168.1.100:1026/']);
	M(http, 'postSync', ['http://192.168.1.100:1026/Tools/upload_file', 'e=E']);
	M(http, 'postSync', ['http://192.168.1.100:1026/Tools/upload_file', new Buffer('f=F')]);
	M(http, 'userAgent');
	M(http, 'setUserAgent', [http.userAgent() + ',AAAA']);
	M(http, 'userAgent');
	M(http, 'postSync', ['http://192.168.1.100:1026/Tools/upload_file', 'h=H']);
	M(http, 'cachePath')
	M(http, 'clearCache')
	M(http, 'clearCookie')
	//M(http, 'sslCacertFile')
	//M(http, 'setSslCacertFile', [http.sslCacertFile()]);
	//M(http, 'setSslClientKeyfile', ['xxxxx.pem'])
	//M(http, 'setSslClientKeypasswd', ['abcdefg'])
}
