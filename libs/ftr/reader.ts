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

import { Encoding } from './buffer';
import { AsyncTask, StreamData, Dirent } from './fs';

const _reader = __require__('_reader');

export declare function readFile(path: string): AsyncTask<Uint8Array>;
export declare function readFile(path: string, encoding: Encoding): AsyncTask<string>;
export declare function readFileSync(path: string): Uint8Array;
export declare function readFileSync(path: string, encoding: Encoding): string;
export declare function existsSync(path: string): boolean;
export declare function isFileSync(path: string): boolean;
export declare function isDirectorySync(path: string): boolean;
export declare function readdirSync(path: string): Dirent[];
export declare function abort(id: number): void;
export declare function clear(): void; // clear cache

Object.assign(exports, _reader);

export function readStream(path: string, cb: (stream: StreamData)=>void): AsyncTask<void> {
	return new AsyncTask<void>(function(resolve, reject): number {
		return _reader.readStream(function(err?: Error, r?: StreamData) {
			if (err) {
				reject(err);
			} else {
				var stream = r as StreamData;
				cb(stream);
				if (stream.complete) {
					resolve();
				}
			}
		}, path);
	});
}

exports.readFile = function(...args: any[]) {
	return new AsyncTask<any>(function(resolve, reject) {
		return _reader.readFile((err?: Error, r?: any)=>err?reject(err):resolve(r), ...args);
	});
};

exports.readFileSync = function(...args: any[]) {
	return _reader.readFileSync(...args);
};