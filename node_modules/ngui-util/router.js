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
var static_service = require('./static_service');

var Router = util.class('Router', {

	/**
	 * 路由规则
	 * @type {Object[]}
	 */
	rules: null,
  
	/**
	 * Service to handle static files
	 * @type {String}
	 */
	static_service: 'StaticService',
  
	/**
	 * 构造函数
	 * @constructor
	 */
	constructor: function () {
		this.rules = [];
	},
  
	/**
	 * 设置路由器
	 * @param {Object} rules   路由配置
	 */
	config: function (conf) {
    
		var virtual = conf.virtual || '';
		util.update(this, { static_service: conf.static_service });
    
		var defines = [ 
		  /* 默认api调用路由 */ 
		  { match: '/?service={service}&method={action}' }, 
		  { match: '/?service={service}&multiple=1', action: 'multiple' },
		].concat(util.is_array(conf.router) ? conf.router : []);
    
		this.rules = [ ];
    
		for (var i = 0; i < defines.length; i++) {
      
      var item = defines[i];
			var rule = { 
			  match: null, // 用来匹配请求的url,如果成功匹配,把请求发送到目标服务处理
			  keys: [],    // 关键字信息,用来给目标服务提供参数
			  default_value: { } // 如果匹配成功后,做为目标服务的默认参数
			};
			
			// 创建表达式字符串
			// 替换{name}关键字表达式并且转义表达式中的特殊字符
			var match = (virtual + item.match)
				.replace(/\{([^\}]+)\}|[\|\[\]\(\)\{\}\?\.\+\*\!\^\$\:\<\>\=]/g,
				function (all, key) {
				  
					if (key) {
						rule.keys.push(key); // 添加一个关键字
						switch (key) {
							case 'service': return '([\\w\\$]+)';
							case 'action':  return '([\\w\\$]+)';
						}
						//return '([^&\?]+)'; // 至少匹配到一个字符
						return '([^&\?]*)';   // 匹配0到多个
					} else {
						return '\\' + all;  // 转义
					}
				});
      
      // 额外的url参数不需要在匹配范围,所以不必需从头匹配到尾
      rule.match = new RegExp('^' + match + (match.match(/\?/) ? '' : '(?:\\?|$)'));

      //console.log( rule.match )
      
			for (var j in item) {
				if (j != 'match') {
					rule.default_value[j] = item[j]; // 路由默认属性
				}
			}
      
      // 必需要有service、action 关键字,否则丢弃掉
			if ((rule.keys.indexOf('service') !== -1 || rule.default_value.service) &&
				  (rule.keys.indexOf('action') !== -1 || rule.default_value.action)) {
			  this.rules.push(rule);
			}
		}
	},


	/**
	 * find router info by url
	 * @param  {String} url
	 * @return {Object}
	 */
	find: function (url) {
    
		for (var i = 0; i < this.rules.length; i++) {
		  
			var item = this.rules[i];
			var mat = url.match(item.match);
			if (mat) {
			  
				var info = util.ext({ }, item.default_value);
				for (var j = 1; j < mat.length; j++) {
					info[item.keys[j - 1]] = mat[j];
				}
        
				return info;
			}
		}
    
    // 找不到任何匹配的服务,只能使用使用静态文件服务
		return {
			service: this.static_service,
			action: 'unknown'
		};
	},

});

exports.Router = Router;
