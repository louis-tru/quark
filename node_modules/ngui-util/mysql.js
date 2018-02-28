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
var Database = require('./database').Database;
var constants = require('./mysql/constants');
var Query = require('./mysql/query').Query;
var OutgoingPacket = require('./mysql/qutgoing_packet').OutgoingPacket;
var connect_util = require('./mysql/connect');
var Buffer = require('buffer').Buffer;

//private:
//close back connect
function close (self) {
	var connect = self._connect;
	self._connect = null;
	self.connected = false;
	connect && connect.back && connect.back();
}

//net error and connection error
function connectionErrorHandler (self, err) {
	close(self);
	console.error(err);
  
	var task = self._queue[0];
	var cb = task ? task.cb : null;
  
	if (cb instanceof Query)
		return cb.onerror.trigger(err);
  
	if (cb)
		cb(err);
	else
		self.onerror.trigger(err);
	dequeue(self);
}

//onpacket handle
function handlePacket (e) {
	var self = this;
	var packet = e.data;

	// @TODO Simplify the code below and above as well
	var type = packet.type;
	var task = self._queue[0];
	var cb = task ? task.cb : null;

	if (cb instanceof Query)
		return cb.handlePacket(packet);

	//delete packet.ondata;
	if (type === Parser.ERROR_PACKET) {
		packet = packet.toUserObject();

		console.error(packet);
		if (cb)
			cb(packet);
		else
			self.onerror.trigger(packet);
	}
	else if (cb)
		cb(null, packet.toUserObject());

	dequeue(self);
}

//get connect
function connect (self) {

	var num = util.random(1);
	var opt = {
		host: self.host,
		port: self.port,
		user: self.user,
		password: self.password,
		database: self.database
	};
	self._connect = num;

	connect_util.get(opt, function (err, connect) {

		if (self._connect !== num) //not current connect
			return connect && connect.back();
		if (err)
			return connectionErrorHandler(self, err);

		self._connect = connect;
		self.connected = true;
		
		connect.onpacket.on(handlePacket, self);
		connect.onerror.on(function (e) {
			connectionErrorHandler(self, e.data);
		});
		self._queue[0].exec();
	});
}

//write packet
function write (self, packet) {
	self._connect.write(packet.buffer);
}

//enqueue
function enqueue (self, exec, cb) {
	if (!self._connect){
		connect(self);
	}
	var query = self._queue;

	query.push({ exec: exec, cb: cb });
	if (query.length === 1 && self.connected)
		exec();
}

//dequeue
function dequeue (self) {
	var queue = self._queue;

	queue.shift();
	if (!queue.length)
		return;
	if (!self._connect)
		return connect(self);

	self.connected && queue[0].exec();
}

//public:
var Mysql = util.class('Mysql', Database, {

	//private:
	_queue: null,
	_connect: null,
	_transaction: false,

	//public:
	port: 3306,

	/**
		* is connection
		* @type {Boolean}
		*/
	connected: false,

	/**
		* constructor function
		* @param {Object} opt (Optional)
		* @constructor
		*/
	constructor: function (opt) {
		Database.call(this);
		util.update(this, opt);
		this._queue = [];
	},

	//overlay
	statistics: function (cb) {
		var self = this;
		enqueue(self, function () {
			var packet = new OutgoingPacket(1);
			packet.writeNumber(1, constants.COM_STATISTICS);
			write(self, packet);
		}, cb);
	},

	//overlay
	query: function (sql, cb) {
		var self = this;
		var query = new Query(sql);

		if (cb) {
			var rows = [];
			var fields = {};

			query.onerror.on(function (e) {
				cb(e.data);
				dequeue(self);
			});
			query.onfield.on(function (e) {
				var field = e.data;
				fields[field.name] = field;
			});

			query.onrow.on(function (e) {
				rows.push(e.data);
			});
			query.onend.on(function (e) {
				var result = e.data;
				result ? cb(null, result) : cb(null, rows, fields);
				dequeue(self);
			});
		}

		else {

			query.onerror.on(function (e) {
				self.onerror.trigger(e.data);
				dequeue(self);
			});

			query.onend.on(function () {
				dequeue(self);
			});
		}

		enqueue(self, function () {

			var packet = new OutgoingPacket(1 + Buffer.byteLength(sql, 'utf-8'));

			packet.writeNumber(1, constants.COM_QUERY);
			packet.write(sql, 'utf-8');
			write(self, packet);
		}, query);

		return query;
	},

	//overlay
	close: function () {
		var self = this;

		if (this._transaction)
			this.commit();

		enqueue(self, function () {
			close(self);
			dequeue(self);
		});
	},

	//overlay
	transaction: function () {
		if (this._transaction)
			return;
		this._transaction = true;
		this.query('START TRANSACTION');
	},

	//overlay
	commit: function () {
		this._transaction = false;
		this.query('COMMIT');
	},

	//overlay
	rollback: function () {
		this._queue = [];
		this._transaction = false;
		this.query('ROLLBACK');
	},
	
});

exports.Mysql = Mysql;
