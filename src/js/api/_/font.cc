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
	 * @method set_default_fonts(fonts)
	 * @param fonts {String|Array}
	 */
	static void set_default_fonts(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_CHECK_APP();
		if ( args.length() < 1 ) {
			Js_Throw("Bad argument.");
		}
		if ( args[0]->IsString(worker) ) {
			font_pool()->set_default_fonts( args[0]->toStringValue(worker).split(',') );
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
	 * @method default_font_names()
	 * @return {Array}
	 */
	static void default_font_names(FunctionArgs args) {
		Js_Worker(args);
		Js_CHECK_APP();
		Array<String> arr = font_pool()->default_font_names();
		Js_Return(arr);
	}
	
	/**
	 * @method family_names()
	 * @return {Array}
	 */
	static void family_names(FunctionArgs args) {
		Js_Worker(args);
		Js_CHECK_APP();
		Array<String> arr = font_pool()->family_names();
		Js_Return(arr);
	}
		
	/**
	 * @method font_names(family)
	 * @param family {String}
	 * @return {Array}
	 */
	static void font_names(FunctionArgs args) {
		Js_Worker(args);
		Js_CHECK_APP();
		if ( args.length() < 1 ) {
			Js_Throw(
				"* @method fontNames(family)\n"
				"* @param family {String}\n"
				"* @return {Array}\n"
			);
		}
		Array<String> arr = font_pool()->font_names( args[0]->toStringValue(worker) );
		Js_Return(arr);
	}
	
	/**
	 * @method test(name) test font or family
	 * @param name {String} font name or family name
	 * @return {bool}
	 */
	static void test(FunctionArgs args) {
		Js_Worker(args);
		Js_CHECK_APP();
		if ( args.length() < 1 ) {
			Js_Throw(
				"* @method test(name) test font or family\n"
				"* @param name {String} font name or family name\n"
				"* @return {bool}\n"
			);
		}
		bool is = font_pool()->test( args[0]->toStringValue(worker) );
		Js_Return(is);
	}
	
	/**
	 * @method register_font(font_data[,alias])
	 * @param font_data {Buffer} 
	 * @param [alias] {String}
	 * @return {bool}
	 */
	static void register_font(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_CHECK_APP();
		if ( args.length() < 1 || !args[0]->IsBuffer() ) {
			Js_Throw(
				"* @method registerFont(font_data)\n"
				"* @param font_data {Buffer}\n"
				"* @param [alias] {String}\n"
				"* @return {bool}\n"
			);
		}

		auto buf = args[0]->AsBuffer(worker);
		String alias;
		
		if ( args.length() > 1 ) {
			alias = args[1]->toStringValue(worker);
		}
		Js_Return( font_pool()->register_font(buf.copy(), alias) );
	}
	
	/**
	 * @method register_font_file(path[,alias])
	 * @param path {String}
	 * @param alias {String}
	 * @return {bool}
	 */
	static void register_font_file(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_CHECK_APP();
		if ( args.length() < 1 || !args[0]->IsString(worker) ) {
			Js_Throw(
				"* @method registerFontFile(path[,alias])\n"
				"* @param path {String}\n"
				"* @param alias {String}\n"
				"* @return {bool}\n"
			);
		}
		String path = args[0]->toStringValue(worker);
		String alias;
		
		if ( args.length() > 1 ) {
			alias = args[1]->toStringValue(worker);
		}
		Js_Return( font_pool()->register_font_file(path, alias) );
	}
	
	/**
	 * @method set_family_alias(family, alias)
	 * @param family {String}
	 * @param alias {String}
	 */
	static void set_family_alias(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_CHECK_APP();
		if ( args.length() < 2 ) {
			Js_Throw(
				"* @method setFamilyAlias(family, alias)\n"
				"* @param family {String}\n"
				"* @param alias {String}\n"
			);
		}
		font_pool()->set_family_alias( args[0]->toStringValue(worker), args[1]->toStringValue(worker) );
	}
	
	static void binding(JSObject* exports, Worker* worker) {
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
