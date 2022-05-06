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

import util from 'somes';
import service from 'somes/service';
import { HttpService } from 'somes/http_service';
import path from 'somes/path';
import * as fs from 'somes/fs';
import * as remote_log from './remote_log';
import {gen_html} from './marked/html';

function resolveLocal(...args: string[]) {
	return path.fallbackPath(path.resolve(...args));
}

export default class File extends HttpService {

	async action(info: any) {
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
			return this.versions_json({ pathname:this.pathname });
		}
		super.action(info);
	}

	marked_assets({pathname}: {pathname:string}) {
		// console.log('marked_assets', pathname);
		this.returnFile(resolveLocal(__dirname, 'marked/assets', pathname));
	}

	marked({pathname}: {pathname:string}) {
		var self = this;
		var filename = this.server.root + '/' + pathname;

		return new Promise<void>((ok)=>{

			fs.stat(filename, function (err, stat) {

				if (err) {
					return self.returnErrorStatus(404), ok();
				}
				
				if (!stat.isFile()) {
					return self.returnErrorStatus(404), ok();
				}
				
				//for file
				if (stat.size > Math.min(self.server.maxFileSize, 5 * 1024 * 1024)) { 
					//File size exceeds the limit
					return self.returnErrorStatus(403), ok();
				}
				
				var mtime = stat.mtime;
				var ims = self.request.headers['if-modified-since'];
				var res = self.response;
	
				self.setDefaultHeader();
				res.setHeader('Last-Modified', mtime.toUTCString());
				res.setHeader('Content-Type', 'text/html; charset=utf-8');
	
				if (ims && new Date(ims).valueOf() - mtime.valueOf() === 0) { //use 304 cache
					res.writeHead(304);
					res.end(), ok();
					return;
				}
				
				fs.readFile(filename, function(err, data) {
					if (err) {
						return self.returnErrorStatus(404), ok();
					}
					// template, title, text_md, no_index
					var res = self.response;
					var html = gen_html(data.toString('utf8')).html;
					res.writeHead(200);
					res.end(html);
					ok();
				});
	
			});

		});
	}

	packages_json({pathname}: {pathname:string}) {
		var self = this;
		var dir = resolveLocal(this.server.root[0], path.dirname(pathname));
		var res = self.response;

		if (fs.existsSync(dir + '/packages.json')) {
			self.returnFile(dir + '/packages.json');
		} 
		else {
			if ( fs.existsSync(dir) ) {
				self.markReturnInvalid();

				var pkgs: Dict = {};
				(fs.listSync(dir) as fs.StatsDescribe[]).forEach(function(stat) {
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
				this.returnErrorStatus(404);
			}
		}
	}

	versions_json({pathname}: {pathname:string}) {
		var self = this;
		var dir = resolveLocal(this.server.root[0], path.dirname(pathname));
		var res = self.response;

		if (fs.existsSync(dir + '/versions.json')) {
			self.returnFile(dir + '/versions.json');
		}
		else {
			var pkg = dir + '/package.json';
			
			if ( fs.existsSync(dir) && fs.existsSync(pkg) ) {
				self.markReturnInvalid();

				var config = JSON.parse(fs.readFileSync(pkg, 'utf8'));
				var versions: Dict = {};

				dir = resolveLocal(dir, config.src || '');

				fs.ls_sync(dir, true, function(stat, pathname) {
					if (stat.name == '.git' || stat.name == '.svn')
						return true; // cancel each children
					var hash = util.hash(stat.mtime.valueOf() + '');
					if ( stat.isFile() ) {
						versions[pathname] = hash;
					} else if (pathname) {
						versions[pathname + '/packages.json'] = hash;
					} else {
						versions['packages.json'] = hash;
					}
				});

				var data = JSON.stringify({ versions: versions }, null, 2);
				self.setDefaultHeader();
				res.setHeader('Content-Type', 'application/json; charset=utf-8');
				res.writeHead(200);
				res.end(data);
			} else {
				this.returnErrorStatus(404);
			}
		}
	}
	
}

service.set('File', File);