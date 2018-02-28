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

exports.Cookie = util.class('Cookie', {

	//private:
	_req: null,
	_res: null,

	/**
	 * 构造函数
	 * @param {http.ServerResponse} req
	 * @param {http.ServerResponse} res
	 * @constructor
	 */
	constructor: function (req, res) {
		this._req = req;
		this._res = res;
	},
  
	/**
	 * 根据名字取Cookie值
	 * @param  {String}  name cookie的名称
	 * @return {String} 返回cookie值
	 */
	get: function (name) {
		var cookie = this._req.headers.cookie;
		if (cookie) {
			var i = cookie.match('(?:^|;\\s*){0}=([^;]+)(;|$)'.format(name));
			return i && decodeURIComponent(i[1]);
		}
		return null;
	},


	/**
	 * 设置cookie值
	 * @param {String}  name 名称
	 * @param {String}  value 值
	 * @param {Date}    expires (Optional) 过期时间
	 * @param {String}  path    (Optional)
	 * @param {String}  domain  (Optional)
	 * @param {Boolran} secure  (Optional)
	 */
	set: function (name, value, expires, path, domain, secure) {
		var setcookie = this._res.getHeader('Set-Cookie') || [];
		
		if (typeof setcookie == 'string')
			setcookie = [setcookie];
			
		for (var i = setcookie.length - 1; i > -1; i--) {
			if (setcookie[i].indexOf(name + '=') === 0)
				setcookie.splice(i, 1);
		}
		setcookie.push(
			'{0}={1}{2}{3}{4}{5}'.format(
				name,
				encodeURIComponent(value),
				expires ? '; Expires=' + expires.toUTCString() : '',
				path ? '; Path=' + path : '',
				domain ? '; Domain=' + domain : '',
				secure ? '; Secure' : ''
			));
		this._res.setHeader('Set-Cookie', setcookie);
	},


	/**
	 * 删除一个cookie
	 * @param {String}  name 名称
	 * @param {String}  path    (Optional)
	 * @param {String}  domain  (Optional)
	 */
	del: function (name, path, domain) {
		this.set(name, 'NULL', new Date(0, 1, 1), path, domain);
	},


	/**
	 * 获取全部Cookie
	 * @return {Object} 返回cookie值
	 */
	get_all: function () {
		var j = (this._req.headers.cookie || '').split(';');
		var cookie = { };
		
		for (var i = 0, len = j.length; i < len; i++) {
			var item = j[i];
			if (item) {
				item = item.split('=');
				cookie[item[0]] = decodeURIComponent(item[1]);
			}
		}
		return cookie;
	},

	/**
	 * 删除全部cookie
	 */
	del_all: function () {
		var cookie = this.get_all();
		for (var i in cookie)
			this.del(i);
	},
  // @end
});

