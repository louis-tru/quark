
import { LOG, Pv, Mv, Ca } from './tool'
import {HttpClientRequest,HttpMethod, HttpReadyState} from 'quark/http'
import * as http from 'quark/http'
import path from 'quark/path'
import * as fs from 'quark/fs'
import * as buffer from 'quark/buffer'
import util from 'quark/util'

// setInterval(()=>util.gc());
const tools_test_url = 'http://192.168.2.169:1026';

export default async function(_: any) {
	LOG('\nHttpClientRequest:\n')

	let save = path.documents('baidu.html');
	let cl = new HttpClientRequest();

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
		LOG('http ondata:', ev.data.length, cl.url, buffer.toString(ev.data));
		// LOG('http ondata:', ev.data.length, cl.url);
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
		LOG('fs.readFileSync', fs.readFileSync(save, 'utf8'));
		test_download(cl);
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

function test_download(cl: HttpClientRequest) {
	LOG('\nTest test_download:\n')
	Mv(cl, 'setSavePath', ['']) // no save path
	Mv(cl, 'disableCookie', [false])
	Mv(cl, 'disableSendCookie', [false])
	Mv(cl, 'setMethod', [HttpMethod.GET])
	Mv(cl, 'setUrl', ['https://github.com/louis-tru/quark/blob/master/doc/index.md'])
	Mv(cl, 'disableCache', [true]);
	Mv(cl.onEnd, 'on', [()=>test_upload(cl), '1']);
	Mv(cl, 'send', [])
}

function test_upload(cl: HttpClientRequest) {
	var file = path.documents('test_upload.txt');
	var file2 = path.documents('test_upload2.txt');

	LOG('\nTest Upload File:')
	LOG('Test disable_cookie:\n')

	Mv(fs, 'writeFileSync', [file, 'ABCDEFG'])
	Mv(fs, 'writeFileSync', [file2, '你好吗？升级不安全请求'])

	Mv(cl, 'clearRequestHeader', [])
	Mv(cl, 'clearFormData', [])
	Mv(cl, 'setUrl', [`${tools_test_url}/Tools/upload_file`])
	Mv(cl, 'setMethod', [HttpMethod.POST])
	Mv(cl, 'setSavePath', ['']); // no save path
	Mv(cl, 'setKeepAlive', [true])
	Mv(cl, 'disableCookie', [true])
	Mv(cl, 'disableSendCookie', [true])
	Mv(cl, 'setForm', ['data', 'The test file upload'])
	Mv(cl, 'setUploadFile', ['upload_file', file])
	Mv(cl, 'setUploadFile', ['upload_file2', file2])
	Mv(cl.onEnd, 'on', [()=>{ /*test_5()*/ }, '1']);
	Mv(cl, 'send', [])
}

function test_4(cl: HttpClientRequest) {
	LOG('\nTest disable_ssl_verify:\n')
	Mv(cl, 'setUrl', ['https://kyfw.12306.cn/otn/regist/init']) // 12306的证书一直都是失效的
	Mv(cl, 'disableSslVerify', [true]);
	Mv(cl, 'disableCache', [false]);
	Mv(cl.onEnd, 'on', [test_5, '1']);
	Mv(cl, 'send', [])
}

function test_5() {
	Ca(async_test_helper)
}

async function async_test_helper() {
	LOG('\nTest HttpHelper:\n')

	Mv(http, 'abort', [http.get('https://www.baidu.com/').id]);

	await Mv(http, 'request', [{
		url: `${tools_test_url}/Tools/upload_file`,
		method: HttpMethod.POST,
		headers: { 'test': 'test' },
		postData: 'a=AA',
		save: path.documents('test_request_save.html'),
		upload: path.resources('quark/base.js'),
	}])
	Mv(fs, 'existsSync', [path.documents('test_request_save.html')], true);
	Mv(http, 'getSync', ['http://192.168.1.100:1026/out/temp/util.js'], d=>!!d.length);

	await Mv(http, 'request', [{ 
		url: `${tools_test_url}/Tools/upload_file`,
		method: HttpMethod.POST,
		postData: 'a=AA',
	}])

	await Mv(http, 'request', [{ 
		url: 'https://www.baidu.com/',
		method: HttpMethod.GET,
		disableSslVerify: true,
		disableCache: true,
		disableCookie: true,
	}])
	await Mv(http, 'requestStream', [{ url: 'https://www.baidu.com/' }, d=>d.complete]);

	Mv(http, 'requestSync', [{ url: 'https://www.baidu.com/' }]);
	await Mv(http, 'download', ['https://www.baidu.com/', path.documents('down.html')]);
	Mv(fs, 'existsSync', [path.documents('down.html')], true);
	//M(http, 'downloadSync', ['https://www.baidu.com/', url.documents('down2.html')]);
	//VM(fs, 'existsSync', [url.documents('down2.html')], true);

	await Mv(http, 'upload', [
		`${tools_test_url}/Tools/upload_file`, path.resources('testing/test_http.js')]);
	Mv(http, 'requestSync',
		[{url:`${tools_test_url}/out/temp/test_http.js`,disableCache:true}], d=>d instanceof Uint8Array);

	Mv(http, 'uploadSync', [
		`${tools_test_url}/Tools/upload_file`, path.resources('testing/test_path.js')]);
	Mv(http, 'requestSync', 
		[{url:'http://192.168.1.100:1026/out/temp/test_path.js',disableCache:true}], d=>d instanceof Uint8Array);

	await Mv(http, 'get', ['http://192.168.1.100:1026/out/temp/http.js']);
	//await Mv(http, 'get_stream', ['https://www.baidu.com/', d=>d.complete]);
	await Mv(http, 'post', [`${tools_test_url}/Tools/upload_file`, 'b=B']);
	await Mv(http, 'post', [`${tools_test_url}/Tools/upload_file`, buffer.fromString('c=C')]);
	Mv(http, 'getSync', ['http://192.168.1.100:1026/']);
	Mv(http, 'postSync', [`${tools_test_url}/Tools/upload_file`, 'e=E']);
	Mv(http, 'postSync', [`${tools_test_url}/Tools/upload_file`, buffer.fromString('f=F')]);
	Mv(http, 'userAgent', []);
	Mv(http, 'setUserAgent', [http.userAgent() + ',AAAA']);
	Mv(http, 'userAgent', []);
	Mv(http, 'postSync', [`${tools_test_url}/Tools/upload_file`, 'h=H']);
	Mv(http, 'cachePath', [])
	Mv(http, 'clearCache', [])
	Mv(http, 'clearCookie', [])
	//M(http, 'sslCacertFile')
	//M(http, 'setSslCacertFile', [http.sslCacertFile()]);
	//M(http, 'setSslClientKeyfile', ['xxxxx.pem'])
	//M(http, 'setSslClientKeypasswd', ['abcdefg'])
}