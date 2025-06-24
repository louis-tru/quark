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

/**
 * @const defaultMode
 * 
 * 创建与设置文件的默认`mode`值,这与文件的权限相关,这是一个`int`整数类型值
*/
export declare const defaultMode: number;

/**
 * @callback StreamResponseCallback(stream)void
 * @param stream {StreamResponse}
*/
export type StreamResponseCallback = (stream: StreamResponse)=>void;

/**
 * @class Dirent
 * 
 * 调用 readdir/readdirSync 返回的结果
 * 
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
	/** 文件是否有效 */
	isValid(): boolean;
	/** 是否为普通文件 */
	isFile(): boolean;
	/** 是否为目录 */
	isDir(): boolean;
	/** 是否为符号链接 */
	isLink(): boolean;
	/** 是否为套接字 (Socket) */
	isSock(): boolean;
	/** 文件权限掩码 */
	mode(): number;
	/** 文件类型 */
	type(): FileType;
	/** 用户组ID */
	group(): number;
	/** 用户ID */
	owner(): number;
	/** 文件大小 */
	size(): number;
	/** 文件硬链接数量 */
	nlink(): number;
	/** 文件系统特定的文件“Inode”编号 */
	ino(): number;
	/** 用于 i/o 操作的文件系统块大小 */
	blksize(): number;
	/** 为此文件分配的块数 */
	blocks(): number;
	/** flags */
	flags(): number;
	/** gen */
	gen(): number;
	/** 包括该文件设备ID */
	dev(): number;
	/** 如果文件代表设备，则为数字设备标识符 */
	rdev(): number;
	/** 指示上次访问此文件的时间戳 */
	atime(): number;
	/** 指示此文件上次修改时间的时间戳 */
	mtime(): number;
	/** 指示文件状态上次更改的时间戳 */
	ctime(): number;
	/** 指示此文件创建时间的时间戳 */
	birthtime(): number;
}

/**
 * @class Stream
*/
export interface Stream {
	/** 暂停流读取 */
	pause(): void;
	/** 恢复流读取 */
	resume(): void;
};

/**
 * @class StreamResponse
*/
export interface StreamResponse {
	/** 数据大小 */
	size: number;
	/** 数据总大小 */
	total: number;
	/** 数据 */
	data: Uint8Array;
	/** 是否已经结束 */
	ended: boolean;
}

/**
 * @class AsyncTask
 * @extends Promise
*/
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
	
	/** 异步任务id可通过abort(id)中止运行 */
	get id() { return this._id }
	/** 异步任务是否完成 */
	get complete() { return this._complete }

	/**
	 * 中止任务
	 * @method abort(reason)
	 * @param reason? {Error} 如果传入参数会抛出异常
	*/
	abort(reason?: Error): void {
		_fs.abort(this._id);
		if (reason) {
			// ERR_READ_STREAM_ABORT: ErrnoCode = [-12001, 'ERR_READ_STREAM_ABORT']
			_util.nextTick(()=>this._err(reason));
		}
	}
}

// sync
/**
 * @method chmodSync(path[,mode])
 * 
 * 同步更改文件的权限
 * 
 * @param path {string}
 * @param mode? {int}
 * 
 * ```ts
 * chmodSync('my_file.txt', 0o775)
 * ```
*/
export declare function chmodSync(path: string, mode?: number): void;

/**
 * @method chownSync(path,owner,group)
 * 
 * 同步设置文件所属用户`owner`与组`group`
 * 
 * @param path {string}
 * @param owner {number}
 * @param group {number}
*/
export declare function chownSync(path: string, owner: number, group: number): void;

/**
 * @method mkdirSync(path[,mode])
 * 
 * 同步创建目录, 如果路径已存在会抛出异常
 * 
 * @param path {string}
 * @param mode? {int} **[`defaultMode`]**
*/
export declare function mkdirSync(path: string, mode?: number): void;

/**
 * @method mkdirsSync(path[,mode])
 * 
 * 同步递归创建目录，目录存在时不会抛出异常
 * 
 * @param path string
 * @param mode? int **[`defaultMode`]**
 */
export declare function mkdirsSync(path: string, mode?: number): void;

/**
 * 同步重命名文件与目录名称
 * 
 * @mehod renameSync(name,newName)
 * @param name {string}
 * @param newName {string}
*/
export declare function renameSync(name: string, newName: string): void;

/**
 * 同步创建文件硬链接
 * 
 * @method linkSync(src,target)
 * @param src {string} 文件原始路径
 * @param target {string} 链接目标路径
*/
export declare function linkSync(src: string, target: string): void;

/**
 * 同步删除文件硬链接，如果文件只有一个链接文件将会被物理删除
 * 
 * @method unlinkSync(path)
 * @param path {string}
*/
export declare function unlinkSync(path: string): void;

/**
 * 同步删除文件目录，文件目录必须为空
 * 
 * @method rmdirSync(path)
 * @param path {string}
*/
export declare function rmdirSync(path: string): void;

/**
 * 同步读取文件目录文件列表
 * 
 * @method readdirSync(path)
 * @param path {string}
 * @return {Dirent[]}
*/
export declare function readdirSync(path: string): Dirent[];

/**
 * @method statSync(path)
 * 
 * 同步读取文件状态信息
 * 
 * @return {FileState}
*/
export declare function statSync(path: string): FileStat;

/**
 * 同步检查文件是否存在
 * @method existsSync(path)
 * @return {bool}
*/
export declare function existsSync(path: string): boolean;

/**
 * 同步检查是否为文件路径，如果路径不存在返回`false`
 * @method isFileSync(path)
 * @param path {string}
 * @return {bool}
*/
export declare function isFileSync(path: string): boolean;

/**
 * 同步检查是否为目录路径，如果路径不存在返回`false`
 * @method isDirectorySync(path)
 * @param path {string}
 * @return {bool}
*/
export declare function isDirectorySync(path: string): boolean;

/**
 * 同步检查是否为可读，如果路径不存在返回`false`
 * @method readableSync(path)
 * @param path {string}
 * @return {bool}
*/
export declare function readableSync(path: string): boolean;

/**
 * 同步检查是否为可写，如果路径不存在返回`false`
 * @method writableSync(path)
 * @param path {string}
 * @return {bool}
*/
export declare function writableSync(path: string): boolean;

/**
 * 同步检查是否为可执行，如果路径不存在返回`false`
 * @method executableSync(path)
 * @param path {string}
 * @return {bool}
*/
export declare function executableSync(path: string): boolean;

/**
 * 
 * @mehod chmodRecursionSync(path[,mode])
 * 
 * 同步递归设置文件的权限属性mode（TODO: 会柱塞调用线程，请谨慎使用）
 * 
 * @param path {string}
 * @param mode? {int} **[`defaultMode`]**
 * 
 * Example:
 * 
 * ```ts
 * fs.chmodRecursionSync(mypath, 0755);
 * ```
*/
export declare function chmodRecursionSync(path: string, mode?: number): void;

/**
 * @method chownRecursionSync(path,owner,group)
 * 
 * 同步递归设置文件owner与group属性（TODO: 会柱塞调用线程，请谨慎使用）
 * 
 * @param path string
 * @param owner int 操作系统用户id
 * @param group int 操作系统组id
 */
export declare function chownRecursionSync(path: string, owner: number, group: number): void;

/**
 * 
 * @method removeRecursionSync(path)
 * 
 * 同步递归删除目录或文件,在javascript中谨慎使用这个方法,有可能会造成线程长时间被柱塞
 * 
 * @param path {string}
 */
export declare function removeRecursionSync(path: string): void;

/**
 * @method copyRecursionSync(path,target)
 * 
 * 同步递归拷贝文件,在javascript中谨慎使用这个方法,有可能会造成线程长时间被柱塞
 *
 * @param path strings
 * @param target string
 * 
 */
export declare function copyRecursionSync(path: string, target: string): void;

/**
 * @method copySync
 * 
 * 同步拷贝文件（TODO: 会柱塞调用线程，请谨慎使用）
 * 
 * 同：拷贝文件与目录[`copyRecursionSync(path，target)`]
 * 
 * @param path string
 * @param target string
*/
export declare function copySync(path: string, target: string): void;

// read/write file sync
/**
 * @method writeFileSync(path,data[,size])
 * 
 * 同步写入数据到文件
 * 
 * @param path {string}
 * @param data {Uint8Array}
 * @param size? {uint} 不传入参数时写入全部数据
*/
export declare function writeFileSync(path: string, data: Uint8Array, size?: number): number;

/**
 * @method writeFileSync(path,data[,encoding])
 * 
 * 同步写入字符串到文件
 * 
 * @param path {string}
 * @param data {string}
 * @param encoding? {Encoding} 如果不传入默认使用`utf-8`编码字符串
*/
export declare function writeFileSync(path: string, data: string, encoding?: Encoding): number;

/**
 * @method readFileSync(path)
 * 
 * 同步读取文件
 * 
 * @param path {string}
 * @return {Uint8Array}
*/
export declare function readFileSync(path: string): Uint8Array;

/**
 * @method readFileSync(path,encoding)
 * 
 * 同步读取文件为字符串
 * 
 * @param path {string}
 * @param encoding {Encoding} 解码数据到字符串的编码
 * @return {string}
*/
export declare function readFileSync(path: string, encoding: Encoding): string;

/**
 * @method openSync(path[,flags])
 * 
 * 同步方式通过路径打开文件，并返回打开的文件句柄
 * 
 * @param path {string}
 * @param flags? {FileOpenFlag} 打开文件标志掩码
 * @return {uint}
*/
export declare function openSync(path: string, flags?: FileOpenFlag /*= FileOpenFlag.FOPEN_R*/): number;

/**
 * @method closeSync(fd)
 * 
 * 同步方式关闭文件句柄
*/
export declare function closeSync(fd: number): void;

/**
 * @method readSync(fd,out[,size[,offsetFd]])
 * 
 * 同步读取文件内容从文件句柄
 * 
 * @param fd {uint} 打开的文件句柄
 * @param out {Uint8Array} 读取文件并保存到这里
 * @param size? {int} 不传入或传入`-1`时使用`out`参数的长度
 * @param offsetFd? {int} 不传入或传入`-1`时使用文件句柄内部偏移值（每次读取后将向前进）
 * @return {uint} 返回数据实际读取的大小
*/
export declare function readSync(fd: number, out: Uint8Array, size?: number /*= -1*/, offsetFd?: number /*= -1*/): number;

/**
 * @method writeSync(fd,data[,size[,offsetFd]])
 * 
 * 同步写入数据到文件句柄
 * 
 * @param fd {uint} 打开的文件句柄
 * @param data {Uint8Array} 要写入的数据
 * @param size? {int} 不传入或传入`-1`时写入全部数据
 * @param offsetFd? {int} 不传入或传入`-1`时使用文件句柄内部偏移值（每次写入后将向前进）
 * @return {uint} 写入数据的实际大小
*/
export declare function writeSync(fd: number, data: Uint8Array, size?: number /*= -1*/, offsetFd?: number /*= -1*/): number;

/**
 * @method writeSync(fd,data[,offsetFd])
 * 
 * 同步写入数据到文件句柄,并且使用`utf-8`对数据进行编码
 * 
 * @param fd {uint} 打开的文件句柄
 * @param data {string} 要写入的字符串
 * @param offsetFd? {int} 不传入或传入`-1`时使用文件句柄内部偏移值（每次写入后将向前进）
 * @return {uint} 写入数据的实际大小
*/
export declare function writeSync(fd: number, data: string, offsetFd?: number /*= -1*/): number;

/**
 * @method writeSync(fd,data,encoding[,offsetFd])
 * 
 * 同步写入数据到文件句柄
 * 
 * @param fd {uint} 打开的文件句柄
 * @param data {string} 要写入的数据
 * @param encoding {Encoding} 编码类型
 * @param offsetFd? {int} 不传入或传入`-1`时使用文件句柄内部偏移值（每次写入后将向前进）
 * @return {uint} 写入数据的实际大小
*/
export declare function writeSync(fd: number, data: string, encoding: Encoding, offsetFd?: number /*= -1*/): number;

Object.assign(exports, {..._fs, ...exports});

// async
/**
 * Please ref: sync method [`chmodSync(path[,mode])`]
*/
export function chmod(path: string, mode: number = _fs.defaultMode) {
	return new Promise<void>(function(resolve, reject) {
		_fs.chown(path, mode, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * Please ref: sync method [`chownSync(path,owner,group)`]
*/
export function chown(path: string, owner: number, group: number) {
	return new Promise<void>(function(resolve, reject) {
		_fs.chown(path, owner, group, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * Please ref: sync method [`mkdirSync(path[,mode])`]
*/
export function mkdir(path: string, mode: number = _fs.defaultMode) {
	return new Promise<void>(function(resolve, reject) {
		_fs.mkdir(path, mode, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * @method mkdirs(path[,mode])
 * 
 * 递归创建目录，这个方法会依次创建目录树,目录存在也不会抛出异常
 * 
 * Ref: sync method [`mkdirsSync(path[,mode])`]
 * 
 * @arg path string
 * @arg mode? {int} **[`defaultMode`]**
 * @return {Promise}
 * 
 * Example:
 * ```ts
 * fs.mkdirs(mypath).then(()=>{
 * 	// Success
 * }).catch(err=>{
 * 	// Fail
 * });
 * ```
 */
export function mkdirs(path: string, mode: number = _fs.defaultMode) {
	return new Promise<void>(function(resolve, reject) {
		_fs.mkdirs(path, mode, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * Please ref: sync method [`renameSync(name,newName)`]
*/
export function rename(name: string, newName: string) {
	return new Promise<void>(function(resolve, reject) {
		_fs.rename(name, newName, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * Please ref: sync method [`linkSync(src,target)`]
*/
export function link(src: string, target: string) {
	return new Promise<void>(function(resolve, reject) {
		_fs.link(src, target, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * Please ref: sync method [`unlinkSync(path)`]
*/
export function unlink(path: string) {
	return new Promise<void>(function(resolve, reject) {
		_fs.unlink(path, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * Please ref: sync method [`rmdirSync(path)`]
*/
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
	*	```ts
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

/**
 * Please ref: sync method [`statSync(path)`]
*/
export function stat(path: string) {
	return new Promise<FileStat>(function(resolve, reject) {
		_fs.stat(path, (err?: Error, r?: FileStat)=>err?reject(err):resolve(r as FileStat));
	});
}

/**
 * Please ref: sync method [`existsSync(path)`]
*/
export function exists(path: string) {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.exists(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}

/**
 * Please ref: sync method [`isFileSync(path)`]
*/
export function isFile(path: string) {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.isFile(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}

/**
 * Please ref: sync method [`isDirectorySync(path)`]
*/
export function isDirectory(path: string) {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.isDirectory(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}

/**
 * Please ref: sync method [`readableSync(path)`]
*/
export function readable(path: string) {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.readable(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}

/**
 * Please ref: sync method [`writableSync(path)`]
*/
export function writable(path: string) {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.writable(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}

/**
 * Please ref: sync method [`executableSync(path)`]
*/
export function executable(path: string) {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.executable(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}

/**
 * @method chmodRecursion(path[,mode])
 * 
 * 异步递归设置文件或目录`mode`属性
 *
 * @param path {string}
 * @param mode? {int} **[`defaultMode`]**
 * @return {AsyncTask}
 * 
 * Example:
 * 
 * ```ts
 * // `mypath`为文件路径,可以为文件也可以为目录
 * fs.chmodR(mypath, 0755).then(function() {
 * 	console.log('Success');
 * }).catch(err=>{
 * 	console.log('Fail');
 * });
 * var id = fs.chmodR(mydir, 0775);
 * fs.abort(id);
 * ```
 */
export function chmodRecursion(path: string, mode: number = _fs.defaultMode) {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.chmodRecursion(path, mode, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 *
 * @method chownRecursion(path,owner,group)
 * 
 * 异步递归设置文件或目录`owner`与`group`属性
 * 
 * @param path {string}
 * @param owner {int}
 * @param group {int}
 * @return {AsyncTask}
 * 
 * Example:
 * 
 * ```ts
 * var task = chownRecursion(mypath, 501, 501);
 * fs.abort(task.id); // force abort task
 * ```
 */
export function chownRecursion(path: string, owner: number, group: number) {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.chownRecursion(path, owner, group, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * @method removeRecursion(path)
 * 
 * 递归删除文件与目录
 *
 * @param path {string}
 * @return {AsyncTask}
 * 
 * Example:
 * ```ts
 * var task = fs.removeRecursion(mypath);
 * task.then(()=>{
 * 	// Success
 * }).catch(err=>{
 * 	// Fail
 * });
 * // 通过id可中止删除任务
 * fs.abort(task.id);
```
*/
export function removeRecursion(path: string) {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.removeRecursion(path, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * @method copyRecursion(path,target)
 * 
 * 递归拷贝文件
 *
 * `copyRecursion()`与`copy()`区别在于，`copy()`只能拷贝单个文件
 *
 * @param path string
 * @param target string
 * @return {AsyncTask}
 * 
 * Example:
 * ```ts
 * fs.copy(source, target).then(()=>{
 * 	// Success
 * }).catch(err=>{
 * 	// Fail
 * });
 * ```
 */
export function copyRecursion(path: string, target: string) {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.copyRecursion(path, target, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * @method copy(path,target)
 * 
 * 拷贝单个文件
 * 
 * Ref: [`copyRecursion(path,target)`]
 * 
 * @param path {string}
 * @param target {string}
 * @return {AsyncTask}
*/
export function copy(path: string, target: string) {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.copy(path, target, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * @method readStream(path,cb)
 * 
 * 使用异步流方式读取文件内容
 * 
 * @param path {string} 读取目标路径
 * @param cb {StreamResponseCallback} 异步流回调函数
 * @return {AsyncTask}
*/
export function readStream(path: string, cb: StreamResponseCallback): AsyncTask<void> {
	return new AsyncTask<void>(function(resolve, reject): number {
		return _fs.readStream(function(err?: Error, r?: StreamResponse) {
			if (err) {
				reject(err);
			} else {
				let stream = r!;
				cb(stream);
				if (stream.ended) {
					resolve();
				}
			}
		}, path);
	});
}

/**
 * @method abort(id)
 * 
 * 通过`id`强制中止运行中的异步任务
 * 
 * 如果传入无意义的`id`或`id`所属的任务已经完成，不做任何处理
 * 
 * @param id {int}
 * 
 * Example:
 * 
 * ```ts
 * var a = fs.chmod(mypath, 0o755);
 * var b = fs.chown(mypath, 501, 501);
 * var c = fs.copy(mypath, newpath);
 * // force abort task
 * fs.abort(a.id);
 * fs.abort(b.id);
 * fs.abort(c.id);
 * ```
*/
export declare function abort(id: number): void;

// async
/**
 * Please ref: sync method [`writeFileSync(path,data[,size])`]
*/
export declare function writeFile(path: string, data: Uint8Array, size?: number): Promise<number>;

/**
 * Please ref: sync method [`writeFileSync(path,data[,encoding])`]
*/
export declare function writeFile(path: string, data: string, encoding?: Encoding): Promise<number>;

/**
 * Please ref: sync method [`readFileSync(path)`]
*/
export declare function readFile(path: string): Promise<Uint8Array>;

/**
 * Please ref: sync method [`readFileSync(path,encoding)`]
*/
export declare function readFile(path: string, encoding: Encoding): Promise<string>;

/**
 * Please ref: sync method [`openSync(path[,flags])`]
*/
export declare function open(path: string, flags?: FileOpenFlag): Promise<number>;

/**
 * Please ref: sync method [`closeSync(fd)`]
*/
export declare function close(path: number): Promise<void>;

/**
 * Please ref: sync method [`readSync(fd,out[,size[,offsetFd]])`]
*/
export declare function read(fd: number, out: Uint8Array, size?: number, offsetFd?: number): Promise<number>;

/**
 * Please ref: sync method [`writeSync(fd,data[,size[,offsetFd]])`]
*/
export declare function write(fd: number, data: Uint8Array, size?: number, offsetFd?: number): Promise<number>;

/**
 * Please ref: sync method [`writeSync(fd,data[,offsetFd])`]
*/
export declare function write(fd: number, data: string, offsetFd?: number): Promise<number>;

/**
 * Please ref: sync method [`writeSync(fd,data,encoding[,offsetFd])`]
*/
export declare function write(fd: number, data: string, encoding: Encoding, offsetFd?: number): Promise<number>;

/**
 * @interface Reader
 *
 * 这里提供的方法可以针对不同协议的uri路径进行基本的读取操作
 * 
 * 现在支持的路径类型：
 * 
 * * `http://` or `https://` - 可使用同步或异步方式进行读取,但不能读取目录或测试存在, 
 * `readdirSync()`返回空数组而`isFileSync()`永远返回`false`。
 * 
 * * `file://` 本地文件路径。`/var/data` or `var/data` 都可做为本地路径，并不会出错。
 * 
 * * `zip://`	这是`zip`包内路径的一种表示方法，`zip:///var/data/test.zip@/a.txt` 
 * 这个路径表示`zip:///var/data/test.zip`中的`a.txt`文件。注意这个路径一定要存在于本地文件系统中
 *
 * 
 * The methods provided here can perform basic read operations on URI paths of different protocols
 *
 * Currently supported path types:
 *
 * * `http://` or `https://` - can be read synchronously or asynchronously, but cannot read directories or test existence,
 * `readdirSync()` returns an empty array and `isFileSync()` always returns `false`.
 *
 * * `file://` local file path. `/var/data` or `var/data` can be used as local paths without error.
 *
 * * `zip://` This is a way to represent the path in the `zip` package, `zip:///var/data/test.zip@/a.txt`
 * This path represents the `a.txt` file in `zip:///var/data/test.zip`. Note that this path must exist in the local file system
 */
export interface Reader {

	/**
	 * @method readFile(path)
	 * 
	 * 读取文件数据
	 * 
	 * Reading file data
	 * 
	 * @param path {string}
	 * @return {AsyncTask<Uint8Array>}
	 * 
	 * For Example:
	 * 
	 * ```ts
	 * reader.readFile('http://xxx.com/test.txt').then(()=>{
	 * 	// Success
	 * }).catch(err=>{
	 * 	// Fail
	 * })
	 * ```
	*/
	readFile(path: string): AsyncTask<Uint8Array>;

	/**
	 * @method readFile(path,encoding)
	 * 
	 * 读取文件数据，并解码为字符串
	 * 
	 * Read file data and decode it into a string
	 * 
	 * @param path {string}
	 * @param encoding {Encoding}
	 * @return {AsyncTask<string>}
	*/
	readFile(path: string, encoding: Encoding): AsyncTask<string>;

	/**
	 * @method readStream(path,cb)
	 * 
	 * 以异步流方式读取文件数据
	 * 
	 * Read file data by asynchronous streaming
	 * 
	 * @param path {string}
	 * @param cb {StreamResponseCallback}
	 * @return {AsyncTask<void>}
	 * 
	 * Example:
	 * 
	 * ```ts
	 * // async read file stream 
	 * reader.readStream('http://www.baidu.com', function(d){}));
	 * reader.readStream('file:///var/data/test.txt', function(d){}));
	 * reader.readStream('zip:///var/data/test.zip@aa.txt', function(d){
	 * 	// Success
	 * 	console.log(d.data.length, d.ended);
	 * });
	 * ```
	*/
	readStream(path: string, cb: StreamResponseCallback): AsyncTask<void>;

	/**
	 * Please ref: method [`Reader.readFile(path)`]
	*/
	readFileSync(path: string): Uint8Array;

	/**
	 * Please ref: method [`Reader.readFile(path,encoding)`]
	*/
	readFileSync(path: string, encoding: Encoding): string;

	/**
	 * @method existsSync(path)
	 * 
	 * 同步测试文件或目录是否存在，如果文件存在会返回`false`
	 * 这个方法不能处理`http://`与`https://`类型的路径,如果传入这种路径立即返回`false`
	 * 
	 * Synchronously test whether a file or directory exists. If the file exists, it will return `false`
	 * This method cannot handle paths of the `http://` and `https://` type. If such a path is passed, it will immediately return `false`
	 * 
	 * @param path {string}
	 * @return {bool}
	*/
	existsSync(path: string): boolean;

	/**
	 * @method isFileSync(path)
	 * 
	 * 同步检查路径是否为文件，如果路径不存在返回 `false`
	 * 这个方法不能处理`http://`与`https://`类型的路径,如果传入这种路径立即返回`false`
	 * 
	 * Synchronously check if the path is a file, and return `false` if the path does not exist
	 * This method cannot handle `http://` and `https://` type paths, and immediately returns `false` if such a path is passed
	 * 
	 * @param path {string}
	 * @return {bool}
	*/
	isFileSync(path: string): boolean;

	/**
	 * @method isDirectorySync(path)
	 * 
	 * 同步检查路径是否为目录，如果路径不存在返回 `false`
	 * 这个方法不能处理`http://`与`https://`类型的路径,如果传入这种路径立即返回`false`
	 * 
	 * Synchronously check if the path is a directory, and return `false` if the path does not exist
	 * This method cannot handle `http://` and `https://` type paths, and immediately returns `false` if such a path is passed in
	 * 
	 * @param path {string}
	 * @return {bool}
	*/
	isDirectorySync(path: string): boolean;

	/**
	 * @method readdirSync(path)
	 * 
	 * 同步读取目录文件列表,
	 * 这个方法不能处理`http://`与`https://`类型的路径,如果传入这种路径立即返回一个空数组[`Array`],
	 * 这个方法也不会抛出异常，如果不能读取路径，只会返回空数组[`Array`]
	 * 
	 * Synchronously read the directory file list,
	 * This method cannot handle `http://` and `https://` type paths. If such a path is passed, an empty array [`Array`] will be immediately returned.
	 * This method will not throw an exception. If the path cannot be read, it will only return an empty array [`Array`]
	 * 
	 * @param path {string}
	 * @return {Dirent[]}
	*/
	readdirSync(path: string): Dirent[];

	/**
	 * @method abort(id) 通过`id`中止异步任务
	 * @param id {uint}
	*/
	abort(id: number): void;

	/**
	 * @method clear() To clear cache of reader
	*/
	clear(): void;
}

/**
 * @const reader
*/
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
					if (stream.ended) {
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