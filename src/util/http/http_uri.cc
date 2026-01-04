/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "../http.h"
#include "../uv.h"

namespace qk {

	// ----------------------------- URL -----------------------------

	URI::URI(): _uritype(URI_UNKNOWN), _port(0) { }

	URI::URI(cString& src): _uritype(URI_UNKNOWN), _port(0), _href(src) {
		
		String s = src.substr(0, 9).to_lower_case();
		
		if ( s.index_of("file:///") == 0 ) {
			_uritype = URI_FILE;
			_pathname = src.substr(7);
			_origin = "file://";
		} else if ( s.index_of("zip:///") == 0 ) {
			_uritype = URI_ZIP;
			_pathname = src.substr(6);
			_origin = "zip://";
		}
		else {
			int start = 0;
			
			if ( s.index_of("http://") == 0 ) {
				start = 7;
				_uritype = URI_HTTP;
				_origin = "http://";
			} else if ( s.index_of("https://") == 0 ) {
				start = 8;
				_uritype = URI_HTTPS;
				_origin = "https://";
			} else if ( s.index_of("ws://") == 0 ) {
				start = 5;
				_uritype = URI_WS;
				_origin = "ws://";
			} else if ( s.index_of("wss://") == 0 ) {
				start = 6;
				_uritype = URI_WSS;
				_origin = "wss://";
			} else if ( s.index_of("ftp://") == 0 ) {
				start = 6;
				_uritype = URI_FTP;
				_origin = "ftp://";
			} else if ( s.index_of("ftps://") == 0 ) {
				start = 7;
				_uritype = URI_FTPS;
				_origin = "ftps://";
			} else if ( s.index_of("sftp://") == 0 ) {
				start = 7;
				_uritype = URI_SFTP;
				_origin = "sftp://";
			} else {
				return;
			}
			
			bool abort = false;
			int index = src.index_of('/', start);
			
			if (index == -1) {
				abort = true;
				_pathname = '/';
				_hostname = _host = src.substr(start);
			}
			else{
				_pathname = src.substr(index);
				_hostname = _host = src.substring(start, index);
			}
			
			_origin += _host;
			
			index = _hostname.last_index_of(':');
			if ( index != -1 ) {
				_port = _hostname.substr(index + 1).to_number<uint32_t>();
				_hostname = _hostname.substr(0, index);
			}
			
			_domain = _hostname;
			
			in_addr_t s_addr;
			
			if (uv_inet_pton(AF_INET, *_hostname, &s_addr) != 0) { // not ip address
				index = _domain.last_index_of('.');
				
				if (index != -1) {
					index = _domain.last_index_of('.', index - 1);
					if (index != -1) {
						_domain = _domain.substr(index + 1);
					}
				}
			}
			
			if (abort) return;
		}
		
		int index = _pathname.index_of('?');
		if (index != -1) {
			_search = _pathname.substr(index);
			_pathname = _pathname.substr(0, index);
		}
		
		index = _pathname.last_index_of('/');
		_dir = _pathname.substr(0, index);
		_basename = _pathname.substr(index + 1);
		
		index = _basename.last_index_of('.');
		if (index != -1) {
			_extname = _basename.substr(index).lower_case();
		}
	}

#define IN_RANGE(a,b,c) (a >= b && a <= c)

	static inline int needs_encoding(Char ch, Char next
																	 , bool component, bool secondary)
	{
		if ( ch < 0 ) {
			return 1;
		}

		// alpha capital/small
		if (IN_RANGE(ch, 0x0041, 0x005A) || IN_RANGE(ch, 0x061, 0x007A)) {
			return 0;
		}
		
		// decimal digits
		if (IN_RANGE(ch, 0x0030, 0x0039)) {
			return 0;
		}
		
		// reserved Chars
		// - _ . ! ~ * ' ( )
		switch (ch) {
			case '-':
			case '_':
			case '.':
			case '!':
			case '~':
			case '*':
			case '(':
			case ')':
				return 0;
			default: break;
		}
		
		if ( !component ) {
			
			switch (ch) {
				case ';':
				case '/':
				case '?':
				case ':':
				case '@':
				case '&':
				case '=':
				case '+':
				case '$':
				case ',':
				case '#': return 0;
				case '%':
					if ( secondary ) return 0; // secondary encoding
				default: break;
			}
		}
		
		return 1;
	}

	String uri_encode(cString& url, bool component, bool secondary) {
		uint8_t ch = 0;
		int len = url.length();
		cChar* src = *url;
		size_t msize = 0;
		
		for (int i = 0; i < len; ++i) {
			switch (needs_encoding(src[i], src[i+1], component, secondary)) {
				case -1: return String();
				case 0: msize++; break;
				case 1: msize = (msize + 3); break;
			}
		}
		
		// alloc with probable size
		Char* enc = (Char*)malloc((sizeof(Char) * msize) + 1);
		if ( !enc ) {
			return String();
		}
		
		uint32_t size = 0;
		cChar* hex = "0123456789ABCDEF";
		
		for ( int i = 0; i < len; i++ ) {
			ch = src[i];
			if (needs_encoding(ch, src[i], component, secondary)) {
				enc[size++] = '%';
				enc[size++] = hex[ch >> 4];
				enc[size++] = hex[ch & 0xf];
			} else {
				enc[size++] = ch;
			}
		}

		enc[size] = '\0';
		
		return Buffer::from(enc, uint32_t(size));
	}

	String URI::encode(cString &url, bool component) {
		return uri_encode(url, component, false);
	}

	String URI::decode(cString& url) {
		int i = 0;
		size_t size = 0;
		size_t len = 0;
		Char *dec = NULL;
		Char tmp[3];
		Char ch = 0;
		
		// Chars len
		len = url.length();
		cChar* src = *url;
		
		// alloc
		dec = (Char *) malloc(len + 1);
		
#define push(c) (dec[size++] = c)
		
		// decode
		while (len--) {
			ch = src[i++];
			
			// if prefix `%' then read uint8_t and decode
			if ('%' == ch) {
				tmp[0] = src[i++];
				tmp[1] = src[i++];
				tmp[2] = '\0';
				push(strtol(tmp, NULL, 16));
			} else {
				push(ch);
			}
		}
		
		dec[size] = '\0';
		
#undef push
		
		return Buffer::from(dec, uint32_t(size));
	}

}
