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
	kNone, //!< None
	kETH, //!< Ethernet
	kWifi, // Wifi
	kMobile, //!< Mobile
	k2G, //!< Mobile 2G
	k3G, //!< Mobile 3G
	k4G, //!< Mobile 4G
	k5G, //!< Mobile 5G
}

/**
 * Get CPU architecture type of the current device.
 *
 * Possible values:
 *   - "arm64"   64-bit ARM architecture
 *   - "x64"     64-bit x86 architecture
 *   - "arm"     32-bit ARM architecture
 *   - "x86"     32-bit x86 architecture
 *   - "mips"    32-bit MIPS architecture
 *   - "mips64"  64-bit MIPS architecture
 *
 * Notes:
 * - The returned value reflects the runtime CPU architecture,
 *   not the ABI or instruction set availability.
 * - On emulators or compatibility layers, the value may differ
 *   from the physical hardware.
 */
export declare function arch(): | 'arm64' | 'x64' | 'arm' | 'x86' | 'mips' | 'mips64';

/**
 * Get operating system name.
 *
 * Possible values:
 *   - "Android"
 *   - "iOS"
 *   - "MacOSX"
 *   - "Linux"
 *
 * Notes:
 * - The returned value represents the operating system family,
 *   not a specific distribution or version.
 * - For Apple platforms, iOS and macOS are reported separately.
 */
export declare function name(): | 'Android' | 'iOS' | 'MacOSX' | 'Linux' /* | 'Windows' */;

/**
 * Get operating system version information.
 *
 * Platform-specific behavior:
 *
 * - iOS / macOS:
 *   Returns the system version string.
 *   Example:
 *     - "17.1.2"
 *     - "14.2"
 *
 * - Android:
 *   Returns the Android OS version.
 *   Example:
 *     - "14"
 *     - "13"
 *
 * - Linux:
 *   Returns the kernel release version.
 *   Example:
 *     - "6.6.12-arch1-1"
 *     - "5.15.0-89-generic"
 *
 * Notes:
 * - The format of the returned string is platform-dependent.
 * - This value is intended for informational and compatibility
 *   checks, not strict semantic version comparison.
 */
export declare function version(): string;

/**
 * Get device or operating system brand.
 *
 * Platform-specific behavior:
 *
 * - Apple platforms (iOS / macOS):
 *   Returns "Apple".
 *
 * - Android:
 *   Returns the device manufacturer.
 *   Examples:
 *     - "google"
 *     - "samsung"
 *     - "Xiaomi"
 *     - "HUAWEI"
 *     - "OPPO"
 *     - "vivo"
 *     - "OnePlus"
 *
 * - Linux:
 *   Returns the distribution name if available.
 *   Examples:
 *     - "Ubuntu"
 *     - "Arch Linux"
 *     - "Debian GNU/Linux"
 *
 * Notes:
 * - On some platforms or environments, this value may be empty
 *   or unavailable.
 * - The returned string should be treated as informational only.
 */
export declare function brand(): string;

/**
 * Get device model / platform model identifier.
 *
 * Platform-specific behavior:
 *
 * - iOS:
 *   Returns device family name.
 *   Possible values:
 *     - "iPhone"
 *     - "iPad"
 *     - "iPod touch"
 *
 * - macOS:
 *   Returns Apple hardware model identifier.
 *   Example:
 *     - "Mac16,10"
 *
 * - Android:
 *   Returns OEM device model string (`Build.MODEL`).
 *   Examples:
 *     - "Pixel 7"
 *     - "Pixel 8 Pro"
 *     - "SM-S9110"
 *     - "2304FPN6DC"
 *     - "ALN-AL00"
 *     - "sdk_gphone64_arm64" (emulator)
 *
 * - Linux:
 *   No standard device model exists.
 *   Returns an empty string.
 *
 * Notes:
 * - The returned value is platform-dependent and not guaranteed
 *   to be comparable across different operating systems.
 * - This API is intended for identification, logging, and
 *   platform-specific behavior selection, not for strict
 *   device classification.
 */
export declare function model(): 'iPhone' | 'iPad' | 'iPod touch' /*| 'Apple TV'*/ | `Mac${string},${number}` | string;

/**
 * Get OS information
 * 
 * @example
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
 * Get List of OS supported languages, May value is `'en-us'`|`'zh-cn'`|`'zh-tw'`
*/
export declare function languages(): string[];

/**
 * Is it a wifi network type?
*/
export declare function isWifi(): boolean;

/**
 * Is it a mobile network type?
*/
export declare function isMobile(): boolean;

/**
 * Get the network interface
*/
export declare function networkInterface(): NetworkInterface;

/**
 * Is there an external power supply connected?
*/
export declare function isAcPower(): boolean;

/**
 * Is there a battery device?
*/
export declare function isBattery(): boolean;

/**
 * Get battery power percentage if have a battery
 * @return range 0 to 1
*/
export declare function batteryLevel(): number;

/**
 * Get the memory total
*/
export declare function memory(): number;

/**
 * Get the used memory size
*/
export declare function usedMemory(): number;

/**
 * Get the available memory size
*/
export declare function availableMemory(): number;

/**
 * Get CPU usage percentage
*/
export declare function cpuUsage(): number;

Object.assign(exports, _os);