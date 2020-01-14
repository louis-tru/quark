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

interface RequireFunction {
	(id: string): any;
}

interface RequireResolve {
	(id: string, options?: { paths?: string[]; }): string;
	paths(request: string): string[] | null;
}

interface NguiExtensions {
	'.js': (m: NguiModule, filename: string) => any;
	'.json': (m: NguiModule, filename: string) => any;
	'.node': (m: NguiModule, filename: string) => any;
	[ext: string]: (m: NguiModule, filename: string) => any;
}

interface NguiRequire extends RequireFunction {
	resolve: RequireResolve;
	cache: Dict<NguiModule>;
	/**
	 * @deprecated
	 */
	extensions: NguiExtensions;
	main: NguiModule | undefined;
}

interface NguiModule {
	exports: any;
	require: RequireFunction;
	id: string;
	filename: string;
	loaded: boolean;
	parent: NguiModule | null;
	children: NguiModule[];
	paths: string[];
}

declare var __requireNgui__: RequireFunction;
declare var require: NguiRequire;
declare var module: NguiModule;
// Same as module.exports
declare var exports: any;

interface ObjectConstructor {
	hashCode(obj: any): number;
}

interface Object {
	hashCode(): number;
}

// Dictionaries
interface Dict<T = any> {
	[key: string]: T;
}

type TimeoutResult = any; // NodeJS.Timeout | number;

interface Function {
	hashCode(): number;
	setTimeout(this: Function, time: number, ...argArray: any[]): TimeoutResult;
}

interface CallableFunction extends Function {
	setTimeout<A extends any[], R>(this: (...args: A) => R, time: number, ...args: A): TimeoutResult;
}

interface ArrayConstructor {
	toArray(obj: any, index?: number, end?: number): any[];
}

interface Array<T> {
	hashCode(): number;
	deleteOf(value: T): T[];
	indexReverse(index: number): T;
}

interface StringConstructor {
	format(str: string, ...args: any[]): string;
}

interface String {
	hashCode(): number;
}

interface Number {

	hashCode(): number;

	/**
	* 转换为前后固定位数的字符串
	* @arg before {Number}  小数点前固定位数
	* @arg [after] {Number} 小数点后固定位数
	*/
	toFixedBefore(before: number, after?: number): string;

}

interface Boolean {
	hashCode(): number;
}

interface DateConstructor {

	/**
	 * @field current timezone
	 */
	currentTimezone: number;

	/**
	 * 解析字符串为时间
	 * <pre><code>
	 * var i = '2008-02-13 01:12:13';
	 * var date = Date.parseDate(i); //返回的新时间
	 * </code></pre>
	 * @func parseDate(str[,format[,timezone]])
	 * @arg str {String}        要解析的字符串
	 * @arg [format] {String}   date format   default yyyyMMddhhmmssfff
	 * @arg [timezone] {Number} 要解析的时间所在时区,默认为当前时区
	 * @ret {Date}              返回新时间
	 */
	parseDate(date_str: string, format?: string, timezone?: number): Date;

	/**
		* 格式化时间戳(单位:毫秒)
		* <pre><code>
		* var time_span = 10002100;
		* var format = 'dd hh:mm:ss';
		* var str = Date.formatTimeSpan(time_span, format); // str = '0 2:46:42'
		* var format = 'dd天hh时mm分ss秒';
		* var str = Date.formatTimeSpan(time_span, format); // str = '0天2时46分42秒'
		* format = 'hh时mm分ss秒';
		* str = Date.formatTimeSpan(time_span, format); // str = '2时46分42秒'
		* format = 'mm分ss秒';
		* str = Date.formatTimeSpan(time_span, format); // str = '166分42秒'
		* </code></pre>
		* @func formatTimeSpan(ts[,format])
		* @arg ts {Number} 要格式化的时间戳
		* @arg [format]  {String} 要格式化的时间戳格式
		* @ret {String} 返回的格式化后的时间戳
		*/
	formatTimeSpan(time_span: number, format?: string): string;

}

interface Date {

	hashCode(): number;

	/**
	 * @func add 给当前Date时间追加毫秒,改变时间值
	 * @arg ms {Number}  要添追加的毫秒值
	 * @ret {Date}
	 */
	add(ms: number): Date;

	/**
		* 给定日期格式返回日期字符串
		* <pre><code>
		* var date = new Date();
		* var format = 'yyyy-MM-dd hh:mm:ss.fff';
		* var dateStr = date.toString(format); // dateStr的值为 '2008-12-10 10：32：23'
		* format = 'yyyy-MM-dd hh:mm:ss';
		* dateStr = date.toString(format); // dateStr的值为 '2008-12-10 10：32：23'
		* format = 'yyyy/MM/dd';
		* dateStr = date.toString(format); // dateStr的值为 '2008/12/10'
		* format = 'yyyy-MM-dd hh';
		* dateStr = date.toString(format); // dateStr的值为 '2008-12-10 10'
		* </code></pre>
		* @func date_to_string(date[,foramt])
		* @arg date {Date}
		* @arg [format] {String} 要转换的字符串格式
		* @ret {String} 返回格式化后的时间字符串
		*/
	toString(format?: string, timezone?: number): string;

}

interface ErrorDescribe {
	name?: string;
	message?: string;
	error?: string;
	description?: string;
	errno?: number;
	child?: Error | Error[];
	[prop: string]: any;
}

type ErrnoCode = [number/*errno*/, string/*message*/, string?/*description*/];
type ErrorNewArg = ErrnoCode | Error | string | ErrorDescribe;

interface ErrorConstructor {
	'new'(err: ErrorNewArg, ...child: ErrorNewArg[]): Error;
	toJSON(err: Error): any;
	setStackTraceJSON(enable: boolean): void;
	/** Create .stack property on a target object */
	captureStackTrace(targetObject: Object, constructorOpt?: Function): void;
}

interface Error {
	errno?: number;
	description?: string;
	child?: Error[];
	[prop: string]: any;
}