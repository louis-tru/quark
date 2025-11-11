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

import * as types from './types';
Object.assign(exports, __binding__('_font'));

/**
 * @class FontPool
 * 
 * Manage all font types.
 * 
 * An FFID represents a group of font families.
 * Usually a text needs multiple fonts to represent it, first, alternative, and exception.
*/
export declare class FontPool {
	/** The number of font families currently being read */
	readonly countFamilies: number;
	/** List of default font family names */
	readonly defaultFamilyNames: string[]
	/** Default font family name FFID */
	readonly defaultFontFamilies: types.FFID;

	/**
	 * Get the font FFID by name. Use `,` to separate multiple names.
	 *
	 * @example
	 *	```ts
	 *	var ffid = pool.getFontFamilies('黑体,PingFang-SC')
	 *	console.log(getFamiliesName(ffid))
	 *	```
	 */
	getFontFamilies(families?: string): types.FFID;

	/**
	 * Add an external font
	 *
	 * @param data Font Buffer Data
	 * @param alias? Add an alias
	 * @return The font family name
	*/
	addFontFamily(data: Uint8Array, alias?: string): string;

	/**
	 * Read font family name by index
	 * 
	 * @param index
	*/
	getFamilyName(index: Uint): string;
}

/**
 * Get the font family object FFID by font family name
*/
export declare function getFontFamilies(families?: string): types.FFID;

/**
 * Get the name by font family FFID
*/
export declare function getFamiliesName(ffid: types.FFID): string;