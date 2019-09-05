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

var url = require('lkit/url');
var fs = require('lkit/fs');
var path = require('path');

function resolve(name) {
	return path.resolve(__dirname + '/' + name);
}

if ( fs.existsSync(resolve('product/langou.gypi')) ) {
	module.exports = {
		langou_gyp: '',
		includes_gypi: [ resolve('product/langou.gypi') ],
		default_modules: [ /*resolve('product/libs/langou'),*/ ],
		examples: resolve('product/examples'),
		bundle_resources: [ /*resolve('product/cacert.pem')*/ ],
		includes: [ resolve('product/include') ],
		librarys: {
			ios: [
				resolve('product/ios'),
				'ios/Frameworks/iphoneos/Debug/lutils.framework ../lutils.framework',
				'ios/Frameworks/iphoneos/Debug/langou.framework ../langou.framework',
				'ios/Frameworks/iphoneos/Debug/langou-media.framework ../langou-media.framework',
				'ios/Frameworks/iphoneos/Release/lutils.framework ../lutils.framework',
				'ios/Frameworks/iphoneos/Release/langou.framework ../langou.framework',
				'ios/Frameworks/iphoneos/Release/langou-js.framework ../langou-js.framework',
				'ios/Frameworks/iphoneos/Release/langou-v8.framework ../langou-v8.framework',
				'ios/Frameworks/iphoneos/Release/langou-node.framework ../langou-node.framework',
				'ios/Frameworks/iphoneos/Release/langou-media.framework ../langou-media.framework',
				'ios/Frameworks/iphonesimulator/Debug/lutils.framework ../lutils.framework',
				'ios/Frameworks/iphonesimulator/Debug/langou.framework ../langou.framework',
				'ios/Frameworks/iphonesimulator/Debug/langou-js.framework ../langou-js.framework',
				'ios/Frameworks/iphonesimulator/Debug/langou-v8.framework ../langou-v8.framework',
				'ios/Frameworks/iphonesimulator/Debug/langou-node.framework ../langou-node.framework',
				'ios/Frameworks/iphonesimulator/Debug/langou-media.framework ../langou-media.framework',
				'ios/Frameworks/iphonesimulator/Release/lutils.framework ../lutils.framework',
				'ios/Frameworks/iphonesimulator/Release/langou.framework ../langou.framework',
				'ios/Frameworks/iphonesimulator/Release/langou-js.framework ../langou-js.framework',
				'ios/Frameworks/iphonesimulator/Release/langou-v8.framework ../langou-v8.framework',
				'ios/Frameworks/iphonesimulator/Release/langou-node.framework ../langou-node.framework',
				'ios/Frameworks/iphonesimulator/Release/langou-media.framework ../langou-media.framework',
			],
			android: [ resolve('product/android') ],
		},
	};
} else { // debug
	module.exports = {
		langou_gyp: __dirname + '/../../langou.gyp',
		includes_gypi: [
			__dirname + '/../../out/config.gypi',
			__dirname + '/../../tools/common.gypi',
		],
		default_modules: [],
		examples: __dirname + '/../../examples',
		bundle_resources: [ /*__dirname + '/../../langouutil/cacert.pem'*/ ],
		includes: [],
		librarys: {},
	};
}
