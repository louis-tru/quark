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

#include "langou/utils/fs.h"
#include "langou/utils/jsx.h"
#include "langou/utils/codec.h" 

using namespace langou;

#define DEBUG_JSA 0
#define DEBUG_JSA_PATH "/Users/louis/Project/TouchCode/trunk/langou_ace/ace/Makefile.dryice.js"

#define error(err, ...) { XX_ERR(err, ##__VA_ARGS__); return 1; }

bool transform_js(cString& src, Ucs2String in, Buffer& out, bool jsx, bool clean_comment) {
#if DEBUG_JSA
	if ( jsx ) {
		out = Codec::encoding(Encoding::utf8, javascript_transform_x(in, src, clean_comment));
	} else {
		out = Codec::encoding(Encoding::utf8, javascript_transform(in, src, clean_comment));
	}
#else
	try {
		if ( jsx ) {
			out = Codec::encoding(Encoding::utf8, javascript_transform_x(in, src, clean_comment));
		} else {
			out = Codec::encoding(Encoding::utf8, javascript_transform(in, src, clean_comment));
		}
	} catch(Error& err) {
		error(err.message());
	}
#endif
	return 0;
}

int main(int argc, char* argv[]) {

#if DEBUG_JSA
	String src = DEBUG_JSA_PATH;
	String target = DEBUG_JSA_PATH"c";
#else
	if ( argc < 3 ) {
		error("Bad argument.");
	}
	String src = argv[1];
	String target = argv[2];
#endif
	
	if ( ! FileHelper::exists_sync(src) ) {
		error("Bad argument. cannot find %s", *src);
	}
	
	String extname = Path::extname(src).lower_case();
		
	Ucs2String in;
	Buffer out;
	bool clean_comment = 0;

	if (argc > 3) {
		if (strcmp(argv[3], "--clean-comment") == 0) {
			clean_comment = 1;
		}
	}
	
	in = Codec::decoding_to_uint16(Encoding::utf8, FileHelper::read_file_sync(src));
	
	int r = 0;
		
	if ( extname == ".js" ) {
		r = transform_js(src, in, out, false, clean_comment);
	} else if ( extname == ".jsx" ) {
		r = transform_js(src, in, out, true, clean_comment);
	} else {
		error("Bad argument.");
	}
	
	if ( r == 0 ) {
		FileHelper::write_file_sync(target, move(out));
	}

	return r;
}
