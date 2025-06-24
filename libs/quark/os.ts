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

const _os = __binding__('_os');

/**
 * @enum NetworkInterface
*/
export enum NetworkInterface {
	kNone, /// None
	kETH, /// Ethernet
	kWifi, // Wifi
	kMobile, /// Mobile
	k2G, /// Mobile 2G
	k3G, /// Mobile 3G
	k4G, /// Mobile 4G
	k5G, /// Mobile 5G
}

/**
 * @method arch() Get CPU arch type
 * @return {string}
*/
export declare function arch(): string;

/**
 * @method name() Get OS name, For: `Android`/`iOS`/`MacOSX`/`Linux`
 * @return {string}
*/
export declare function name(): string;

/**
 * @method version() Get OS version info
 * @return {string}
*/
export declare function version(): string;

/**
 * @method brand() Get the Device brand, For: Apple/Huawei
 * @return {string}
*/
export declare function brand(): string;

/**
 * @method model() Get OS model, iPad‌/iPhone‌/Mac16,10
 * @return {string}
*/
export declare function model(): string;

/**
 * @method info() Get OS information
 * @return {string}
 * 
 * Example:
 * 
 * ```ts
 * // Prints:
 * // sysname: Louis-iPhone
 * // sysname: Darwin
 * // machine: iPhone7,2
 * // nodename: Louis-iPhone
 * // version: Darwin Kernel Version 16.6.0: Mon Apr 17 17:33:35 PDT 2017; root:xnu-3789.60.24~24/RELEASE_ARM64_T7000
 * // release: 16.6.0
 * console.log(os.info());
 * ```
*/
export declare function info(): string;

/**
 * @method languages() Get List of OS supported languages, May value is `'en-us'`|`'zh-cn'`|`'zh-tw'`
 * @return {string[]}
*/
export declare function languages(): string[];

/**
 * @method isWifi() Is it a wifi network type?
 * @return {bool}
*/
export declare function isWifi(): boolean;

/**
 * @method isMobile() Is it a mobile network type?
 * @return {bool}
*/
export declare function isMobile(): boolean;

/**
 * @method networkInterface() Get the network interface
 * @return {NetworkInterface}
*/
export declare function networkInterface(): NetworkInterface;

/**
 * @method isAcPower() Is there an external power supply connected?
 * @return {bool}
*/
export declare function isAcPower(): boolean;

/**
 * @method isBattery() Is there a battery device?
 * @return {bool}
*/
export declare function isBattery(): boolean;

/**
 * @method batteryLevel() Get battery power percentage if have a battery
 * @return {float} range 0 to 1
*/
export declare function batteryLevel(): number;

/**
 * @method memory() Get the memory total
 * @return {uint}
*/
export declare function memory(): number;

/**
 * @method memory() Get the used memory size
 * @return {uint}
*/
export declare function usedMemory(): number;

/**
 * @method memory() Get the available memory size
 * @return {uint}
*/
export declare function availableMemory(): number;

/**
 * @method memory() Get CPU usage percentage
 * @return {float}
*/
export declare function cpuUsage(): number;

Object.assign(exports, _os);