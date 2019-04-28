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

var url = require('qkit/url');
var fs = require('qkit/fs');
var path = require('path');

function resolve(name) {
	return path.resolve(__dirname + '/' + name);
}

if ( fs.existsSync(resolve('product/qgr.gypi')) ) {
	module.exports = {
		qgr_gyp: '',
		includes_gypi: [ resolve('product/qgr.gypi') ],
		default_modules: [ /*resolve('product/libs/qgr'),*/ ],
		examples: resolve('product/examples'),
		bundle_resources: [ /*resolve('product/cacert.pem')*/ ],
		includes: [ resolve('product/include') ],
		librarys: {
			ios: [
				resolve('product/ios'),
				'ios/Frameworks/iphoneos/Debug/qgr-media.framework ../qgr-media.framework',
				'ios/Frameworks/iphoneos/Debug/qgr.framework ../qgr.framework',
				'ios/Frameworks/iphoneos/Release/qgr.framework ../qgr.framework',
				'ios/Frameworks/iphoneos/Release/qgr-js.framework ../qgr-js.framework',
				'ios/Frameworks/iphoneos/Release/qgr-v8.framework ../qgr-v8.framework',
				'ios/Frameworks/iphoneos/Release/qgr-node.framework ../qgr-node.framework',
				'ios/Frameworks/iphoneos/Release/qgr-media.framework ../qgr-media.framework',
				'ios/Frameworks/iphonesimulator/Debug/qgr.framework ../qgr.framework',
				'ios/Frameworks/iphonesimulator/Debug/qgr-js.framework ../qgr-js.framework',
				'ios/Frameworks/iphonesimulator/Debug/qgr-v8.framework ../qgr-v8.framework',
				'ios/Frameworks/iphonesimulator/Debug/qgr-node.framework ../qgr-node.framework',
				'ios/Frameworks/iphonesimulator/Debug/qgr-media.framework ../qgr-media.framework',
				'ios/Frameworks/iphonesimulator/Release/qgr.framework ../qgr.framework',
				'ios/Frameworks/iphonesimulator/Release/qgr-js.framework ../qgr-js.framework',
				'ios/Frameworks/iphonesimulator/Release/qgr-v8.framework ../qgr-v8.framework',
				'ios/Frameworks/iphonesimulator/Release/qgr-node.framework ../qgr-node.framework',
				'ios/Frameworks/iphonesimulator/Release/qgr-media.framework ../qgr-media.framework',
			],
			android: [ resolve('product/android') ],
		},
	};
} else { // debug
	module.exports = {
		qgr_gyp: __dirname + '/../../qgr.gyp',
		includes_gypi: [
			__dirname + '/../../out/config.gypi',
			__dirname + '/../../tools/common.gypi',
		],
		default_modules: [],
		examples: __dirname + '/../../examples',
		bundle_resources: [ /*__dirname + '/../../qgrutil/cacert.pem'*/ ],
		includes: [],
		librarys: {},
	};
}
