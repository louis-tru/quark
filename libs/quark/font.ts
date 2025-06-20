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
 * 所有字体类型管理
 * 
 * 一个FFID表示一组字体家庭，通常在一个文本需要多个字体来表示，首先、备选、异常
*/
export declare class FontPool {
	/** 当前读取的字体家族数量 */
	readonly countFamilies: number;
	/** 默认字体家族名称列表 */
	readonly defaultFamilyNames: string[]
	/** 默认字体家族名FFID */
	readonly defaultFontFamilies: types.FFID;

	/**
	 * @method getFontFamilies
	 * 
	 * 通过名称获取字体FFID，多个名称使用`,`分割
	 * 
	 * @param families? {string}
	 * @return {FFID}
	 *
	 * For examples:
	 *	```ts
	 *	var ffid = pool.getFontFamilies('黑体,PingFang-SC')
	 *	console.log(getFamiliesName(ffid))
	 *	```
	 */
	getFontFamilies(families?: string): types.FFID;

	/**
	 * @method addFontFamily
	 * 添加一个外部字体数据
	 * 
	 * @param data {Uint8Array} 字体Buffer数据
	 * @param alias? {string} 添加一个别名
	*/
	addFontFamily(data: Uint8Array, alias?: string): void;

	/**
	 * @method getFamilyName
	 * 
	 * 通过索引读取字体家族名称
	 * 
	 * @param index {uint}
	 * @return {string}
	*/
	getFamilyName(index: number): string;
}

/**
 * @method getFontFamilies
 * 
 * 通过名称字体家族名称获取字体家族对像FFID
 * 
 * @param families? {string}
*/
export declare function getFontFamilies(families?: string): types.FFID;

/**
 * @method getFamiliesName
 * 
 * 通过字体家族对像FFID获取名称
*/
export declare function getFamiliesName(ffid: types.FFID): string;