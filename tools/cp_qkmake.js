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

var fs = require('encark/fs');
var { execSync } = require('encark/syscall');
var { copy_header } = require('./cp_header');
var path = require('path');
var read_version = require('./read_version');

var args = process.argv.slice(2);
var root = path.resolve(__dirname, '..');
var target = args[0] ? path.resolve(args[0]) : root + '/out/qkmake';
var include = target + '/product/include';

fs.rm_r_sync(include);
fs.rm_r_sync(target + '/product/libs');
fs.rm_r_sync(target + '/product/examples');
fs.rm_r_sync(target + '/product/quark');

read_version.update_quark_version();

// build quark
execSync(`cd ${root}/libs/quark && npm run build`);
fs.cp_sync(root + '/libs/quark/out/@types', target + '/product/@types');

// build qkmake
execSync(`cd ${root}/libs/qkmake && npm run build`);
fs.cp_sync(root + '/libs/qkmake/out/qkmake', target, {ignore_hide:0,symlink: 0});
fs.cp_sync(root + '/libs/qkmake/gyp', target + '/gyp', {ignore_hide:1,replace:0});

fs.chmodSync(target + '/gyp/gyp', 0o755);
fs.chmodSync(target + '/shell.js', 0o755);

copy_header(root + '/quark', `${include}/quark`);
copy_header(root + '/quark-js', `${include}/quark-js`);
// copy_header(`${root}/deps/v8-link/include`, include);
// copy_header(`${root}/deps/node/deps/openssl/openssl/include/openssl`, `${include}/openssl`);
// copy_header(`${root}/deps/node/deps/openssl/config`, `${include}/openssl`);
// copy_header(`${root}/deps/node/deps/uv/include`, include);
// copy_header(`${root}/deps/node/deps/zlib/zlib.h`, `${include}/zlib.h`);
// copy_header(`${root}/deps/node/deps/zlib/zconf.h`, `${include}/zconf.h`);
// copy_header(`${root}/deps/node/src/node_api.h`, `${include}/node_api.h`);
// copy_header(`${root}/deps/node/src/node_api_types.h`, `${include}/node_api_types.h`);
// copy_header(`${root}/deps/node/src/node_buffer.h`, `${include}/node_buffer.h`);
// copy_header(`${root}/deps/node/src/node.h`, `${include}/node.h`);
// copy_header(`${root}/deps/node/src/node_object_wrap.h`, `${include}/node_object_wrap.h`);
// copy_header(`${root}/deps/node/src/node_version.h`, `${include}/node_version.h`);

// fs.cp_sync(root + '/libs/quark', target + '/product/libs/quark');
fs.cp_sync(root + '/examples', target + '/product/examples');
fs.cp_sync(root + '/tools/product.gypi', target + '/product/quark.gypi');
// fs.cp_sync(root + '/tools/common.gypi', target + '/product/common.gypi');
// fs.cp_sync(root + '/out/config.gypi', target + '/product/config.gypi');
