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
var { Buffer } = require('buffer');

var Query = util.class('Query', {

	_eofs: 0,

	/**
	 * query sql
	 * @type {String}
	 */
	sql: '',

	/**
	 * @event onError
	 */
	onError: null,

	/**
	 * @event onResolve
	 */
	onResolve: null,

	/**
	 * @event onField
	 */
	onField: null,

	/**
	 * @event onRow
	 */
	onRow: null,


	/**
	 * @event onEnd
	 */
	onEnd: null,

	/**
	 * @constructor
	 * @arg sql {String}
	 */
	constructor: function (sql) {
		event.initEvents(this, 'Resolve', 'Row', 'Field', 'End', 'Error');
		this.sql = sql;
	},

	handlePacket: function(packet) {

		// We can't do this require() on top of the file.
		// That's because there is circular dependency and we're overwriting
		// module.exports
		var self = this;

		switch (packet.type) {
			case parser.OK_PACKET:
				self.onResolve.trigger(packet.toUserObject());
				if (packet.serverStatus == 2) {
					self.onEnd.trigger();
				}
				break;
			case parser.ERROR_PACKET:
				packet.sql = self.sql;
				self.onError.trigger(packet.toUserObject());
				break;
			case parser.FIELD_PACKET:
				if (!self._fields) {
					self._fields = [];
					self.onResolve.trigger();
				}
				this._fields.push(packet);
				self.onField.trigger(packet);
				break;
			case parser.EOF_PACKET:
				if (!self._eofs) {
					self._eofs = 1;
				} else {
					self._eofs++; 
				}
				if (self._eofs == 2) {
					this._fields = null;
					self._eofs = 0;
					if (packet.serverStatus == 34 || packet.serverStatus == 2) {
						self.onEnd.trigger();
					}
				}
				break;
			case parser.ROW_DATA_PACKET:
				var row = {};
				var field, value;
				self._rowIndex = 0;
				self._row = row;

				packet.ondata.on(function (e) {

					var data = e.data;
					var buffer = data.buffer;
					var remaining = data.remaining;

					if (!field) {
						field = self._fields[self._rowIndex];
					}

					if (buffer) {
						if (value) {
							value = Buffer.concat([value, buffer]);
						} else {
							value = buffer;
						}
					}
					else {
						row[field.name] = value = null;
					}

					if (remaining !== 0) {
						return;
					}

					self._rowIndex++;
					// NOTE: need to handle more data types, such as binary data
					if (value !== null) {
						value = value.toString('utf8');

						switch (field.fieldType) {
							case exports.FIELD_TYPE_TIMESTAMP:
							case exports.FIELD_TYPE_DATE:
							case exports.FIELD_TYPE_DATETIME:
							case exports.FIELD_TYPE_NEWDATE:
								row[field.name] = new Date(value);
								break;
							case exports.FIELD_TYPE_TINY:
							case exports.FIELD_TYPE_SHORT:
							case exports.FIELD_TYPE_LONG:
							case exports.FIELD_TYPE_LONGLONG:
							case exports.FIELD_TYPE_INT24:
							case exports.FIELD_TYPE_YEAR:
								row[field.name] = parseInt(value, 10);
								break;
							case exports.FIELD_TYPE_FLOAT:
							case exports.FIELD_TYPE_DOUBLE:
								// decimal types cannot be parsed as floats because
								// V8 Numbers have less precision than some MySQL Decimals
								row[field.name] = parseFloat(value);
								break;
							case exports.FIELD_TYPE_BIT:
								row[field.name] = value == '\u0000' ? false : true;
								break;
							default:
								row[field.name] = value;
								break;
						}
					}
					
					if (self._rowIndex == self._fields.length) {
						delete self._row;
						delete self._rowIndex;
						self.onRow.trigger(row);
						return;
					}
					field = null;
					value = null;
				});
				break;
			default: break;
		}
	},

});

exports = module.exports = {
	Query: Query,
	FIELD_TYPE_DECIMAL: 0x00,
	FIELD_TYPE_TINY: 0x01,
	FIELD_TYPE_SHORT: 0x02,
	FIELD_TYPE_LONG: 0x03,
	FIELD_TYPE_FLOAT: 0x04,
	FIELD_TYPE_DOUBLE: 0x05,
	FIELD_TYPE_NULL: 0x06,
	FIELD_TYPE_TIMESTAMP: 0x07,
	FIELD_TYPE_LONGLONG: 0x08,
	FIELD_TYPE_INT24: 0x09,
	FIELD_TYPE_DATE: 0x0a,
	FIELD_TYPE_TIME: 0x0b,
	FIELD_TYPE_DATETIME: 0x0c,
	FIELD_TYPE_YEAR: 0x0d,
	FIELD_TYPE_NEWDATE: 0x0e,
	FIELD_TYPE_VARCHAR: 0x0f,
	FIELD_TYPE_BIT: 0x10,
	FIELD_TYPE_NEWDECIMAL: 0xf6,
	FIELD_TYPE_ENUM: 0xf7,
	FIELD_TYPE_SET: 0xf8,
	FIELD_TYPE_TINY_BLOB: 0xf9,
	FIELD_TYPE_MEDIUM_BLOB: 0xfa,
	FIELD_TYPE_LONG_BLOB: 0xfb,
	FIELD_TYPE_BLOB: 0xfc,
	FIELD_TYPE_VAR_STRING: 0xfd,
	FIELD_TYPE_STRING: 0xfe,
	FIELD_TYPE_GEOMETRY: 0xff
};
