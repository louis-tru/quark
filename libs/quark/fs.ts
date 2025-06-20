/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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

import _util from './_util';
import {Encoding} from './buffer';

const _fs = __binding__('_fs');

/**
 * @enum FileOpenFlag
 * 
 * Opening file Flags
*/
export enum FileOpenFlag {
	FOPEN_ACCMODE = 0o3,
	/** 仅打开只读文件，文件必须存在 */
	FOPEN_RDONLY = 0o0,
	/** 仅打开只写文件，文件必须存在 */
	FOPEN_WRONLY = 0o1,
	/** 打开读写文件，文件必须存在 */
	FOPEN_RDWR = 0o2,
	/** 创建文件，如果文件不存在 */
	FOPEN_CREAT = 0o100,
	/** 创建文件时可执行 */
	FOPEN_EXCL = 0o200,
	/** No Controlling TTY */
	FOPEN_NOCTTY = 0o400,
	/** 清空文件内容，如果文件存在 */
	FOPEN_TRUNC = 0o1000,
	/** 将文件读取游标移动到结尾，如果文件存在 */
	FOPEN_APPEND = 0o2000,
	/** 阻塞模式, 通常使用open函数 */
	FOPEN_NONBLOCK = 0o4000,
	/** r 打开只读文件，该文件必须存在。 */
	FOPEN_R = FOPEN_RDONLY,
	/** w 打开只写文件，若文件存在则文件长度清为零，即该文件内容会消失，若文件不存在则建立该文件。 */
	FOPEN_W = FOPEN_WRONLY | FOPEN_CREAT | FOPEN_TRUNC,
	/**
	 * a 以附加的方式打开只写文件。若文件不存在，则会建立该文件，如果文件存在，
	 *   写入的数据会被加到文件尾，即文件原先的内容会被保留。
	 */
	FOPEN_A = FOPEN_WRONLY | FOPEN_CREAT | FOPEN_APPEND,
	/** r+ 打开可读写文件，该文件必须存在。 */
	FOPEN_RP = FOPEN_RDWR,
	/**
	 * w+ 打开可读写文件，若文件存在则文件长度清为零，即该文件内容会消失。
	 *    若文件不存在则建立该文件。
	 */
	FOPEN_WP = FOPEN_RDWR | FOPEN_CREAT | FOPEN_TRUNC,
	/**
	 * a+	以附加方式打开可读写的文件。若文件不存在，则会建立该文件，如果文件存在，
	 *    写入的数据会被加到文件尾后，即文件原先的内容会被保留。
	 */
	FOPEN_AP = FOPEN_RDWR | FOPEN_CREAT | FOPEN_APPEND,
}

/**
 * @enum FileType
 * 
 * File type
*/
export enum FileType {
	/** 未知类型 */
	FTYPE_UNKNOWN,
	/** 普通文件 (Regular File) */
	FTYPE_FILE,
	/** 目录 (Directory) */
	FTYPE_DIR,
	/** 符号链接文件（软链接） */
	FTYPE_LINK,
	/** 命名管道，一种特殊类型的文件，用于进程间通信 (IPC) */
	FTYPE_FIFO,
	/** 套接字 (Socket) */
	FTYPE_SOCKET,
	/** 无缓冲的字符设备，如：键盘鼠标(/dev/input/*)、终端 (/dev/tty*) */
	FTYPE_CHAR,
	/**
	 * 提供带缓冲的、面向块（固定大小数据块）的 I/O 访问，
	 * 如：硬盘驱动器 (/dev/sda, /dev/sda1, /dev/nvme0n1p1)
	 */
	FTYPE_BLOCK,
}

export declare const defaultMode: number;

/**
 * @interface Dirent
 * 
 * 调用 readdir/readdirSync 返回的结果
*/
export interface Dirent {
	/** 文件名称 */
	name: string;
	/** 文件的完整路径 */
	pathname: string;
	/** 文件类型 */
	type: FileType;
}

/**
 * @class FileState
*/
export declare class FileStat {
	isValid(): boolean;
	isFile(): boolean;
	isDir(): boolean;
	isLink(): boolean;
	isSock(): boolean;
	mode(): number;
	type(): FileType;
	group(): number;
	owner(): number;
	size(): number;
	nlink(): number;
	ino(): number;
	blksize(): number;
	blocks(): number;
	flags(): number;
	gen(): number;
	dev(): number;
	rdev(): number;
	atime(): number;
	mtime(): number;
	ctime(): number;
	birthtime(): number;
}

export interface Stream {
	pause(): void;
	resume(): void;
};

export interface StreamResponse {
	size: number;
	total: number;
	data: Uint8Array;
	end: boolean;
}

export class AsyncTask<T> extends Promise<T> {
	private _resolve: any;
	private _reject: any;
	private _complete: boolean;
	private _id: number;
	private _ok(r: T) {
		if (!this._complete) {
			this._complete = true;
			this._resolve(r);
		}
	}
	private _err(err: Error) {
		if (!this._complete) {
			this._complete = true;
			this._reject(err);
		}
	}
	constructor(exec: (resolve: any, reject: any)=>number) {
		let _resolve: any;
		let _reject: any;
		let self: this;
		let id: number = 0;
		super(function(resolve, reject) {
			_resolve = resolve;
			_reject = reject;
			id = exec((r: any)=>self._ok(r), (err: any)=>self._err(err));
		});
		self = this;
		this._resolve = _resolve;
		this._reject = _reject;
		this._complete = false;
		this._id = id;
	}
	get id() { return this._id }
	get complete() { return this._complete }

	abort(reason?: Error): void {
		_fs.abort(this._id);
		if (reason) {
			// ERR_READ_STREAM_ABORT: ErrnoCode = [-12001, 'ERR_READ_STREAM_ABORT']
			_util.nextTick(()=>this._err(reason));
		}
	}
}

// sync
export declare function chmodSync(path: string, mode?: number): void;
export declare function chownSync(path: string, owner: number, group: number): void;
export declare function mkdirSync(path: string, mode?: number): void;
export declare function mkdirsSync(path: string, mode?: number): void;
export declare function renameSync(name: string, newName: string): void;
export declare function linkSync(src: string, target: string): void;
export declare function unlinkSync(path: string): void;
export declare function rmdirSync(path: string): void;
export declare function readdirSync(path: string): Dirent[];
export declare function statSync(path: string): FileStat;
export declare function existsSync(path: string): boolean;
export declare function isFileSync(path: string): boolean;
export declare function isDirectorySync(path: string): boolean;
export declare function readableSync(path: string): boolean;
export declare function writableSync(path: string): boolean;
export declare function executableSync(path: string): boolean;
export declare function chmodRecursionSync(path: string, mode?: number): void;
export declare function chownRecursionSync(path: string, owner: number, group: number): void;
export declare function removeRecursionSync(path: string): void;
export declare function copyRecursionSync(path: string, target: string): void;
export declare function copySync(path: string, target: string): void;
// read/write file sync
export declare function writeFileSync(path: string, data: Uint8Array, size?: number): number;
export declare function writeFileSync(path: string, data: string, encoding?: Encoding): number;
export declare function readFileSync(path: string): Uint8Array;
export declare function readFileSync(path: string, encoding?: Encoding): string;
export declare function openSync(path: string, flags?: FileOpenFlag /*= FileOpenFlag.FOPEN_R*/): number;
export declare function closeSync(path: number): void;
export declare function readSync(fd: number, out: Uint8Array, size?: number /*= -1*/, offsetFd?: number /*= -1*/): number;
export declare function writeSync(fd: number, data: Uint8Array, size?: number /*= -1*/, offsetFd?: number /*= -1*/): number;
export declare function writeSync(fd: number, data: string, offsetFd?: number /*= -1*/): number;
export declare function writeSync(fd: number, data: string, encoding?: Encoding, offsetFd?: number /*= -1*/): number;

Object.assign(exports, {..._fs, ...exports});

// async
export function chmod(path: string, mode: number = _fs.defaultMode) {
	return new Promise<void>(function(resolve, reject) {
		_fs.chown(path, mode, (err?: Error)=>err?reject(err):resolve());
	});
}
export function chown(path: string, owner: number, group: number) {
	return new Promise<void>(function(resolve, reject) {
		_fs.chown(path, owner, group, (err?: Error)=>err?reject(err):resolve());
	});
}
export function mkdir(path: string, mode: number = _fs.defaultMode) {
	return new Promise<void>(function(resolve, reject) {
		_fs.mkdir(path, mode, (err?: Error)=>err?reject(err):resolve());
	});
}
export function mkdirs(path: string, mode: number = _fs.defaultMode) {
	return new Promise<void>(function(resolve, reject) {
		_fs.mkdirs(path, mode, (err?: Error)=>err?reject(err):resolve());
	});
}
export function rename(name: string, newName: string) {
	return new Promise<void>(function(resolve, reject) {
		_fs.rename(name, newName, (err?: Error)=>err?reject(err):resolve());
	});
}
export function link(src: string, target: string) {
	return new Promise<void>(function(resolve, reject) {
		_fs.link(src, target, (err?: Error)=>err?reject(err):resolve());
	});
}
export function unlink(path: string) {
	return new Promise<void>(function(resolve, reject) {
		_fs.unlink(path, (err?: Error)=>err?reject(err):resolve());
	});
}
export function rmdir(path: string) {
	return new Promise<void>(function(resolve, reject) {
		_fs.rmdir(path, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
	* @method readdir
	*
	* 读取目录列表信息，失败抛出异常,成功返回[`Dirent`]的[`Array`]
	*
	* @param path {string}
	* @return {Promise<Dirent[]>}
	*
	*	Example:
	*
	*	```js
	*	// Prints:
	*	// {
	*	//   name: "cp.txt",
	*	//   pathname: "file:///var/mobile/Containers/Data/Application/64DAC3FC-A4FD-4274-A2E7-B834EE4930B4/Documents/test/cp.txt",
	*	//   type: 1
	*	// }
	*	fs.readdir(mydir).then(dirents=>{
	*		for (var dirent of dirents) {
	*			// TODO...
	*			console.log(dirent);
	*		}
	*	}).catch(err=>{
	*		// Fail
	*	});
	*	```
*/
export function readdir(path: string) {
	return new Promise<Dirent[]>(function(resolve, reject) {
		_fs.readdir(path, (err?: Error, r?: Dirent[])=>err?reject(err):resolve(r as Dirent[]));
	});
}
export function stat(path: string) {
	return new Promise<FileStat>(function(resolve, reject) {
		_fs.stat(path, (err?: Error, r?: FileStat)=>err?reject(err):resolve(r as FileStat));
	});
}
export function exists(path: string) {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.exists(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}
export function isFile(path: string) {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.isFile(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}
export function isDirectory(path: string) {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.isDirectory(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}
export function readable(path: string) {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.readable(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}
export function writable(path: string) {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.writable(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}
export function executable(path: string) {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.executable(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}
export function chmodRecursion(path: string, mode: number = _fs.defaultMode) {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.chmodRecursion(path, mode, (err?: Error)=>err?reject(err):resolve());
	});
}
export function chownRecursion(path: string, owner: number, group: number) {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.chownRecursion(path, owner, group, (err?: Error)=>err?reject(err):resolve());
	});
}
export function removeRecursion(path: string) {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.removeRecursion(path, (err?: Error)=>err?reject(err):resolve());
	});
}
export function copyRecursion(path: string, target: string) {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.copyRecursion(path, target, (err?: Error)=>err?reject(err):resolve());
	});
}
export function copy(path: string, target: string) {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.copy(path, target, (err?: Error)=>err?reject(err):resolve());
	});
}

export function readStream(path: string, cb: (stream: StreamResponse)=>void): AsyncTask<void> {
	return new AsyncTask<void>(function(resolve, reject): number {
		return _fs.readStream(function(err?: Error, r?: StreamResponse) {
			if (err) {
				reject(err);
			} else {
				let stream = r!;
				cb(stream);
				if (stream.end) {
					resolve();
				}
			}
		}, path);
	});
}

export declare function abort(id: number): void;

// async
export declare function writeFile(path: string, data: Uint8Array, size?: number): Promise<number>;
export declare function writeFile(path: string, data: string, encoding?: Encoding): Promise<number>;
export declare function readFile(path: string): Promise<Uint8Array>;
export declare function readFile(path: string, encoding?: Encoding): Promise<string>;
export declare function open(path: string, flags?: FileOpenFlag): Promise<number>;
export declare function close(path: number): Promise<void>;
export declare function read(fd: number, out: Uint8Array, size?: number, offsetFd?: number): Promise<number>;
export declare function write(fd: number, data: Uint8Array, size?: number, offsetFd?: number): Promise<number>;
export declare function write(fd: number, data: string, offsetFd?: number): Promise<number>;
export declare function write(fd: number, data: string, encoding?: Encoding, offsetFd?: number): Promise<number>;

// reader
export interface Reader {
	readFile(path: string): AsyncTask<Uint8Array>;
	readFile(path: string, encoding: Encoding): AsyncTask<string>;
	readStream(path: string, cb: (stream: StreamResponse)=>void): AsyncTask<void>;
	readFileSync(path: string): Uint8Array;
	readFileSync(path: string, encoding: Encoding): string;
	existsSync(path: string): boolean;
	isFileSync(path: string): boolean;
	isDirectorySync(path: string): boolean;
	readdirSync(path: string): Dirent[];
	abort(id: number): void;
	clear(): void; // clear cache
}

export const reader: Reader = {
	..._fs.reader,
	readFile: function(...args: any[]) {
		return new AsyncTask<any>(function(resolve, reject) {
			return _fs.reader.readFile((err?: Error, r?: any)=>err?reject(err):resolve(r), ...args);
		});
	},
	readStream: function(path: string, cb: (stream: StreamResponse)=>void): AsyncTask<void> {
		return new AsyncTask<void>(function(resolve, reject): number {
			return _fs.reader.readStream(function(err?: Error, r?: StreamResponse) {
				if (err) {
					reject(err);
				} else {
					let stream = r!;
					cb(stream);
					if (stream.end) {
						resolve();
					}
				}
			}, path);
		});
	},
};

exports.writeFile = function(...args: any[]) {
	return new Promise<number>(function(resolve, reject) {
		_fs.writeFile((err?: Error, r?: number)=>err ? reject(err) : resolve(r as number), ...args);
	});
};

exports.readFile = function(...args: any[]) {
	return new Promise<any>(function(resolve, reject) {
		_fs.readFile((err?: Error, r?: any)=>err ? reject(err) : resolve(r), ...args);
	});
};

exports.open = function(...args: any[]) {
	return new Promise<any>(function(resolve, reject) {
		_fs.open((err?: Error, r?: any)=>err?reject(err):resolve(r), ...args);
	});
};

exports.close = function(...args: any[]) {
	return new Promise<void>(function(resolve, reject) {
		_fs.close((err?: Error)=>err?reject(err):resolve(), ...args);
	});
};

exports.read = function(...args: any[]) {
	return new Promise<any>(function(resolve, reject) {
		_fs.read((err?: Error, r?: any)=>err?reject(err):resolve(r), ...args);
	});
};

exports.write = function(...args: any[]) {
	return new Promise<any>(function(resolve, reject) {
		_fs.write((err?: Error, r?: any)=>err?reject(err):resolve(r), ...args);
	});
};