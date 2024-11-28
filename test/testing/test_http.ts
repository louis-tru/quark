
import { LOG, Pv, Mv, Mvp, Ca } from './tool'
import {HttpClientRequest,HttpMethod, HttpReadyState} from 'quark/http'
import * as http from 'quark/http'
import path from 'quark/path'
import * as fs from 'quark/fs'
import * as buffer from 'quark/buffer'
import util from 'quark/util'

const tools_test_url = 'http://192.168.2.169:1026'

export default async function(_: any) {
	const cl = new HttpClientRequest()
	await Ca(test_cl, cl)
	await Ca(test_download, cl)
	await Ca(test_upload, cl)
	await test_5()
}

function test_cl(cl: HttpClientRequest, cb: any) {
	LOG('\nHttpClientRequest:\n')

	let save = path.documents('baidu.html');

	cl.onError.on(function(ev) {
		LOG('http onerror:', ev.data.message) 
		Pv(cl, 'readyState', e=>e!=HttpReadyState.Completed)
		Pv(cl, 'statusCode', e=>e!=200)
	})
	cl.onWrite.on(function(ev) {
		LOG('http onwrite:')
		Pv(cl, 'uploadTotal', e=>e!=0)
		Pv(cl, 'uploadSize', e=>e!=0)
	})
	cl.onHeader.on(function(ev) {
		LOG('http onheader:') 
		Mv(cl, 'getResponseHeader', ['expires']);
		for (let [k,v] of Object.entries(Mv(cl, 'getAllResponseHeaders', []))) {
			LOG('http onheader:', k, ':', v);
		}
	})
	cl.onData.on(function(ev){
		// LOG('http ondata:', ev.data.length, cl.url, buffer.toString(ev.data));
		LOG('http ondata:', ev.data.length, cl.url);
	})
	cl.onEnd.on(function(ev){
		LOG('http onend:')
		Mv(cl, 'pause', [])
		Mv(cl, 'resume', [])
		Mv(cl, 'abort', [])
		// Pv(cl, 'uploadTotal', e=>e==0)
		// Pv(cl, 'uploadSize', e=>e==0)
		// Pv(cl, 'downloadTotal', e=>e!=0)
		Pv(cl, 'downloadSize', e=>e!=0)
		//Pv(cl, 'readyState', HttpReadyState.Completed)
		//Pv(cl, 'statusCode', 200)
	})
	cl.onReadystateChange.on(function(ev){
		LOG('http onReadystateChange:', cl.readyState, cl.statusCode)
	})
	cl.onTimeout.on(function(ev) {
		LOG('http ontimeout:')
	})
	cl.onAbort.on(function(ev) {
		LOG('http onabort:') 
	})
	Mv(cl.onEnd, 'on', [function(ev){
		LOG('fs.readFileSync', fs.readFileSync(save).length);
		cb();
	}, '1']);

	Mv(cl, 'setMethod', [HttpMethod.GET]);
	Mv(cl, 'setUrl', ['https://www.baidu.com/']);
	Mv(cl, 'setSavePath', [save]);
	Mv(cl, 'setUsername', ['louis']);
	Mv(cl, 'setPassword', ['Alsk106612']);
	Mv(cl, 'clearRequestHeader', [])
	Mv(cl, 'clearFormData', [])
	Mv(cl, 'setRequestHeader', ['test_set_request_header', 'test'])
	Mv(cl, 'getResponseHeader', ['expires'])
	Mv(cl, 'getAllResponseHeaders', [])
	// Mv(cl, 'setKeepAlive', [false])
	Mv(cl, 'setTimeout', [10000])
	Mv(cl, 'pause', [])
	Mv(cl, 'resume', [])
	Mv(cl, 'abort', [])
	Pv(cl, 'uploadTotal', 0)
	Pv(cl, 'uploadSize', 0)
	Pv(cl, 'downloadTotal', 0)
	Pv(cl, 'downloadSize', 0)
	Pv(cl, 'readyState', HttpReadyState.Initial)
	Pv(cl, 'statusCode', 0)
	Pv(cl, 'url', 'https://www.baidu.com/')
	Mv(cl, 'disableCookie', [true])
	Mv(cl, 'disableSendCookie', [true])
	Mv(cl, 'send', [])
}

function test_download(cl: HttpClientRequest, cb: any) {
	LOG('\nTest test_download:\n')
	Mv(cl, 'setSavePath', ['']) // no save path
	Mv(cl, 'disableCookie', [false])
	Mv(cl, 'disableSendCookie', [false])
	Mv(cl, 'setMethod', [HttpMethod.GET])
	Mv(cl, 'setUrl', ['https://github.com/louis-tru/quark/blob/master/doc/index.md'])
	Mv(cl, 'disableCache', [true]);
	Mv(cl.onEnd, 'on', [()=>cb(), '1']);
	Mv(cl, 'send', [])
}

function test_upload(cl: HttpClientRequest, cb: any) {
	var file = path.documents('test_upload.txt');
	var file2 = path.documents('test_upload2.txt');

	LOG('\nTest Upload File:\n')

	Mv(fs, 'writeFileSync', [file, 'ABCDEFG'])
	Mv(fs, 'writeFileSync', [file2, '你好吗？升级不安全请求'])

	Mv(cl, 'clearRequestHeader', [])
	Mv(cl, 'clearFormData', [])
	Mv(cl, 'setUrl', [`${tools_test_url}/Tools/upload_file`])
	Mv(cl, 'setMethod', [HttpMethod.POST])
	Mv(cl, 'setSavePath', ['']); // no save path
	Mv(cl, 'setKeepAlive', [true])
	Mv(cl, 'setForm', ['data', 'The test file upload'])
	Mv(cl, 'setUploadFile', ['upload_file', file])
	Mv(cl, 'setUploadFile', ['upload_file2', file2])
	Mv(cl.onEnd, 'on', [()=>{cb()}, '1']);
	Mv(cl, 'send', [])
}

async function test_5() {
	LOG('\nTest HttpHelper:\n')

	await Mv(http, 'request', [{
		url: `${tools_test_url}/Tools/upload_file`,
		method: HttpMethod.POST,
		headers: { 'test': 'test' },
		save: path.documents('test_request_save.html'),
		upload: path.resources('testing/test.txt'),
	}], e=>e.statusCode==200&&fs.existsSync(path.documents('test_request_save.html')))

	await Mv(http, 'get', [`${tools_test_url}/out/temp/test.txt`], e=>buffer.toString(e.data)=='Test');
	Mv(http, 'getSync', [`${tools_test_url}/out/temp/test.txt`], e=>buffer.toString(e)=='Test');

	await Mv(http, 'request', [{
		url: `${tools_test_url}/Tools/upload_file`, method: HttpMethod.POST, postData: 'a=AA',}], e=>e.data.length==279)
	await Mv(http, 'post', [`${tools_test_url}/Tools/upload_file`, 'b=B'], d=>d.data.length==279);
	await Mv(http, 'post', [`${tools_test_url}/Tools/upload_file`, buffer.fromString('c=C')], d=>d.data.length==279);

	Mv(http, 'postSync', [`${tools_test_url}/Tools/upload_file`, 'd=D'], d=>d.length==279);
	Mv(http, 'postSync', [`${tools_test_url}/Tools/upload_file`, buffer.fromString('e=E')], d=>d.length==279);

	await Mv(http, 'upload', [
		`${tools_test_url}/Tools/upload_file`, path.resources('testing/all_we_know.mp3')], e=>e.data.length==279)
	
	Mv(http, 'requestSync',
		[{url:`${tools_test_url}/out/temp/all_we_know.mp3`,disableCache:true}], d=>d.length==7766770)

	Mv(http, 'uploadSync', [
		`${tools_test_url}/Tools/upload_file`, path.resources('testing/iconfont.ttf')])

	Mv(http, 'requestSync',
		[{url:`${tools_test_url}/out/temp/iconfont.ttf`,disableCache:true}], d=>d.length==10616)

	// --------------------------------------------------------

	await Mv(http, 'request', [{url:'https://kyfw.12306.cn/otn/regist/init',disableSslVerify: true,disableCache: true}])

	Mv(http, 'abort', [http.get('https://www.baidu.com/').id]);
	await Mv(http, 'request', [{
		url: 'https://www.baidu.com/',
		method: HttpMethod.GET, disableSslVerify: true, disableCache: true, disableCookie: true,}], e=>e.statusCode==200)
	await Mv(http, 'requestStream', [{ url: 'https://www.baidu.com/' }, d=>LOG('requestStream:', d.data, 'complete:', d.complete)])
	await Mv(http, 'getStream', ['https://www.baidu.com/', d=>LOG('requestStream:', d.data, 'complete:', d.complete)]);
	Mv(http, 'requestSync', [{ url: 'https://www.baidu.com/' }])
	await Mv(http, 'download', ['https://www.baidu.com/', path.documents('down.html')], e=>fs.existsSync(path.documents('down.html')))
	Mv(http, 'downloadSync', ['https://www.baidu.com/', path.documents('down2.html')], e=>fs.existsSync(path.documents('down2.html')))

	Mvp(http, 'userAgent', [])
	Mv(http, 'setUserAgent', [http.userAgent() + ',Test'])
	Mvp(http, 'userAgent', [])
	Mvp(http, 'cachePath', [])
	Mv(http, 'clearCache', [])
	Mv(http, 'clearCookie', [])
}
