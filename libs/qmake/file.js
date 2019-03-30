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

var util = require('qkit/util');
var service = require('qkit/service');
var HttpService = require('qkit/http_service').HttpService;
var StaticService = require('qkit/static_service').StaticService;
var path = require('qkit/path');
var fs = require('qkit/fs');
var keys = require('qkit/keys');
var Buffer = require('buffer').Buffer;
var remote_log = require('./remote_log');
var gen_html = require('./marked/html').gen_html;

var File = util.class('File', HttpService, {

	action: function(info) {
		var log = 'Request: ' + this.url;
		console.log(log);
		remote_log.remote_log_print(log);
		if ( /.+\.(mdown|md)/i.test(this.pathname) ) {
			return this.marked({pathname:this.pathname});
		} 
		else if ( /\/packages.json$/.test(this.pathname) ) {
			return this.packages_json({pathname:this.pathname});
		}
		else if ( /\/versions.json$/.test(this.pathname) ) {
			return this.versions_json({pathname:this.pathname});
		}
		HttpService.members.action.call(this, info);
	},

	marked_assets: function({pathname}) {
		// console.log('marked_assets', pathname);
		this.returnFile(path.resolveLocal(__dirname, '../marked/assets', pathname));
	},

	marked: function({pathname}) {
		var self = this;
		var filename = this.server.root + '/' + pathname;

		fs.stat(filename, function (err, stat) {
			
			if (err) {
				return self.returnStatus(404);
			}
			
			if (!stat.isFile()) {
				return self.returnStatus(404);
			}
			
			//for file
			if (stat.size > Math.min(self.server.maxFileSize, 5 * 1024 * 1024)) { 
				//File size exceeds the limit
				return self.returnStatus(403);
			}
			
			var mtime = stat.mtime;
			var ims = self.request.headers['if-modified-since'];
			var res = self.response;

			self.setDefaultHeader();
			res.setHeader('Last-Modified', mtime.toUTCString());
			res.setHeader('Content-Type', 'text/html; charset=utf-8');

			if (ims && new Date(ims) - mtime === 0) { //use 304 cache
				res.writeHead(304);
				res.end(); 
				return;
			}
			
			fs.readFile(filename, function(err, data) {
				if (err) {
					return self.returnStatus(404);
				}
				// template, title, text_md, no_index
				var res = self.response;
				var html = gen_html(data.toString('utf8')).html;
				res.writeHead(200);
				res.end(html);
			});

		});
	},

	packages_json: function({pathname}) {
		var self = this;
		var dir = path.resolveLocal(this.server.root, path.dirname(pathname));
		var res = self.response;

		if (fs.existsSync(dir + '/packages.json')) {
			self.returnFile(dir + '/packages.json');
		} 
		else {
			if ( fs.existsSync(dir) ) {
				var pkgs = { };
				var ls = fs.ls_sync(dir);
				fs.ls_sync(dir).forEach(function(stat) {
					if ( stat.isDirectory() ) {
						var pkg = dir + '/' + stat.name + '/package.json';
						if (fs.existsSync(pkg)) {
							var config = JSON.parse(fs.readFileSync(pkg, 'utf8'));
							pkgs[config.name] = config;
						}
					}
				});
				var data = JSON.stringify(pkgs, null, 2);
				self.setDefaultHeader();
				res.setHeader('Content-Type', 'application/json; charset=utf-8');
				res.writeHead(200);
				res.end(data);
			} else {
				this.returnStatus(404);
			}
		}
	},

	versions_json: function({pathname}) {
		var self = this;
		var dir = path.resolveLocal(this.server.root, path.dirname(pathname));
		var res = self.response;

		if (fs.existsSync(dir + '/versions.json')) {
			self.returnFile(dir + '/versions.json');
		} 
		else {
			var pkg = dir + '/package.json';
			
			if ( fs.existsSync(dir) && fs.existsSync(pkg) ) {
				var config = JSON.parse(fs.readFileSync(pkg, 'utf8'));
				var versions = { };

				dir = path.resolveLocal(dir, config.src || '');

				fs.ls_sync(dir, true, function(stat, pathname) {
					if ( stat.isFile() ) {
						versions[pathname] = util.hash(stat.mtime.valueOf() + '');
					}
				});
				
				var data = JSON.stringify({ versions: versions, pkg_files: {} }, null, 2);
				self.setDefaultHeader();
				res.setHeader('Content-Type', 'application/json; charset=utf-8');
				res.writeHead(200);
				res.end(data);
			} else {
				this.returnStatus(404);
			}
		}
	}
	
});

service.set('File', File);
