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

if (typeof requireNative != 'function') { // qgr
	require('./_ext');
}

const haveNode = !!global.process;
const haveQgr = !!global.requireNative;
const haveWeb = !!global.location;
const base64_chars =
	'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-'.split('');
const assign = Object.assign;

if (haveQgr) {
	var _util = requireNative('_util');
	var platform = _util.platform;
	var argv = _util.argv;
}
else if (haveNode) {
	var platform = process.platform;
	var argv = process.argv;
}
else if (haveWeb) {
	var platform = 'web';
	var argv = [];

	var flags = function() {
		var USER_AGENT = navigator.userAgent;
		var mat = USER_AGENT.match(/\(i[^;]+?; (U; )?CPU.+?OS (\d).+?Mac OS X/);
		var ios = !!mat;
		return {
			windows: USER_AGENT.indexOf('Windows') > -1,
			windowsPhone: USER_AGENT.indexOf('Windows Phone') > -1,
			linux: USER_AGENT.indexOf('Linux') > -1,
			android: /Android|Adr/.test(USER_AGENT),
			macos: USER_AGENT.indexOf('Mac OS X') > -1,
			ios: ios,
			iphone: USER_AGENT.indexOf('iPhone') > -1,
			ipad: USER_AGENT.indexOf('iPad') > -1,
			ipod: USER_AGENT.indexOf('iPod') > -1,
			mobile: USER_AGENT.indexOf('Mobile') > -1 || 'ontouchstart' in global,
			touch: 'ontouchstart' in global,
			//--
			trident: !!USER_AGENT.match(/Trident|MSIE/),
			presto: !!USER_AGENT.match(/Presto|Opera/),
			webkit: 
			USER_AGENT.indexOf('AppleWebKit') > -1 || 
			!!global.WebKitCSSKeyframeRule,
			gecko:
			USER_AGENT.indexOf('Gecko') > -1 &&
			USER_AGENT.indexOf('KHTML') == -1 || !!global.MozCSSKeyframeRule
		};
	}();
}

/**
 * defined class members func
 */
function def_class_members(klass, base, members) {
	if (base)
		members.__proto__ = base.prototype;
	klass.prototype = members;
}

function $class() {
	var args = Array.toArray(arguments);
	var name = args.shift();

	var base = args[0];
	var members = args[1];

	if (typeof base == 'object') {
		members = base;
		base = null;
	}
	var klass = null;

	if (members) {
		klass = members.constructor;
		if (typeof klass != 'function' || klass === Object) {
			klass = base ? function () { base.apply(this, arguments) } : function (){ };
		}
	} else {
		members = { };
		klass = base ? function () { base.apply(this, arguments) } : function (){ };
	}
	def_class_members(klass, base, members);
	klass.prototype.constructor = klass;
	klass.class_name = name;
	klass.base = base;
	klass.members = members;
	
	return klass;
}

function hashCode(data) {
	data = String(data);
	var _hash = 5381;
	var len = data.length;
	if (typeof data == 'string') {
		while (len--) _hash += (_hash << 5) + data.charCodeAt(len);
	} else {
		while (len--) _hash += (_hash << 5) + data[len];
	}
	return _hash & 0x7FFFFFFF
}

/**
	* @fun hash # gen hash value
	* @arg input {Object} 
	* @ret {String}
	*/
function hash(data) {
	var value = hashCode(data);
	var retValue = '';
	do {
		retValue += base64_chars[value & 0x3F];
	}
	while ( value >>= 6 );
	return retValue;
}

function unrealized() {
	throw new Error('Unrealized');
}

var nextTick = haveNode ? process.nextTick: function(cb, ...args) {
	if (typeof cb != 'function')
		throw new Error('callback must be a function');
	if (haveQgr) {
		_util.nextTick(e=>cb(...args));
	} else {
		setTimeout(e=>cb(...args), 1);
	}
};

module.exports = {
	unrealized: unrealized,
	version: unrealized,
	addNativeEventListener: unrealized,
	removeNativeEventListener: unrealized,
	garbageCollection: unrealized,
	runScript: unrealized,
	transformJsx: unrealized,
	transformJs: unrealized,
	hashCode: hashCode,
	hash: hash,
	class: $class,
	_eval: eval,
	nextTick: nextTick,
	platform: platform,
	haveNode: haveNode,
	haveQgr: haveQgr,
	haveWeb: haveWeb,
	argv: argv,
	flags: flags || {},
};
