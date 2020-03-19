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

import * as fs from 'nxkit/fs';
import * as path from 'path';

function resolve(name: string) {
	return path.resolve(__dirname, name);
}

export default {
	ngui_gyp: '',
	includes_gypi: [ resolve('product/ngui.gypi') ] as string[],
	default_modules: [ /*resolve('product/libs/ngui'),*/ ] as string[],
	examples: resolve('product/examples'),
	types: resolve('product/@types'),
	bundle_resources: [ /*resolve('product/cacert.pem')*/ ] as string[],
	includes: [ resolve('product/include') ] as string[],
	librarys: {
		ios: [
			resolve('product/ios'),
			'ios/Frameworks/iphoneos/Debug/nxkit.framework ../nxkit.framework',
			'ios/Frameworks/iphoneos/Debug/ngui.framework ../ngui.framework',
			'ios/Frameworks/iphoneos/Debug/ngui-media.framework ../ngui-media.framework',
			'ios/Frameworks/iphoneos/Release/nxkit.framework ../nxkit.framework',
			'ios/Frameworks/iphoneos/Release/ngui.framework ../ngui.framework',
			'ios/Frameworks/iphoneos/Release/ngui-js.framework ../ngui-js.framework',
			'ios/Frameworks/iphoneos/Release/ngui-v8.framework ../ngui-v8.framework',
			'ios/Frameworks/iphoneos/Release/ngui-node.framework ../ngui-node.framework',
			'ios/Frameworks/iphoneos/Release/ngui-media.framework ../ngui-media.framework',
			'ios/Frameworks/iphonesimulator/Debug/nxkit.framework ../nxkit.framework',
			'ios/Frameworks/iphonesimulator/Debug/ngui.framework ../ngui.framework',
			'ios/Frameworks/iphonesimulator/Debug/ngui-js.framework ../ngui-js.framework',
			'ios/Frameworks/iphonesimulator/Debug/ngui-v8.framework ../ngui-v8.framework',
			'ios/Frameworks/iphonesimulator/Debug/ngui-node.framework ../ngui-node.framework',
			'ios/Frameworks/iphonesimulator/Debug/ngui-media.framework ../ngui-media.framework',
			'ios/Frameworks/iphonesimulator/Release/nxkit.framework ../nxkit.framework',
			'ios/Frameworks/iphonesimulator/Release/ngui.framework ../ngui.framework',
			'ios/Frameworks/iphonesimulator/Release/ngui-js.framework ../ngui-js.framework',
			'ios/Frameworks/iphonesimulator/Release/ngui-v8.framework ../ngui-v8.framework',
			'ios/Frameworks/iphonesimulator/Release/ngui-node.framework ../ngui-node.framework',
			'ios/Frameworks/iphonesimulator/Release/ngui-media.framework ../ngui-media.framework',
		],
		android: [ resolve('product/android') ],
	} as Dict<string[]>,
};

if ( !fs.existsSync(resolve('product/ngui.gypi')) ) { // debug status
	exports.default = { 
		ngui_gyp: __dirname + '/../../ngui.gyp',
		includes_gypi: [
			__dirname + '/../../out/config.gypi',
			__dirname + '/../../tools/common.gypi',
		],
		default_modules: [],
		examples: __dirname + '/../../examples',
		types: __dirname + '/../../libs/ngui/out/@types',
		bundle_resources: [ /*__dirname + '/../../nguiutil/cacert.pem'*/ ],
		includes: [],
		librarys: {},
	};
}