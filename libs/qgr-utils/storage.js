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
var url = require('./url');
var DelayCall = require('./delay_call').DelayCall;
var { haveNode, haveQgr, haveWeb } = util;

if (haveQgr) {
	var fs = requireNative('_fs');
} else if (haveNode) {
	var fs = require('fs');
}

var shared = null;

function sync_local(self) {
	if (!haveWeb && self.m_change) {
		fs.writeFileSync(self.m_path, JSON.stringify(self.m_value, null, 2));
		self.m_change = false;
	}
}

function format_key(self, key) {
	if (haveWeb) {
		return self.m_prefix + key;
	} else {
		return key;
	}
}

function commit(self) {
	if (!haveWeb) {
		self.m_change = true;
		self.m_sync.notice();
	}
}

/**
 * @class Storage
 */
class Storage {

	constructor(path = util.cwd() + '/' + '.storage') {
		this.m_path = url.fallbackPath(path);
		this.m_prefix = '';
		this.m_change = false;

		if (haveWeb) {
			this.m_sync = { notice: util.noop };
			this.m_prefix = util.hash(this.m_path || 'default') + '_';
			this.m_value = localStorage;
		} else {
			this.m_sync = new DelayCall(e=>sync_local(this), 100); // 100ms后保存到文件
			if (fs.existsSync(this.m_path)) {
				try {
					this.m_value = JSON.parse(fs.readFileSync(this.m_path, 'utf-8')) || {};
				} catch(e) {}
			}
		}
	}

	get(key, defaultValue) {
		key = format_key(this, key);
		if (key in this.m_value) {
			return this.m_value[key];
		} else {
			if (defaultValue !== undefined) {
				this.m_value[key] = defaultValue;
				commit(this);
				return defaultValue;
			}
		}
	}

	has(key) {
		key = format_key(this, key);
		return key in this.m_value;
	}

	set(key, value) {
		key = format_key(this, key);
		this.m_value[key] = value;
		commit(this);
	}

	del(key) {
		key = format_key(this, key);
		delete this.m_value[key];
		commit(this);
	}

	claer() {
		if (haveWeb) {
			var keys = [];
			for (var i in this.m_value) {
				if (i.substr(0, this.m_prefix.length) == this.m_prefix) {
					keys.push(this.m_value);
				}
			}
			for (var key of keys) {
				delete this.m_value[key];
			}
		} else {
			this.m_value = {};
		}
		commit(this);
	}

	save() {
		sync_local(this);
	}

}

module.exports = {

	Storage: Storage,

	get shared() {
		if (!shared) {
			shared = new Storage();
		}
		return shared;
	},

	setShared: function(value) {
		shared = value;
	},

	get: function(key, defaultValue) {
		return shared.get(key, defaultValue);
	},

	has: function(key) {
		return shared.has(key);
	},

	set: function(key, value) {
		return shared.set(key, value);
	},

	del: function(key) {
		return shared.del(key);
	},

	clear: function() {
		return shared.clear();
	},

	save: function() {
		shared.save();
	},

};
