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
var Buffer = require('buffer').Buffer;

var POWS = [1, 256, 65536, 16777216];
var p = 0;
var s = 0;
var CONSTANTS = {
	LENGTH_CODED_NULL: 251,
	LENGTH_CODED_16BIT_WORD: 252,
	LENGTH_CODED_24BIT_WORD: 253,
	LENGTH_CODED_64BIT_WORD: 254,

	// Parser states
	PACKET_LENGTH: s++,
	PACKET_NUMBER: s++,
	GREETING_PROTOCOL_VERSION: s++,
	GREETING_SERVER_VERSION: s++,
	GREETING_THREAD_ID: s++,
	GREETING_SCRAMBLE_BUFF_1: s++,
	GREETING_FILLER_1: s++,
	GREETING_SERVER_CAPABILITIES: s++,
	GREETING_SERVER_LANGUAGE: s++,
	GREETING_SERVER_STATUS: s++,
	GREETING_FILLER_2: s++,
	GREETING_SCRAMBLE_BUFF_2: s++,
	FIELD_COUNT: s++,
	ERROR_NUMBER: s++,
	ERROR_SQL_STATE_MARKER: s++,
	ERROR_SQL_STATE: s++,
	ERROR_MESSAGE: s++,
	AFFECTED_ROWS: s++,
	INSERT_ID: s++,
	SERVER_STATUS: s++,
	WARNING_COUNT: s++,
	MESSAGE: s++,
	EXTRA_LENGTH: s++,
	EXTRA_STRING: s++,
	FIELD_CATALOG_LENGTH: s++,
	FIELD_CATALOG_STRING: s++,
	FIELD_DB_LENGTH: s++,
	FIELD_DB_STRING: s++,
	FIELD_TABLE_LENGTH: s++,
	FIELD_TABLE_STRING: s++,
	FIELD_ORIGINAL_TABLE_LENGTH: s++,
	FIELD_ORIGINAL_TABLE_STRING: s++,
	FIELD_NAME_LENGTH: s++,
	FIELD_NAME_STRING: s++,
	FIELD_ORIGINAL_NAME_LENGTH: s++,
	FIELD_ORIGINAL_NAME_STRING: s++,
	FIELD_FILLER_1: s++,
	FIELD_CHARSET_NR: s++,
	FIELD_LENGTH: s++,
	FIELD_TYPE: s++,
	FIELD_FLAGS: s++,
	FIELD_DECIMALS: s++,
	FIELD_FILLER_2: s++,
	FIELD_DEFAULT: s++,
	EOF_WARNING_COUNT: s++,
	EOF_SERVER_STATUS: s++,
	COLUMN_VALUE_LENGTH: s++,
	COLUMN_VALUE_STRING: s++,

	// Packet types
	GREETING_PACKET: p++,
	OK_PACKET: p++,
	ERROR_PACKET: p++,
	RESULT_SET_HEADER_PACKET: p++,
	FIELD_PACKET: p++,
	EOF_PACKET: p++,
	ROW_DATA_PACKET: p++,
	ROW_DATA_BINARY_PACKET: p++,
	OK_FOR_PREPARED_STATEMENT_PACKET: p++,
	PARAMETER_PACKET: p++,
	USE_OLD_PASSWORD_PROTOCOL_PACKET: p++
};

/**
 * @class private$packet
 * @extends Object
 * @createTime 2012-01-12
 * @author louis.tru <louis.tru@gmail.com>
 * @copyright (C) 2011 louis.tru, http://mooogame.com
 * Released under MIT license, http://license.mooogame.com
 * @version 1.0
 */

var private$packet = util.class('private$packet', {

	/**
		* @event ondata
		*/
	ondata: null,

	index: 0,
	length: 0,
	received: 0,
	number: 0,
	type: CONSTANTS.LENGTH_CODED_NULL,

	/**
		* constructor function
		* @constructor
		*/
	constructor: function () {
		event.init_events(this, 'data');
	},

	/**
		* to user object
		* @return {Object}
		*/
	toUserObject: function () {
		var packet = this;
		var userObject = packet.type == exports.ERROR_PACKET ? new Error() : {};
		for (var key in packet) {
			var newKey = key;
			switch (key) {
				case 'type':
				case 'number':
				case 'length':
				case 'received':
				case 'ondata':
				case 'private$packet':
				case 'toUserObject':
					break;
				default:
					if (key == 'errorMessage')
						newKey = 'message';
					else if (key == 'errorNumber')
						newKey = 'number';
					userObject[newKey] = packet[key];
					break;
			}
		}
		
		return userObject;
	}

});


/**
 * @class Parser
 * @extends Object
 * @createTime 2012-01-12
 * @author louis.tru <louis.tru@gmail.com>
 * @copyright (C) 2011 louis.tru, http://mooogame.com
 * Released under MIT license, http://license.mooogame.com
 * @version 1.0
 */

var Parser = util.class('Parser', {

	_lengthCodedLength: null,
	_lengthCodedStringLength: null,

	state: CONSTANTS.PACKET_LENGTH,
	packet: null,
	greeted: false,
	authenticated: false,
	receivingFieldPackets: false,
	receivingRowPackets: false,

	/**
	 * @event onpacket
	 */
	onpacket: null,

	/**
	 * constructor function
	 * @constructor
	 */
	constructor: function () {
		event.init_events(this, 'packet');
	},

	/**
	 * write buffer and parser
	 * @param {node.Buffer}
	 */
	write: function (buffer) {
		var i = 0;
		var c = null;
		var self = this;
		var state = this.state;
		var length = buffer.length;
		var packet = this.packet;

		function advance(newState) {
			self.state = state = (newState === undefined)
				? self.state + 1
				: newState;
			packet.index = -1;
		}

		function lengthCoded(val, nextState) {
			if (self._lengthCodedLength === null) {
				if (c === exports.LENGTH_CODED_16BIT_WORD) {
					self._lengthCodedLength = 2;
				} else if (c === exports.LENGTH_CODED_24BIT_WORD) {
					self._lengthCodedLength = 3;
				} else if (c === exports.LENGTH_CODED_64BIT_WORD) {
					self._lengthCodedLength = 8;
				} else if (c === exports.LENGTH_CODED_NULL) {
					advance(nextState);
					return null;
				} else if (c < exports.LENGTH_CODED_NULL) {
					advance(nextState);
					return c;
				}

				return 0;
			}

			if (c) {
				val += POWS[packet.index - 1] * c;
			}

			if (packet.index === self._lengthCodedLength) {
				self._lengthCodedLength = null;
				advance(nextState);
			}

			return val;
		}

		function emitPacket() {
			self.packet = null;
			self.state = state = exports.PACKET_LENGTH;
			self.greeted = true;
			delete packet.index;
			self.onpacket.trigger(packet);
			packet = null;
		}

		for (; i < length; i++) {
			c = buffer[i];

			if (state > exports.PACKET_NUMBER) {
				packet.received++;
			}

			switch (state) {
				// PACKET HEADER
				case 0: // PACKET_LENGTH:
					if (!packet)
						packet = this.packet = new private$packet();

					// 3 bytes - Little endian
					packet.length += POWS[packet.index] * c;

					if (packet.index == 2) {
						advance();
					}
					break;
				case 1: // PACKET_NUMBER:
					// 1 byte
					packet.number = c;

					if (!this.greeted) {
						advance(exports.GREETING_PROTOCOL_VERSION);
						break;
					}

					if (this.receivingFieldPackets) {
						advance(exports.FIELD_CATALOG_LENGTH);
					} else if (this.receivingRowPackets) {
						advance(exports.COLUMN_VALUE_LENGTH);
					} else {
						advance(exports.FIELD_COUNT);
					}
					break;

				// GREETING_PACKET
				case 2: // GREETING_PROTOCOL_VERSION:
					// Nice undocumented MySql gem, the initial greeting can be an error
					// packet. Happens for too many connections errors.
					if (c === 0xff) {
						packet.type = exports.ERROR_PACKET;
						advance(exports.ERROR_NUMBER);
						break;
					}

					// 1 byte
					packet.type = exports.GREETING_PACKET;
					packet.protocolVersion = c;
					advance();
					break;
				case 3: // GREETING_SERVER_VERSION:
					if (packet.index == 0) {
						packet.serverVersion = '';
					}

					// Null-Terminated String
					if (c != 0) {
						packet.serverVersion += String.fromCharCode(c);
					} else {
						advance();
					}
					break;
				case 4: // GREETING_THREAD_ID:
					if (packet.index == 0) {
						packet.threadId = 0;
					}

					// 4 bytes = probably Little endian, protocol docs are not clear
					packet.threadId += POWS[packet.index] * c;

					if (packet.index == 3) {
						advance();
					}
					break;
				case 5: // GREETING_SCRAMBLE_BUFF_1:
					if (packet.index == 0) {
						packet.scrambleBuffer = new Buffer(8 + 12);
					}

					// 8 bytes
					packet.scrambleBuffer[packet.index] = c;

					if (packet.index == 7) {
						advance();
					}
					break;
				case 6: // GREETING_FILLER_1:
					// 1 byte - 0x00
					advance();
					break;
				case 7: // GREETING_SERVER_CAPABILITIES:
					if (packet.index == 0) {
						packet.serverCapabilities = 0;
					}
					// 2 bytes = probably Little endian, protocol docs are not clear
					packet.serverCapabilities += POWS[packet.index] * c;

					if (packet.index == 1) {
						advance();
					}
					break;
				case 8: // GREETING_SERVER_LANGUAGE:
					packet.serverLanguage = c;
					advance();
					break;
				case 9: // GREETING_SERVER_STATUS:
					if (packet.index == 0) {
						packet.serverStatus = 0;
					}

					// 2 bytes = probably Little endian, protocol docs are not clear
					packet.serverStatus += POWS[packet.index] * c;

					if (packet.index == 1) {
						advance();
					}
					break;
				case 10: // GREETING_FILLER_2:
					// 13 bytes - 0x00
					if (packet.index == 12) {
						advance();
					}
					break;
				case 11: // GREETING_SCRAMBLE_BUFF_2:
					// 12 bytes - not 13 bytes like the protocol spec says ...
					if (packet.index < 12) {
						packet.scrambleBuffer[packet.index + 8] = c;
					}
					break;

				// OK_PACKET, ERROR_PACKET, or RESULT_SET_HEADER_PACKET
				case 12: // FIELD_COUNT:
					if (packet.index == 0) {
						if (c === 0xff) {
							packet.type = exports.ERROR_PACKET;
							advance(exports.ERROR_NUMBER);
							break;
						}

						if (c == 0xfe && !this.authenticated) {
							packet.type = exports.USE_OLD_PASSWORD_PROTOCOL_PACKET;
							break;
						}

						if (c === 0x00) {
							// after the first OK PACKET, we are authenticated
							this.authenticated = true;
							packet.type = exports.OK_PACKET;
							advance(exports.AFFECTED_ROWS);
							break;
						}
					}

					this.receivingFieldPackets = true;
					packet.type = exports.RESULT_SET_HEADER_PACKET;
					packet.fieldCount = lengthCoded(packet.fieldCount, exports.EXTRA_LENGTH);

					break;

				// ERROR_PACKET
				case 13: // ERROR_NUMBER:
					if (packet.index == 0) {
						packet.errorNumber = 0;
					}

					// 2 bytes = Little endian
					packet.errorNumber += POWS[packet.index] * c;

					if (packet.index == 1) {
						if (!this.greeted) {
							// Turns out error packets are confirming to the 4.0 protocol when
							// not greeted yet. Oh MySql, you are such a thing of beauty ...
							advance(exports.ERROR_MESSAGE);
							break;
						}

						advance();
					}
					break;
				case 14: // ERROR_SQL_STATE_MARKER:
					// 1 character - always #
					packet.sqlStateMarker = String.fromCharCode(c);
					packet.sqlState = '';
					advance();
					break;
				case 15: // ERROR_SQL_STATE:
					// 5 characters
					if (packet.index < 5) {
						packet.sqlState += String.fromCharCode(c);
					}

					if (packet.index == 4) {
						advance(exports.ERROR_MESSAGE);
					}
					break;
				case 16: // ERROR_MESSAGE:
					if (packet.received <= packet.length) {
						packet.errorMessage = (packet.errorMessage || '') + String.fromCharCode(c);
					}
					break;

				// OK_PACKET
				case 17: // AFFECTED_ROWS:
					packet.affectedRows = lengthCoded(packet.affectedRows);
					break;
				case 18: // INSERT_ID:
					packet.insertId = lengthCoded(packet.insertId);
					break;
				case 19: // SERVER_STATUS:
					if (packet.index == 0) {
						packet.serverStatus = 0;
					}

					// 2 bytes - Little endian
					packet.serverStatus += POWS[packet.index] * c;

					if (packet.index == 1) {
						advance();
					}
					break;
				case 20: // WARNING_COUNT:
					if (packet.index == 0) {
						packet.warningCount = 0;
					}

					// 2 bytes - Little endian
					packet.warningCount += POWS[packet.index] * c;

					if (packet.index == 1) {
						packet.message = '';
						advance();
					}
					break;
				case 21: // MESSAGE:
					if (packet.received <= packet.length) {
						packet.message += String.fromCharCode(c);
					}
					break;

				// RESULT_SET_HEADER_PACKET
				case 22: // EXTRA_LENGTH:
					packet.extra = '';
					self._lengthCodedStringLength = lengthCoded(self._lengthCodedStringLength);
					break;
				case 23: // EXTRA_STRING:
					packet.extra += String.fromCharCode(c);
					break;

				// FIELD_PACKET or EOF_PACKET
				case 24: // FIELD_CATALOG_LENGTH:
					if (packet.index == 0) {
						if (c === 0xfe) {
							packet.type = exports.EOF_PACKET;
							advance(exports.EOF_WARNING_COUNT);
							break;
						}
						packet.type = exports.FIELD_PACKET;
					}
					self._lengthCodedStringLength = lengthCoded(self._lengthCodedStringLength);
					break;
				case 25: // FIELD_CATALOG_STRING:
					if (packet.index == 0) {
						packet.catalog = '';
					}
					packet.catalog += String.fromCharCode(c);

					if (packet.index + 1 === self._lengthCodedStringLength) {
						advance();
					}
					break;
				case 26: // FIELD_DB_LENGTH:
					self._lengthCodedStringLength = lengthCoded(self._lengthCodedStringLength);
					if (self._lengthCodedStringLength == 0) {
						advance();
					}
					break;
				case 27: // FIELD_DB_STRING:
					if (packet.index == 0) {
						packet.db = '';
					}
					packet.db += String.fromCharCode(c);

					if (packet.index + 1 === self._lengthCodedStringLength) {
						advance();
					}
					break;
				case 28: // FIELD_TABLE_LENGTH:
					self._lengthCodedStringLength = lengthCoded(self._lengthCodedStringLength);
					if (self._lengthCodedStringLength == 0) {
						advance();
					}
					break;
				case 29: // FIELD_TABLE_STRING:
					if (packet.index == 0) {
						packet.table = '';
					}
					packet.table += String.fromCharCode(c);

					if (packet.index + 1 === self._lengthCodedStringLength) {
						advance();
					}
					break;
				case 30: // FIELD_ORIGINAL_TABLE_LENGTH:
					self._lengthCodedStringLength = lengthCoded(self._lengthCodedStringLength);
					if (self._lengthCodedStringLength == 0) {
						advance();
					}
					break;
				case 31: // FIELD_ORIGINAL_TABLE_STRING:
					if (packet.index == 0) {
						packet.originalTable = '';
					}
					packet.originalTable += String.fromCharCode(c);

					if (packet.index + 1 === self._lengthCodedStringLength) {
						advance();
					}
					break;
				case 32: // FIELD_NAME_LENGTH:
					self._lengthCodedStringLength = lengthCoded(self._lengthCodedStringLength);
					break;
				case 33: // FIELD_NAME_STRING:
					if (packet.index == 0) {
						packet.name = '';
					}
					packet.name += String.fromCharCode(c);

					if (packet.index + 1 === self._lengthCodedStringLength) {
						advance();
					}
					break;
				case 34: // FIELD_ORIGINAL_NAME_LENGTH:
					self._lengthCodedStringLength = lengthCoded(self._lengthCodedStringLength);
					if (self._lengthCodedStringLength == 0) {
						advance();
					}
					break;
				case 35: // FIELD_ORIGINAL_NAME_STRING:
					if (packet.index == 0) {
						packet.originalName = '';
					}
					packet.originalName += String.fromCharCode(c);

					if (packet.index + 1 === self._lengthCodedStringLength) {
						advance();
					}
					break;
				case 36: // FIELD_FILLER_1:
					// 1 bytes - 0x00
					advance();
					break;
				case 37: // FIELD_CHARSET_NR:
					if (packet.index == 0) {
						packet.charsetNumber = 0;
					}

					// 2 bytes - Little endian
					packet.charsetNumber += Math.pow(256, packet.index) * c;

					if (packet.index == 1) {
						advance();
					}
					break;
				case 38: // FIELD_LENGTH:
					if (packet.index == 0) {
						packet.fieldLength = 0;
					}

					// 4 bytes - Little endian
					packet.fieldLength += Math.pow(256, packet.index) * c;

					if (packet.index == 3) {
						advance();
					}
					break;
				case 39: // FIELD_TYPE:
					// 1 byte
					packet.fieldType = c;
					advance();
				case 40: // FIELD_FLAGS:
					if (packet.index == 0) {
						packet.flags = 0;
					}

					// 2 bytes - Little endian
					packet.flags += Math.pow(256, packet.index) * c;

					if (packet.index == 1) {
						advance();
					}
					break;
				case 41: // FIELD_DECIMALS:
					// 1 byte
					packet.decimals = c;
					advance();
					break;
				case 42: // FIELD_FILLER_2:
					// 2 bytes - 0x00
					if (packet.index == 1) {
						advance();
					}
					break;
				case 43: // FIELD_DEFAULT:
					// TODO: Only occurs for mysql_list_fields()
					break;

				// EOF_PACKET
				case 44: // EOF_WARNING_COUNT:
					if (packet.index == 0) {
						packet.warningCount = 0;
					}

					// 2 bytes - Little endian
					packet.warningCount += Math.pow(256, packet.index) * c;

					if (packet.index == 1) {
						advance();
					}
					break;
				case 45: // EOF_SERVER_STATUS:
					if (packet.index == 0) {
						packet.serverStatus = 0;
					}

					// 2 bytes - Little endian
					packet.serverStatus += Math.pow(256, packet.index) * c;

					if (packet.index == 1) {
						if (this.receivingFieldPackets) {
							this.receivingFieldPackets = false;
							this.receivingRowPackets = true;
						} else {
						}
					}
					break;
				case 46: // COLUMN_VALUE_LENGTH:
					if (packet.index == 0) {
						packet.columnLength = 0;
						packet.type = exports.ROW_DATA_PACKET;
					}

					if (packet.received == 1) {
						if (c === 0xfe) {
							packet.type = exports.EOF_PACKET;
							this.receivingRowPackets = false;
							advance(exports.EOF_WARNING_COUNT);
							break;
						}
						this.onpacket.trigger(packet);
					}

					packet.columnLength = lengthCoded(packet.columnLength);

					if (!packet.columnLength && !this._lengthCodedLength) {
						packet.ondata.trigger({ buffer: packet.columnLength === null ? null : new Buffer(0), remaining: 0 });
						if (packet.received < packet.length) {
							advance(exports.COLUMN_VALUE_LENGTH);
						} else {
							self.packet = packet = null;
							self.state = state = exports.PACKET_LENGTH;
							continue;
						}
					}
					break;
				case 47: // COLUMN_VALUE_STRING:
					var remaining = packet.columnLength - packet.index, read;
					if (i + remaining > buffer.length) {
						read = buffer.length - i;
						packet.index += read;
						packet.ondata.trigger({ buffer: buffer.slice(i, buffer.length), remaining: remaining - read });
						// the -1 offsets are because these values are also manipulated by the loop itself
						packet.received += read - 1;
						i = buffer.length;
					} else {
						packet.ondata.trigger({ buffer: buffer.slice(i, i + remaining), remaining: 0 });
						i += remaining - 1;
						packet.received += remaining - 1;
						advance(exports.COLUMN_VALUE_LENGTH);
						// advance() sets this to -1, but packet.index++ is skipped, so we need to manually fix
						packet.index = 0;
					}

					if (packet.received == packet.length) {
						self.packet = packet = null;
						self.state = state = exports.PACKET_LENGTH;
					}

					continue;
			}

			packet.index++;

			if (state > exports.PACKET_NUMBER && packet.received === packet.length) {
				emitPacket();
			}
		}
	}

});

module.exports = CONSTANTS;
module.exports.Parser = Parser;
