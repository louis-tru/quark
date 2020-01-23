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

#include "ngui/js/ngui.h"
#include "ngui/font.h"
#include "ngui/draw.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

using namespace ngui;

/**
 * @class WrapFontStatic
 */
class WrapFontStatic {
 public:

	/**
	 * @func set_default_fonts(fonts)
	 * @arg fonts {String|Array}
	 */
	static void set_default_fonts(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_CHECK_APP();
		if ( args.Length() < 1 ) {
			JS_THROW_ERR("Bad argument.");
		}
		if ( args[0]->IsString(worker) ) {
			font_pool()->set_default_fonts( args[0]->ToStringValue(worker).split(',') );
		} else if ( args[0]->IsArray(worker) ) {
			Array<String> fonts;
			if ( args[0].To<JSArray>()->ToStringArrayMaybe(worker).To(fonts) ) {
				font_pool()->set_default_fonts( fonts );
			}
		} else {
			JS_THROW_ERR("Bad argument.");
		}
	}
	
	/**
	 * @func default_font_names()
	 * @ret {Array}
	 */
	static void default_font_names(FunctionCall args) {
		JS_WORKER(args);
		JS_CHECK_APP();
		Array<String> arr = font_pool()->default_font_names();
		JS_RETURN(arr);
	}
	
	/**
	 * @func family_names()
	 * @ret {Array}
	 */
	static void family_names(FunctionCall args) {
		JS_WORKER(args);
		JS_CHECK_APP();
		Array<String> arr = font_pool()->family_names();
		JS_RETURN(arr);
	}
		
	/**
	 * @func font_names(family)
	 * @arg family {String}
	 * @ret {Array}
	 */
	static void font_names(FunctionCall args) {
		JS_WORKER(args);
		JS_CHECK_APP();
		if ( args.Length() < 1 ) {
			JS_THROW_ERR(
				"* @func fontNames(family)\n"
				"* @arg family {String}\n"
				"* @ret {Array}\n"
			);
		}
		Array<String> arr = font_pool()->font_names( args[0]->ToStringValue(worker) );
		JS_RETURN(arr);
	}
	
	/**
	 * @func test(name) test font or family
	 * @arg name {String} font name or family name
	 * @ret {bool}
	 */
	static void test(FunctionCall args) {
		JS_WORKER(args);
		JS_CHECK_APP();
		if ( args.Length() < 1 ) {
			JS_THROW_ERR(
				"* @func test(name) test font or family\n"
				"* @arg name {String} font name or family name\n"
				"* @ret {bool}\n"
			);
		}
		bool is = font_pool()->test( args[0]->ToStringValue(worker) );
		JS_RETURN(is);
	}
	
	/**
	 * @func register_font(font_data[,alias])
	 * @arg font_data {Buffer} 
	 * @arg [alias] {String}
	 * @ret {bool}
	 */
	static void register_font(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_CHECK_APP();
		if ( args.Length() < 1 || !args[0]->IsBuffer() ) {
			JS_THROW_ERR(
				"* @func registerFont(font_data)\n"
				"* @arg font_data {Buffer}\n"
				"* @arg [alias] {String}\n"
				"* @ret {bool}\n"
			);
		}

    auto buf = args[0]->AsBuffer(worker);
		String alias;
		
		if ( args.Length() > 1 ) {
			alias = args[1]->ToStringValue(worker);
		}
		JS_RETURN( font_pool()->register_font(buf, alias) );
	}
	
	/**
	 * @func register_font_file(path[,alias])
	 * @arg path {String}
	 * @arg alias {String}
	 * @ret {bool}
	 */
	static void register_font_file(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_CHECK_APP();
		if ( args.Length() < 1 || !args[0]->IsString(worker) ) {
			JS_THROW_ERR(
				"* @func registerFontFile(path[,alias])\n"
				"* @arg path {String}\n"
				"* @arg alias {String}\n"
				"* @ret {bool}\n"
			);
		}
		String path = args[0]->ToStringValue(worker);
		String alias;
		
		if ( args.Length() > 1 ) {
			alias = args[1]->ToStringValue(worker);
		}
		JS_RETURN( font_pool()->register_font_file(path, alias) );
	}
	
	/**
	 * @func set_family_alias(family, alias)
	 * @arg family {String}
	 * @arg alias {String}
	 */
	static void set_family_alias(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_CHECK_APP();
		if ( args.Length() < 2 ) {
			JS_THROW_ERR(
				"* @func setFamilyAlias(family, alias)\n"
				"* @arg family {String}\n"
				"* @arg alias {String}\n"
			);
		}
		font_pool()->set_family_alias( args[0]->ToStringValue(worker), args[1]->ToStringValue(worker) );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_SET_METHOD(setDefaultFonts, set_default_fonts);
		JS_SET_METHOD(defaultFontNames, default_font_names);
		JS_SET_METHOD(familyNames, family_names);
		JS_SET_METHOD(fontNames, font_names);
		JS_SET_METHOD(test, test);
		JS_SET_METHOD(registerFont, register_font);
		JS_SET_METHOD(registerFontFile, register_font_file);
		JS_SET_METHOD(setFamilyAlias, set_family_alias);
	}
};

JS_REG_MODULE(_font, WrapFontStatic);

JS_END
