/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "../_view.h"
#include "../../font/pool.h"
#include "../../draw.h"

/**
 * @ns qk::js
 */

Js_BEGIN

using namespace qk;

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
		Js_Worker(args); UILock lock;
		Js_CHECK_APP();
		if ( args.Length() < 1 ) {
			Js_Throw("Bad argument.");
		}
		if ( args[0]->IsString(worker) ) {
			font_pool()->set_default_fonts( args[0]->ToStringValue(worker).split(',') );
		} else if ( args[0]->IsArray(worker) ) {
			Array<String> fonts;
			if ( args[0].To<JSArray>()->ToStringArrayMaybe(worker).To(fonts) ) {
				font_pool()->set_default_fonts( fonts );
			}
		} else {
			Js_Throw("Bad argument.");
		}
	}
	
	/**
	 * @func default_font_names()
	 * @ret {Array}
	 */
	static void default_font_names(FunctionCall args) {
		Js_Worker(args);
		Js_CHECK_APP();
		Array<String> arr = font_pool()->default_font_names();
		Js_Return(arr);
	}
	
	/**
	 * @func family_names()
	 * @ret {Array}
	 */
	static void family_names(FunctionCall args) {
		Js_Worker(args);
		Js_CHECK_APP();
		Array<String> arr = font_pool()->family_names();
		Js_Return(arr);
	}
		
	/**
	 * @func font_names(family)
	 * @arg family {String}
	 * @ret {Array}
	 */
	static void font_names(FunctionCall args) {
		Js_Worker(args);
		Js_CHECK_APP();
		if ( args.Length() < 1 ) {
			Js_Throw(
				"* @func fontNames(family)\n"
				"* @arg family {String}\n"
				"* @ret {Array}\n"
			);
		}
		Array<String> arr = font_pool()->font_names( args[0]->ToStringValue(worker) );
		Js_Return(arr);
	}
	
	/**
	 * @func test(name) test font or family
	 * @arg name {String} font name or family name
	 * @ret {bool}
	 */
	static void test(FunctionCall args) {
		Js_Worker(args);
		Js_CHECK_APP();
		if ( args.Length() < 1 ) {
			Js_Throw(
				"* @func test(name) test font or family\n"
				"* @arg name {String} font name or family name\n"
				"* @ret {bool}\n"
			);
		}
		bool is = font_pool()->test( args[0]->ToStringValue(worker) );
		Js_Return(is);
	}
	
	/**
	 * @func register_font(font_data[,alias])
	 * @arg font_data {Buffer} 
	 * @arg [alias] {String}
	 * @ret {bool}
	 */
	static void register_font(FunctionCall args) {
		Js_Worker(args); UILock lock;
		Js_CHECK_APP();
		if ( args.Length() < 1 || !args[0]->IsBuffer() ) {
			Js_Throw(
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
		Js_Return( font_pool()->register_font(buf.copy(), alias) );
	}
	
	/**
	 * @func register_font_file(path[,alias])
	 * @arg path {String}
	 * @arg alias {String}
	 * @ret {bool}
	 */
	static void register_font_file(FunctionCall args) {
		Js_Worker(args); UILock lock;
		Js_CHECK_APP();
		if ( args.Length() < 1 || !args[0]->IsString(worker) ) {
			Js_Throw(
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
		Js_Return( font_pool()->register_font_file(path, alias) );
	}
	
	/**
	 * @func set_family_alias(family, alias)
	 * @arg family {String}
	 * @arg alias {String}
	 */
	static void set_family_alias(FunctionCall args) {
		Js_Worker(args); UILock lock;
		Js_CHECK_APP();
		if ( args.Length() < 2 ) {
			Js_Throw(
				"* @func setFamilyAlias(family, alias)\n"
				"* @arg family {String}\n"
				"* @arg alias {String}\n"
			);
		}
		font_pool()->set_family_alias( args[0]->ToStringValue(worker), args[1]->ToStringValue(worker) );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		Js_Set_Method(setDefaultFonts, set_default_fonts);
		Js_Set_Method(defaultFontNames, default_font_names);
		Js_Set_Method(familyNames, family_names);
		Js_Set_Method(fontNames, font_names);
		Js_Set_Method(test, test);
		Js_Set_Method(registerFont, register_font);
		Js_Set_Method(registerFontFile, register_font_file);
		Js_Set_Method(setFamilyAlias, set_family_alias);
	}
};

Js_REG_MODULE(_font, WrapFontStatic);

Js_END
