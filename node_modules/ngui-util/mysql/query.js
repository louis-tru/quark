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

var Query = exports.Query = util.class('Query', {

	/**
	 * query sql
	 * @type {String}
	 */
	sql: '',

	/**
	 * @event onerror
	 */
	onerror: null,

	/**
	 * @event onrow
	 */
	onrow: null,

	/**
	 * @event onfield
	 */
	onfield: null,

	/**
	 * @event onend
	 */
	onend: null,

	/**
	 * constructor function
	 * @param {String} sql
	 * @constructor
	 */
	constructor: function (sql) {
		event.init_events(this, 'row', 'field', 'end', 'error');
		this.sql = sql;
	},

	handlePacket: function (packet) {

		// We can't do this require() on top of the file.
		// That's because there is circular dependency and we're overwriting
		// module.exports
		var slef = this;

		switch (packet.type) {
			case parser.OK_PACKET:
				slef.onend.trigger(packet.toUserObject());
				break;
			case parser.ERROR_PACKET:
				packet.sql = slef.sql;
				slef.onerror.trigger(packet.toUserObject());
				break;
			case parser.FIELD_PACKET:
				if (!slef._fields)
					slef._fields = [];

				this._fields.push(packet);
				slef.onfield.trigger(packet);
				break;
			case parser.EOF_PACKET:
				if (!slef._eofs)
					slef._eofs = 1;
				else
					slef._eofs++;

				if (slef._eofs == 2)
					slef.onend.trigger();
				break;
			case parser.ROW_DATA_PACKET:
				var row = {};
				var field;
				slef._rowIndex = 0;
				slef._row = row;

				packet.ondata.on(function (e) {

					var data = e.data;
					var buffer = data.buffer;
					var remaining = data.remaining;

					if (!field) {
						field = slef._fields[slef._rowIndex];
						row[field.name] = '';
					}

					if (buffer)
						row[field.name] += buffer.toString('utf-8');
					else
						row[field.name] = null;

					if (remaining !== 0)
						return;

					slef._rowIndex++;
					//TODO
					// NOTE: need to handle more data types, such as binary data
					if (buffer !== null) {
						switch (field.fieldType) {
							case exports.FIELD_TYPE_TIMESTAMP:
							case exports.FIELD_TYPE_DATE:
							case exports.FIELD_TYPE_DATETIME:
							case exports.FIELD_TYPE_NEWDATE:
								row[field.name] = new Date(row[field.name]);
								break;
							case exports.FIELD_TYPE_TINY:
							case exports.FIELD_TYPE_SHORT:
							case exports.FIELD_TYPE_LONG:
							case exports.FIELD_TYPE_LONGLONG:
							case exports.FIELD_TYPE_INT24:
							case exports.FIELD_TYPE_YEAR:
								row[field.name] = parseInt(row[field.name], 10);
								break;
							case exports.FIELD_TYPE_FLOAT:
							case exports.FIELD_TYPE_DOUBLE:
								// decimal types cannot be parsed as floats because
								// V8 Numbers have less precision than some MySQL Decimals
								row[field.name] = parseFloat(row[field.name]);
								break;
							case exports.FIELD_TYPE_BIT:
								row[field.name] = row[field.name] == '\u0000' ? 0 : 1;
								break;
						}
					}
					
					if (slef._rowIndex == slef._fields.length) {
						delete slef._row;
						delete slef._rowIndex;
						slef.onrow.trigger(row);
						return;
					}

					field = null;
				});
				break;
		}
	},

});

exports = {
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
