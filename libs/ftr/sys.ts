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

const _sys = __require__('_sys');

export enum NetworkInterface {
	NETWORK_NONE,
	NETWORK_ETH,
	NETWORK_WIFI,
	NETWORK_MOBILE, // mobile
	NETWORK_2G, // 2G
	NETWORK_3G, // 3G
	NETWORK_4G, // 4G
	NETWORK_5G, // 5G
}

export interface NetworkStatus {
	interface: NetworkInterface;
}

export declare function time(): number;
export declare function timeMonotonic(): number;
export declare function name(): string;
export declare function info(): string;
export declare function version(): string;
export declare function brand(): string;
export declare function subsystem(): string;
export declare function language(): string;
export declare function isWifi(): boolean;
export declare function isMobile(): boolean;
export declare function isACPower(): boolean;
export declare function isBattery(): boolean;
export declare function batteryLevel(): number;
export declare function memory(): number;
export declare function usedMemory(): number;
export declare function availableMemory(): number;
export declare function cpuUsage(): number;

export function networkStatus(): NetworkStatus {
	return {
		interface: _sys.networkInterface() as NetworkInterface,
	}
}

Object.assign(exports, _sys);