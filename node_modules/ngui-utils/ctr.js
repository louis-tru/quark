/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

var util = require('./util');
var path = require('path');
var fs = require('./fs');
var HttpService = require('./http_service').HttpService;
var Module = require('module').Module;
var vm = require('vm');
var template = require('./template');

var FILE_CACHE_TIMEOUT = util.dev ? 0 : 1000 * 60 * 10; // 10分钟
var FILE_CACHES = {};
var _require = require;

function makeRequireFunction(mod) {
	function require(path) {
		return mod.require(path);
	}
	function resolve(request, options) {
		if (typeof request !== 'string') {
			var actual = request;
			var str = `The "request" argument must be of type string`;
			str += `. Received type ${actual !== null ? typeof actual : 'null'}`;
			throw new Error(str);
		}
		return Module._resolveFilename(request, mod, false, options);
	}
	require.resolve = resolve;
	return require;
}

function require_ejs(filename, options, __mainfilename) {

	var ext = path.extname(filename);

	if (ext != '.ejs' && ext != '.ejsx') {
		return _require(filename);
	}

	var ejs = FILE_CACHES[filename];
	if (ejs) {
		if (ejs.timeout < Date.now())  {
			FILE_CACHES[filename] = ejs = null;
		}
	}
	if (!ejs) {
		ejs = {
			source: fs.readFileSync(filename, 'utf8'),
			timeout: Date.now() + FILE_CACHE_TIMEOUT,
		};
		if (FILE_CACHE_TIMEOUT) {
			FILE_CACHES[filename] = ejs;
		}
	}

	var dirname = path.dirname(filename);
	var mod = { 
		exports: {}, 
		id: filename, 
		dirname: dirname,
		require: function(path) {
			return require_ejs(require.resolve(path), options, __mainfilename);
		},
		paths: Module._nodeModulePaths(dirname),
		filename: filename,
	};
	
	var result = `const __mainfilename = '${__mainfilename}';
								module.exports = ${template(ejs.source, options)}`;
	var wrapper = Module.wrap(result);

	var compiledWrapper = vm.runInThisContext(wrapper, {
		filename: filename,
		lineOffset: 0,
		displayErrors: true
	});

	var require = makeRequireFunction(mod);

	compiledWrapper.call(mod.exports, mod.exports, require, mod, filename, dirname);

	return mod.exports;
}

class ViewController extends HttpService {

	view(name, data) {
		var self = this;
		var ext = path.extname(name);
		var filename = util.resolve(ext ? name: name + '.ejs');
		data = data || {};
		
		try {
			var func = require_ejs(filename, data, filename);
			// fs.writeFileSync(__dirname + '/test.js', func + '');
			var str = func(data);
			self.returnHtml(str);
		} catch(err) {
			self.returnStatus(500, '<pre>' + err.message + '\n' + err.stack + '</pre>');
		}
	}
}

exports.ViewController = ViewController;
