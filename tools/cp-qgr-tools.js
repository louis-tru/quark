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

var util = require('../libs/qkit/util');
var fs = require('../libs/qkit/fs');
var { copy_header } = require('./cp-header');
var path = require('path');

var args = process.argv.slice(2);
var root = path.resolve(__dirname, '..');
var target = args[0] ? path.resolve(args[0]) : root + '/out/qmake';
var include = target + '/product/include/qgr';

fs.rm_r_sync(include);
fs.rm_r_sync(target + '/product/libs');
fs.rm_r_sync(target + '/product/examples');

fs.cp_sync(root + '/libs/qmake', target);
fs.chmodSync(target + '/install', 0755);
// fs.chmodSync(target + '/bin/linux/jsa-shell', 0755);
fs.chmodSync(target + '/bin/osx/jsa-shell', 0755);
fs.chmodSync(target + '/gyp/gyp', 0755);

copy_header(root + '/qgr', include);

fs.cp_sync(root + '/demo/examples', target + '/product/examples');
fs.cp_sync(root + '/libs/qgr', target + '/product/libs/qgr');
fs.cp_sync(root + '/tools/product.gypi', target + '/product/qgr.gypi');

