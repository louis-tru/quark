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
var event = require('./event');
var fs = require('fs');
var Path = require('path');
var StringDecoder = require('string_decoder').StringDecoder;
var WriteStream = require('fs').WriteStream;
var querystring = require('querystring');
var Buffer = require('buffer').Buffer;

// This is a buffering parser, not quite as nice as the multipart one.
// If I find time I'll rewrite this to be fully streaming as well
/**
 * @class QuerystringParser
 * @private
 */
var QuerystringParser = util.class('QuerystringParser', {

	/**
	 * constructor function
	 * @constructor
	 */
	constructor: function () {
		this.buffer = '';
	},

	write: function (buffer) {
		this.buffer += buffer.toString('ascii');
		return buffer.length;
	},

	end: function () {
		var fields = querystring.parse(this.buffer);

		for (var field in fields) {
			this.onField(field, fields[field]);
		}
		this.buffer = '';

		this.onEnd();
	},
	// @end
});

// ----------------------- File -----------------------

/**
 * @File
 * @private
 */
 var File = util.class('File', {

	_writeStream: null,

	path: '',
	name: '',
	type: null,
	size: 0,
	lastModifiedDate: null,

	// @todo Next release: Show error messages when accessing these
	get length() {
		return this.size;
	},

	get filename() {
		return this.name;
	},

	get mime() {
		return this.type;
	},

	/**
	 * @event onprogress
	 */
	onprogress: null,

	/**
	 * @event onend
	 */
	onend: null,

	/**
	 * constructor function
	 * @param {Object} properties
	 * @constructor
	 */
	constructor: function (properties) {
		event.init_events(this, 'progress', 'end');
		util.ext(this, properties);
	},

	open: function () {
		this._writeStream = new WriteStream(this.path);
	},

	write: function (buffer, cb) {
		var self = this;

		if (!self._writeStream)
			self.open();

		self._writeStream.write(buffer, function () {
			self.lastModifiedDate = new Date();
			self.size += buffer.length;
			self.onprogress.trigger(self.size);
			cb();
		});
	},

	end: function (cb) {
		var self = this;

		if (self._writeStream) {
			self._writeStream.end(function () {
				self.onend.trigger();
				cb();
			});
		}
		else {
			self.path = '';
			self.onend.trigger();
			cb();
		}
	},
	// @end
});

// ----------------------- MultipartParser -----------------------

var s = 0,
S =
{ PARSER_UNINITIALIZED: s++,
	START: s++,
	START_BOUNDARY: s++,
	HEADER_FIELD_START: s++,
	HEADER_FIELD: s++,
	HEADER_VALUE_START: s++,
	HEADER_VALUE: s++,
	HEADER_VALUE_ALMOST_DONE: s++,
	HEADERS_ALMOST_DONE: s++,
	PART_DATA_START: s++,
	PART_DATA: s++,
	PART_END: s++,
	END: s++
},

f = 1,
F =
{
	PART_BOUNDARY: f,
	LAST_BOUNDARY: f *= 2
},

LF = 10,
CR = 13,
SPACE = 32,
HYPHEN = 45,
COLON = 58,
A = 97,
Z = 122,

lower = function (c) {
	return c | 0x20;
};

var MultipartParser = util.class('MultipartParser', {

	/**
	 * constructor function
	 * @constructor
	 */
	constructor: function () {
		this.boundary = null;
		this.boundaryChars = null;
		this.lookbehind = null;
		this.state = S.PARSER_UNINITIALIZED;

		this.index = null;
		this.flags = 0;
	},

	init_with_boundary: function (str) {
		this.boundary = new Buffer(str.length + 4);
		this.boundary.write('\r\n--', 0, 'ascii');
		this.boundary.write(str, 4, 'ascii');
		this.lookbehind = new Buffer(this.boundary.length + 8);
		this.state = S.START;

		this.boundaryChars = {};
		for (var i = 0; i < this.boundary.length; i++) {
			this.boundaryChars[this.boundary[i]] = true;
		}
	},
  
	write: function (buffer) {
		var self = this,
			i = 0,
			len = buffer.length,
			prevIndex = this.index,
			index = this.index,
			state = this.state,
			flags = this.flags,
			lookbehind = this.lookbehind,
			boundary = this.boundary,
			boundaryChars = this.boundaryChars,
			boundaryLength = this.boundary.length,
			boundaryEnd = boundaryLength - 1,
			bufferLength = buffer.length,
			c,
			cl,

			mark = function (name) {
				self[name + 'Mark'] = i;
			},
			clear = function (name) {
				delete self[name + 'Mark'];
			},
			callback = function (name, buffer, start, end) {
				if (start !== undefined && start === end) {
					return;
				}

				var callbackSymbol = 'on' + name.substr(0, 1).toUpperCase() + name.substr(1);
				if (callbackSymbol in self) {
					self[callbackSymbol](buffer, start, end);
				}
			},
			dataCallback = function (name, clear) {
				var markSymbol = name + 'Mark';
				if (!(markSymbol in self)) {
					return;
				}

				if (!clear) {
					callback(name, buffer, self[markSymbol], buffer.length);
					self[markSymbol] = 0;
				} else {
					callback(name, buffer, self[markSymbol], i);
					delete self[markSymbol];
				}
			};

		for (i = 0; i < len; i++) {
			c = buffer[i];
			switch (state) {
				case S.PARSER_UNINITIALIZED:
					return i;
				case S.START:
					index = 0;
					state = S.START_BOUNDARY;
				case S.START_BOUNDARY:
					if (index == boundary.length - 2) {
						if (c != CR) {
							return i;
						}
						index++;
						break;
					} else if (index - 1 == boundary.length - 2) {
						if (c != LF) {
							return i;
						}
						index = 0;
						callback('partBegin');
						state = S.HEADER_FIELD_START;
						break;
					}

					if (c != boundary[index + 2]) {
						return i;
					}
					index++;
					break;
				case S.HEADER_FIELD_START:
					state = S.HEADER_FIELD;
					mark('headerField');
					index = 0;
				case S.HEADER_FIELD:
					if (c == CR) {
						clear('headerField');
						state = S.HEADERS_ALMOST_DONE;
						break;
					}

					index++;
					if (c == HYPHEN) {
						break;
					}

					if (c == COLON) {
						if (index == 1) {
							// empty header field
							return i;
						}
						dataCallback('headerField', true);
						state = S.HEADER_VALUE_START;
						break;
					}

					cl = lower(c);
					if (cl < A || cl > Z) {
						return i;
					}
					break;
				case S.HEADER_VALUE_START:
					if (c == SPACE) {
						break;
					}

					mark('headerValue');
					state = S.HEADER_VALUE;
				case S.HEADER_VALUE:
					if (c == CR) {
						dataCallback('headerValue', true);
						callback('headerEnd');
						state = S.HEADER_VALUE_ALMOST_DONE;
					}
					break;
				case S.HEADER_VALUE_ALMOST_DONE:
					if (c != LF) {
						return i;
					}
					state = S.HEADER_FIELD_START;
					break;
				case S.HEADERS_ALMOST_DONE:
					if (c != LF) {
						return i;
					}

					callback('headersEnd');
					state = S.PART_DATA_START;
					break;
				case S.PART_DATA_START:
					state = S.PART_DATA
					mark('partData');
				case S.PART_DATA:
					prevIndex = index;

					if (index == 0) {
						// boyer-moore derrived algorithm to safely skip non-boundary data
						i += boundaryEnd;
						while (i < bufferLength && !(buffer[i] in boundaryChars)) {
							i += boundaryLength;
						}
						i -= boundaryEnd;
						c = buffer[i];
					}

					if (index < boundary.length) {
						if (boundary[index] == c) {
							if (index == 0) {
								dataCallback('partData', true);
							}
							index++;
						} else {
							index = 0;
						}
					} else if (index == boundary.length) {
						index++;
						if (c == CR) {
							// CR = part boundary
							flags |= F.PART_BOUNDARY;
						} else if (c == HYPHEN) {
							// HYPHEN = end boundary
							flags |= F.LAST_BOUNDARY;
						} else {
							index = 0;
						}
					} else if (index - 1 == boundary.length) {
						if (flags & F.PART_BOUNDARY) {
							index = 0;
							if (c == LF) {
								// unset the PART_BOUNDARY flag
								flags &= ~F.PART_BOUNDARY;
								callback('partEnd');
								callback('partBegin');
								state = S.HEADER_FIELD_START;
								break;
							}
						} else if (flags & F.LAST_BOUNDARY) {
							if (c == HYPHEN) {
								callback('partEnd');
								callback('end');
								state = S.END;
							} else {
								index = 0;
							}
						} else {
							index = 0;
						}
					}

					if (index > 0) {
						// when matching a possible boundary, keep a lookbehind reference
						// in case it turns out to be a false lead
						lookbehind[index - 1] = c;
					} else if (prevIndex > 0) {
						// if our boundary turned out to be rubbish, the captured lookbehind
						// belongs to partData
						callback('partData', lookbehind, 0, prevIndex);
						prevIndex = 0;
						mark('partData');

						// reconsider the current character even so it interrupted the sequence
						// it could be the beginning of a new sequence
						i--;
					}

					break;
				case S.END:
					break;
				default:
					return i;
			}
		}

		dataCallback('headerField');
		dataCallback('headerValue');
		dataCallback('partData');

		this.index = index;
		this.state = state;
		this.flags = flags;

		return len;
	},

	end: function () {
		if (this.state != S.END) {
			return new Error('MultipartParser.end(): stream ended unexpectedly: ' + this.explain());
		}
	},

	explain: function () {
		return 'state = ' + module.exports.state_to_string(this.state);
	},

});

// ----------------------- Part -----------------------

var Part = util.class('Part', {

	headers: null,
	name: '',
	filename: '',
	mime: '',
	headerField: '',
	headerValue: '',
  
	ondata: null,
	onend: null,
  
	constructor: function () {
		event.init_events(this, 'data', 'end');
		this.headers = {};
	}
});

// ----------------------- IncomingForm -----------------------

var temp_dir;
var dirs = ['/tmp', process.cwd()];

if (process.env.TMP) {
  dirs.unshift(process.env.TMP);
}

for (var i = 0; i < dirs.length; i++) {
	var dir = dirs[i];
	var isDirectory = false;

	try {
		isDirectory = fs.statSync(dir).isDirectory();
	} catch (e) { }

	if (isDirectory) {
		temp_dir = dir;
		break;
	}
}

function canceled(self){
	//
	for(var i in self.files){
		var files = self.files[i];
		for(var j = 0, len = files.length; i < len; i++){
			fs.unlink(files[j].path);
		}
	}
}

var IncomingForm = util.class('IncomingForm', {
  
	_parser: null,
	_flushing: 0,
	_fields_size: 0,
	_service: null,
  
	error: null,
	ended: false,
  
	/**
	 * default size 2MB
	 * @type {Number}
	 */
	max_fields_size: 5 * 1024 * 1024,
  
	/**
	 * default size 5MB
	 * @type {Number}
	 */
	max_files_size: 5 * 1024 * 1024,
  
	/**
	 * verify_file_mime 'js|jpg|jpeg|png' default as '*' ...
	 * @type {String}
	 */
	verify_file_mime: '*',
  
	/**
	 * is use file upload, default not upload
	 * @type {Boolean}
	 */
	is_upload: false,
  
	fields: null,
	files: null,
  
	keep_extensions: false,
	upload_dir: '',
	encoding: 'utf-8',
	headers: null,
	type: null,
  
	bytes_received: null,
	bytes_expected: null,
  
	onaborted: null,
	onprogress: null,
	onfield: null,
	onfile_begin: null,
	onfile: null,
	onerror: null,
	onend: null,
  
	/**
	 * constructor function
	 * @param {HttpService}
	 * @constructor
	 */
	constructor: function (service) {
	  // 
		event.init_events(this,
		  'aborted', 'progress', 'field', 'file_begin', 'file', 'error', 'end');

    this.upload_dir = service.server.temp;
		this._service = service;
		this.fields = { };
		this.files = { };
		this.max_fields_size = this._service.server.max_form_data_size;
		this.max_files_size = this._service.server.max_upload_file_size;
	},
  
	/**
	 * parse
	 */
	parse: function () {

		var self = this;
		var req = this._service.request;

		req.on('error', function (err) {
			self._error(err);
		});
		req.on('aborted', function () {
			canceled(self);
			self.onaborted.trigger();
		});
		req.on('data', function (buffer) {
			self.write(buffer);
		});
		req.on('end', function () {
			if (self.error) {
				return;
			}
			var err = self._parser.end();
			if (err) {
				self._error(err);
			}
		});

		this.headers = req.headers;
		this._parseContentLength();
		this._parseContentType();
	},

	write: function (buffer) {
		if (!this._parser) {
			this._error(new Error('unintialized parser'));
			return;
		}

		this.bytes_received += buffer.length;
		this.onprogress.trigger({ bytes_received: this.bytes_received, bytes_expected: this.bytes_expected });

		var bytesParsed = this._parser.write(buffer);
		if (bytesParsed !== buffer.length) {
			this._error(new Error('parser error, ' + bytesParsed + ' of ' + buffer.length + ' bytes parsed'));
		}

		return bytesParsed;
	},

	pause: function () {
		try {
			this._service.request.pause();
		} catch (err) {
			// the stream was destroyed
			if (!this.ended) {
				// before it was completed, crash & burn
				this._error(err);
			}
			return false;
		}
		return true;
	},

	resume: function () {
		try {
			this._service.request.resume();
		} catch (err) {
			// the stream was destroyed
			if (!this.ended) {
				// before it was completed, crash & burn
				this._error(err);
			}
			return false;
		}

		return true;
	},

	onpart: function (part) {
		// this method can be overwritten by the user
		this.handle_part(part);
	},

	handle_part: function (part) {
		var self = this;

		if (part.filename === undefined) {
			var value = '';
			var decoder = new StringDecoder(this.encoding);

			part.ondata.on(function (e) {
				var buffer = e.data;
				self._fields_size += buffer.length;
				if (self._fields_size > self.max_fields_size) {
					self._error(new Error('max_fields_size exceeded, received ' + self._fields_size + ' bytes of field data'));
					return;
				}
				value += decoder.write(buffer);
			});

			part.onend.on(function () {
				self._fields_size = 0;
				self.fields[part.name] = value;
				self.onfield.trigger({ name: part.name, value: value });
			});
			return;
		}

		if(!this.is_upload){
			return this._error(new Error('Does not allow file uploads'));
		}

		this._flushing++;

		var file = new File({
			path: this._uploadPath(part.filename),
			name: part.filename,
			type: part.mime
		});

		if (this.verify_file_mime != '*' && !new RegExp('\.(' + this.verify_file_mime + ')$', 'i').test(part.filename)) {
			return this._error(new Error('File mime error'));
		}

		this.onfile_begin.trigger({ name: part.name, file: file });

		part.ondata.on(function (e) {
			var buffer = e.data;
			self.pause();

			self._fields_size += buffer.length;
			if (self._fields_size > self.max_files_size) {

				file.end(function () {
					self._error(new Error('max_files_size exceeded, received ' + self._fields_size + ' bytes of field data'));
				});
				return;
			}

			file.write(buffer, function () {
				self.resume();
			});
		});

		part.onend.on(function () {
			self._fields_size = 0;
			file.end(function () {
				self._flushing--;

				var files = self.files[part.name];
				if (!files)
					self.files[part.name] = files = [];
				files.push(file);

				self.onfile.trigger({ name: part.name, file: file });
				self._maybeEnd();
			});
		});
	},

	_parseContentType: function () {

		var type = this.headers['content-type'];

		if (type && type.match(/multipart/i)) {
			var m;
			if (m = this.headers['content-type'].match(/boundary=(?:"([^"]+)"|([^;]+))/i)) {
				this._initMultipart(m[1] || m[2]);
			} else {
				this._error(new Error('bad content-type header, no multipart boundary'));
			}
		}
		else {
			this._initUrlencoded();
		}
	},

	_error: function (err) {

		if (this.error) {
			return;
		}

		canceled(this);

		this.error = err;
		this._service.request.socket.destroy(); //close socket connect
		this.onerror.trigger(err);
	},

	_parseContentLength: function () {
		if (this.headers['content-length']) {
			this.bytes_received = 0;
			this.bytes_expected = parseInt(this.headers['content-length'], 10);
		}
	},

	_newParser: function () {
		return new MultipartParser();
	},

	_initMultipart: function (boundary) {
		this.type = 'multipart';

		var parser = new MultipartParser();
		var self = this;
		var headerField = '';
		var headerValue = '';
		var part;

		parser.init_with_boundary(boundary);

		parser.onPartBegin = function () {
			part = new Part();
		};

		parser.onHeaderField = function (b, start, end) {
			headerField += b.toString(self.encoding, start, end);
		};

		parser.onHeaderValue = function (b, start, end) {
			headerValue += b.toString(self.encoding, start, end);
		};

		parser.onHeaderEnd = function () {

			headerField = headerField.toLowerCase();
			part.headers[headerField] = headerValue;

			var m;
			if (headerField == 'content-disposition') {
				if (m = headerValue.match(/name="([^"]+)"/i)) {
					part.name = m[1];
				}

				part.filename = self._fileName(headerValue);
			} else if (headerField == 'content-type') {
				part.mime = headerValue;
			}

			headerField = '';
			headerValue = '';
		};

		parser.onHeadersEnd = function () {
			self.onpart(part);
		};

		parser.onPartData = function (b, start, end) {
			part.ondata.trigger(b.slice(start, end));
		};

		parser.onPartEnd = function () {
			part.onend.trigger();
		};

		parser.onEnd = function () {
			self.ended = true;
			self._maybeEnd();
		};

		this._parser = parser;
	},

	_fileName: function (headerValue) {
		var m = headerValue.match(/filename="(.*?)"($|; )/i)
		if (!m) return;

		var filename = m[1].substr(m[1].lastIndexOf('\\') + 1);
		filename = filename.replace(/%22/g, '"');
		filename = filename.replace(/&#([\d]{4});/g, function (m, code) {
			return String.fromCharCode(code);
		});
		return filename;
	},

	_initUrlencoded: function () {
		this.type = 'urlencoded';

		var parser = new QuerystringParser()
		var self = this;

		parser.onField = function (name, value) {
			self.fields[name] = value;
			self.onfield.trigger({ name: name, value: value });
		};

		parser.onEnd = function () {
			self.ended = true;
			self._maybeEnd();
		};

		this._parser = parser;
	},

	_uploadPath: function (filename) {
		var name = '';
		for (var i = 0; i < 32; i++) {
			name += Math.floor(Math.random() * 16).toString(16);
		}

		if (this.keep_extensions) {
			var ext = Path.extname(filename);
			ext = ext.replace(/(\.[a-z0-9]+).*/, '$1');

			name += ext;
		}

		return Path.join(this.upload_dir, 'temp_upload_' + name);
	},

	_maybeEnd: function () {
		if (!this.ended || this._flushing) {
			return;
		}
		this.onend.trigger();
	},

});

// 
module.exports = {

	IncomingForm: IncomingForm,

  temp_dir: temp_dir,
  
  STATUS: S,

  state_to_string: function (stateNumber) {
  	for (var state in S) {
  		var number = S[state];
  		if (number === stateNumber) return state;
  	}
  },
};

