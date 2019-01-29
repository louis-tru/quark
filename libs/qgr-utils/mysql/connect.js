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

var util = require('../util');
var event = require('../event');
var parser = require('./parser');
var constants = require('./constants');
var auth = require('./auth');
var OutgoingPacket = require('./outgoing_packet').OutgoingPacket;
var Buffer = require('buffer').Buffer;
var Socket = require('net').Socket;

var connect_timout = 1e4;
var connect_pool = {};
var require_connect = [];
var { Parser, GREETING_PACKET, USE_OLD_PASSWORD_PROTOCOL_PACKET, ERROR_PACKET, } = parser;

function write(self, packet) {
	self._socket.write(packet.buffer);
}

function sendAuth(self, greeting) {
	var opt = self.opt;
	var token = auth.token(opt.password, greeting.scrambleBuffer);
	var packetSize = (
		4 + 4 + 1 + 23 +
		opt.user.length + 1 +
		token.length + 1 +
		opt.database.length + 1
	);
	var packet = new OutgoingPacket(packetSize, greeting.number + 1);

	packet.writeNumber(4, exports.DEFAULT_FLAGS);
	packet.writeNumber(4, exports.MAX_PACKET_SIZE);
	packet.writeNumber(1, exports.CHAREST_NUMBER);
	packet.writeFiller(23);
	packet.writeNullTerminated(opt.user);
	packet.writeLengthCoded(token);
	packet.writeNullTerminated(opt.database);

	write(self, packet);

	// Keep a reference to the greeting packet. We might receive a
	// USE_OLD_PASSWORD_PROTOCOL_PACKET as a response, in which case we will need
	// the greeting packet again. See sendOldAuth()
	self._greeting = greeting;
}

function sendOldAuth(self, greeting) {
	var token = auth.scramble323(greeting.scrambleBuffer, self.opt.password);
	var packetSize = (token.length + 1);

	var packet = new OutgoingPacket(packetSize, greeting.number + 3);

	// I could not find any official documentation for this, but from sniffing
	// the mysql command line client, I think this is the right way to send the
	// scrambled token after receiving the USE_OLD_PASSWORD_PROTOCOL_PACKET.
	packet.write(token);
	packet.writeFiller(1);

	write(self, packet);
}

function remove_connect(self) {
	self.onerror.off();
	self.onpacket.off();
	self._socket.destroy();

	var opt = self.opt;
	var key = opt.host + ':' + opt.port;
	var pool = connect_pool[key];

	pool.deleteValue(self);

	var queue = require_connect.shift();
	if (queue) {
		clearTimeout(queue.timeout);
		exports.get(...queue.args);
	}
}

var private$connect = util.class('private$connect', {

	//private:
	_greeting: null,
	_socket: null,
	_parser: null,
	_tomeout: 0,
	_use: true,

	//public:

	/**
	 * option
	 * @type {Object}
	 */
	opt: null,

	/**
	 * @event onerror
	 */
	onerror: null,

	/**
	 * @event onpacket
	 */
	onpacket: null,

	/**
	 * constructor function
	 * @param {Object}   opt
	 * @param {Function} cb
	 * @constructor
	 */
	constructor: function (opt, cb) {
		event.initEvents(this, 'error', 'packet');

		this.opt = opt;
		var self = this;
		var parser = self._parser = new Parser();
		var socket = self._socket = new Socket();

		socket.setTimeout(8e7);
		socket.setNoDelay(true);
		socket.on('data', function(data) {
			parser.write(data); 
		});
		socket.on('error', function (err) {
			self.onerror.trigger(err);
			remove_connect(self);
		});
		socket.on('end', function() {
			self.onerror.trigger(new Error('mysql server has been disconnected'));
			remove_connect(self);
		});

		socket.connect(opt.port, opt.host);

		parser.onpacket.on(function (e) {
			self.onpacket.trigger(e.data) 
		});
		self.onerror.on(function (e) {
			cb(e.data) 
		});
		self.onpacket.on(function (e) {
			var packet = e.data;

			if (packet.type == GREETING_PACKET) {
				return sendAuth(self, packet);
			}
			if (packet.type == USE_OLD_PASSWORD_PROTOCOL_PACKET) {
				return sendOldAuth(self, self._greeting);
			}
			//connection ok
			//error
			if (packet.type === ERROR_PACKET) {
				cb(packet.toUserObject());
				remove_connect(self);
			} else {
				self.onerror.off();
				self.onpacket.off();
				cb(null, self);
			}
		});
	},

	/**
	 * write buffer
	 * @param {node.Buffer}
	 */
	write: function (buffer) {
		this._socket.write(buffer);
	},

	/**
	 * return connection pool
	 */
	idle: function() {
		this.onerror.off();
		this.onpacket.off();
		this._use = false;

		for (var i = 0, l = require_connect.length, opt1 = this.opt; i < l; i++) {
			var req = require_connect[i];
			var args = req.args;
			var opt = args[0];

			if (
				opt.host == opt1.host &&
				opt.port === opt1.port &&
				opt.user == opt1.user &&
				opt.password == opt1.password
			) {
				require_connect.splice(i, 1);
				clearTimeout(req.timeout);
				return exports.get(...args);
			}
		}
		this._tomeout = remove_connect.setTimeout(connect_timout, this);
	},

	/**
		* start use connect
		*/
	use: function () {
		this._use = true;
		clearTimeout(this._tomeout);
	},

});

/**
 * get connect
 * @param {Object}   opt
 * @param {Function} cb
 */
exports.get = function (opt, cb) {

	var key = opt.host + ':' + opt.port;
	var pool = connect_pool[key] || (connect_pool[key] = []);

	for (var i = 0, l = pool.length; i < l; i++) {
		var connect = pool[i];
		var opt1 = { ...connect.opt };

		if (
			!connect._use &&
			opt1.user == opt.user &&
			opt1.password == opt.password
		) {
			connect.use();

			var db = opt.database;
			if (opt1.database == db) {
				return util.nextTick(cb, null, connect);
			}

			opt1.database = db;

			//init db
			var packet = new OutgoingPacket(1 + Buffer.byteLength(db, 'utf-8'));
			packet.writeNumber(1, constants.COM_INIT_DB);
			packet.write(db, 'utf-8');
			write(connect, packet);

			connect.onpacket.on(function(e) {
				connect.onpacket.off();
				var packet = e.data;
				if (packet.type === ERROR_PACKET) {
					connect.idle();
					cb(packet.toUserObject());
				} else {
					cb(null, connect);
				}
			});
			return;
		}
	}

	//is max connect
	if (pool.length < exports.MAX_CONNECT_COUNT) {
		return pool.push(new private$connect(opt, cb));
	}

	var req = {
		timeout: function() {
			require_connect.deleteValue(req);
			cb(new Error('obtaining a connection from the connection pool timeout'));
		} .setTimeout(connect_timout),
		args: Array.toArray(arguments)
	};
	
	//append to require connect
	require_connect.push(req);
};

/**
	* <span style="color:#f00">[static]</span>max connect count
	* @type {Numbet}
	* @static
	*/
exports.MAX_CONNECT_COUNT = 20;

/**
	* <b style="color:#f00">[static]</b>default flags
	* @type {Number}
	* @static
	*/
exports.DEFAULT_FLAGS = 
		constants.CLIENT_LONG_PASSWORD
	| constants.CLIENT_FOUND_ROWS
	| constants.CLIENT_LONG_FLAG
	| constants.CLIENT_CONNECT_WITH_DB
	| constants.CLIENT_ODBC
	| constants.CLIENT_LOCAL_FILES
	| constants.CLIENT_IGNORE_SPACE
	| constants.CLIENT_PROTOCOL_41
	| constants.CLIENT_INTERACTIVE
	| constants.CLIENT_IGNORE_SIGPIPE
	| constants.CLIENT_TRANSACTIONS
	| constants.CLIENT_RESERVED
	| constants.CLIENT_SECURE_CONNECTION
	| constants.CLIENT_MULTI_STATEMENTS
	| constants.CLIENT_MULTI_RESULTS;

/**
	* <b style="color:#f00">[static]</b>max packet size
	* @type {Number}
	* @static
	*/
exports.MAX_PACKET_SIZE = 0x01000000;

/**
	* <b style="color:#f00">[static]</b>charest number
	* @type {Number}
	* @static
	*/
exports.CHAREST_NUMBER = constants.UTF8_UNICODE_CI;

