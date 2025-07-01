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
	/** Open a read-only file. The file must exist. */
	FOPEN_RDONLY = 0o0,
	/** Open a write-only file. The file must exist. */
	FOPEN_WRONLY = 0o1,
	/** Open a file for reading and writing. The file must exist. */
	FOPEN_RDWR = 0o2,
	/** Create the file if it does not exist */
	FOPEN_CREAT = 0o100,
	/** Create a file as executable */
	FOPEN_EXCL = 0o200,
	/** No Controlling TTY */
	FOPEN_NOCTTY = 0o400,
	/** Clear the file contents if the file exists */
	FOPEN_TRUNC = 0o1000,
	/** Move the file read cursor to the end if the file exists */
	FOPEN_APPEND = 0o2000,
	/** Blocking mode, usually using the open function */
	FOPEN_NONBLOCK = 0o4000,
	/** r, open the file for reading only, the file must exist */
	FOPEN_R = FOPEN_RDONLY,
	/**
	 * w, Open a write-only file. If the file exists, the file length is cleared to zero, that is,
	 * the file content will disappear. If the file does not exist, it will be created. */
	FOPEN_W = FOPEN_WRONLY | FOPEN_CREAT | FOPEN_TRUNC,
	/**
	 * a, Open a write-only file in append mode. If the file does not exist, it will be created.
	 * If the file exists, the written data will be added to the end of the file, that is, 
	 * the original content of the file will be retained.
	 */
	FOPEN_A = FOPEN_WRONLY | FOPEN_CREAT | FOPEN_APPEND,
	/** 
	 * r+,
	 * Open a file for reading and writing. The file must exist.
	 */
	FOPEN_RP = FOPEN_RDWR,
	/**
	 * w+, Open the file for reading and writing. If the file exists,
	 * the file length will be cleared to zero, that is, the file content will disappear.
	 * If the file does not exist, it will be created.
	 */
	FOPEN_WP = FOPEN_RDWR | FOPEN_CREAT | FOPEN_TRUNC,
	/**
	 * a+, Open a readable and writable file in append mode. If the file does not exist,
	 * it will be created. If the file exists, the written data will be added to the end of the file,
	 * that is, the original content of the file will be retained.
	 */
	FOPEN_AP = FOPEN_RDWR | FOPEN_CREAT | FOPEN_APPEND,
}

/**
 * @enum FileType
 * 
 * File type
*/
export enum FileType {
	/** Unknown Type */
	FTYPE_UNKNOWN,
	/** Normal File */
	FTYPE_FILE,
	/** Directory) */
	FTYPE_DIR,
	/** Symbolic link files（Soft Link） */
	FTYPE_LINK,
	/** Named pipes, a special type of file used for interprocess communication (IPC) */
	FTYPE_FIFO,
	/** Socket */
	FTYPE_SOCKET,
	/**
	 * Unbuffered character devices, such as keyboard and mouse (/dev/input/*), terminal (/dev/tty*)
	 */
	FTYPE_CHAR,
	/**
	 * Provides buffered, block-oriented (fixed-size data blocks) I/O access,
	 * such as hard disk drives (/dev/sda, /dev/sda1, /dev/nvme0n1p1)
	 */
	FTYPE_BLOCK,
}
//!< @end

/**
 * @const defaultMode
 * 
 * Create and set the default `mode` value of the file, 
 * which is related to the file's permissions. This is an `int` integer type value
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
 * The result returned by calling readdir/readdirSync
 * 
*/
export interface Dirent {
	/** File name */
	name: string;
	/** The full path to the file */
	pathname: string;
	/** File Type */
	type: FileType;
}

/**
 * @class FileState
*/
export declare class FileStat {
	/** Is the file valid? */
	isValid(): boolean;
	/** Is it a normal file? */
	isFile(): boolean;
	/** Is it a directory? */
	isDir(): boolean;
	/** Is it a symbolic link? */
	isLink(): boolean;
	/** Is it a socket? */
	isSock(): boolean;
	/** File permission mask */
	mode(): number;
	/** File type */
	type(): FileType;
	/** System Group ID */
	group(): number;
	/** System User ID  */
	owner(): number;
	/** File size */
	size(): number;
	/** Number of hard links to the file */
	nlink(): number;
	/** The file system specific "Inode" number of the file */
	ino(): number;
	/** The file system block size used for I/O operations */
	blksize(): number;
	/** The number of blocks allocated for this file */
	blocks(): number;
	/** flags */
	flags(): number;
	/** gen */
	gen(): number;
	/** The device ID that contains the file */
	dev(): number;
	/** If the file represents a device, the numeric device identifier */
	rdev(): number;
	/** Timestamp indicating the last time this file was accessed */
	atime(): number;
	/** A timestamp indicating when this file was last modified */
	mtime(): number;
	/** Timestamp indicating the last time the file's status changed */
	ctime(): number;
	/** A timestamp indicating when this file was created */
	birthtime(): number;
}

/**
 * @class Stream
*/
export interface Stream {
	/** Pause stream reading */
	pause(): void;
	/** Resume stream reading */
	resume(): void;
};

/**
 * @class StreamResponse
*/
export interface StreamResponse {
	/** Data size */
	size: number;
	/** Total data size */
	total: number;
	/** It is Data */
	data: Uint8Array;
	/** Is it over */
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
	
	/**
	 * Asynchronous I/O task id
	 */
	get id() { return this._id }

	/**
	 * Whether the asynchronous task is completed
	 */
	get complete() { return this._complete }

	/**
	 * Abort I/O task
	 * @method abort(reason)
	 * @param reason? {Error} If the parameter is passed in, an exception will be thrown
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
 * Synchronously change file permissions
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
 * Synchronize the `owner` and `group` of the configuration file
 * 
 * @param path {string}
 * @param owner {number}
 * @param group {number}
*/
export declare function chownSync(path: string, owner: number, group: number): void;

/**
 * @method mkdirSync(path[,mode])
 * 
 * Create a directory synchronously. If the path already exists, an exception will be thrown.
 * 
 * @param path {string}
 * @param mode? {int} **[`defaultMode`]**
*/
export declare function mkdirSync(path: string, mode?: number): void;

/**
 * @method mkdirsSync(path[,mode])
 * 
 * Synchronously recursively create directories,
 * and no exception is thrown if the directory exists
 * 
 * @param path string
 * @param mode? int **[`defaultMode`]**
 */
export declare function mkdirsSync(path: string, mode?: number): void;

/**
 * Synchronously rename files and directories
 * 
 * @mehod renameSync(name,newName)
 * @param name {string}
 * @param newName {string}
*/
export declare function renameSync(name: string, newName: string): void;

/**
 * Synchronously create file hard links
 * 
 * @method linkSync(src,target)
 * @param src {string} Original file path
 * @param target {string} Link target path
*/
export declare function linkSync(src: string, target: string): void;

/**
 * Synchronously delete the file hard link.
 * If the file has only one link, the file will be physically deleted.
 * 
 * @method unlinkSync(path)
 * @param path {string}
*/
export declare function unlinkSync(path: string): void;

/**
 * Synchronously delete the file directory, the file directory must be empty
 * 
 * @method rmdirSync(path)
 * @param path {string}
*/
export declare function rmdirSync(path: string): void;

/**
 * Synchronously read the file directory file list
 * 
 * @method readdirSync(path)
 * @param path {string}
 * @return {Dirent[]}
*/
export declare function readdirSync(path: string): Dirent[];

/**
 * @method statSync(path)
 * 
 * Synchronously read file status information
 * 
 * @return {FileState}
*/
export declare function statSync(path: string): FileStat;

/**
 * Synchronously check if a file exists
 * 
 * @method existsSync(path)
 * @return {bool}
*/
export declare function existsSync(path: string): boolean;

/**
 * Synchronously checks if the path is a file, and returns `false` if the path does not exist
 * 
 * @method isFileSync(path)
 * @param path {string}
 * @return {bool}
*/
export declare function isFileSync(path: string): boolean;

/**
 * Synchronously checks if the path is a directory, and returns `false` if the path does not exist
 * 
 * @method isDirectorySync(path)
 * @param path {string}
 * @return {bool}
*/
export declare function isDirectorySync(path: string): boolean;

/**
 * Synchronously checks if the path is readable, returning `false` if the path does not exist
 * 
 * @method readableSync(path)
 * @param path {string}
 * @return {bool}
*/
export declare function readableSync(path: string): boolean;

/**
 * Synchronously checks if the path is writable, returning `false` if the path does not exist
 * 
 * @method writableSync(path)
 * @param path {string}
 * @return {bool}
*/
export declare function writableSync(path: string): boolean;

/**
 * Synchronously checks if it is executable, returns `false` if the path does not exist
 * 
 * @method executableSync(path)
 * @param path {string}
 * @return {bool}
*/
export declare function executableSync(path: string): boolean;

/**
 * 
 * @mehod chmodRecursionSync(path[,mode])
 * 
 * Synchronously recursively set the file permission attribute mode
 * 
 * (TODO: will block the calling thread, please use with caution)
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
 * Synchronously recursively set file `owner` and `group` attributes
 * 
 * (TODO: will block the calling thread, please use with caution)
 * 
 * @param path string
 * @param owner int System User ID
 * @param group int System Group ID
 */
export declare function chownRecursionSync(path: string, owner: number, group: number): void;

/**
 * 
 * @method removeRecursionSync(path)
 * 
 * Synchronous recursive deletion of directories or files,
 * 
 * use this method with caution, it may cause the thread to be blocked for a long time
 * 
 * @param path {string}
 */
export declare function removeRecursionSync(path: string): void;

/**
 * @method copyRecursionSync(path,target)
 * 
 * Synchronous recursive copy of files,
 * 
 * use this method with caution, it may cause the thread to be blocked for a long time
 *
 * @param path strings
 * @param target string
 * 
 */
export declare function copyRecursionSync(path: string, target: string): void;

/**
 * @method copySync
 * 
 * Copy files synchronously (TODO: will block the calling thread, please use with caution)
 * 
 * Same as: copy files and directories[`copyRecursionSync(path，target)`]
 * 
 * @param path string
 * @param target string
*/
export declare function copySync(path: string, target: string): void;

// read/write file sync
/**
 * @method writeFileSync(path,data[,size])
 * 
 * Synchronously write data to a file
 * 
 * @param path {string}
 * @param data {Uint8Array}
 * @param size? {uint} Write all data when no parameters are passed
*/
export declare function writeFileSync(path: string, data: Uint8Array, size?: number): number;

/**
 * @method writeFileSync(path,data[,encoding])
 * 
 * Synchronously write a string to a file
 * 
 * @param path {string}
 * @param data {string}
 * @param encoding? {Encoding} If not passed in, the default encoding is `utf-8`
*/
export declare function writeFileSync(path: string, data: string, encoding?: Encoding): number;

/**
 * @method readFileSync(path)
 * 
 * Reading files synchronously
 * 
 * @param path {string}
 * @return {Uint8Array}
*/
export declare function readFileSync(path: string): Uint8Array;

/**
 * @method readFileSync(path,encoding)
 * 
 * Synchronously read a file as a string
 * 
 * @param path {string}
 * @param encoding {Encoding} Decode data to string encoding
 * @return {string}
*/
export declare function readFileSync(path: string, encoding: Encoding): string;

/**
 * @method openSync(path[,flags])
 * 
 * Opens a file by path synchronously and returns an open file handle
 * 
 * @param path {string}
 * @param flags? {FileOpenFlag} Open file flags mask
 * @return {uint}
*/
export declare function openSync(path: string, flags?: FileOpenFlag /*= FileOpenFlag.FOPEN_R*/): number;

/**
 * @method closeSync(fd)
 * 
 * Close the file handle synchronously
*/
export declare function closeSync(fd: number): void;

/**
 * @method readSync(fd,out[,size[,offsetFd]])
 * 
 * Synchronously read the file contents from the file handle
 * 
 * @param fd {uint} Open file handles
 * @param out {Uint8Array} Read the file and save it here
 * @param size? {int} If not passed or passed in `-1`, the length of the `out` parameter is used
 * @param offsetFd? {int} If not passed or -1 is passed,
 * 	the internal offset value of the file handle is used (it will advance after each read)
 * @return {uint} Returns the size of the data actually read
*/
export declare function readSync(fd: number, out: Uint8Array, size?: number /*= -1*/, offsetFd?: number /*= -1*/): number;

/**
 * @method writeSync(fd,data[,size[,offsetFd]])
 * 
 * Synchronously write data to a file handle
 * 
 * @param fd {uint} Open file handles
 * @param data {Uint8Array} Data to be written
 * @param size? {int} If not passed or `-1` is passed, all data will be written
 * @param offsetFd? {int} If not passed or `-1` is passed, the internal offset value of 
 * 	the file handle is used (it will advance after each write)
 * @return {uint} The actual size of the data written
*/
export declare function writeSync(fd: number, data: Uint8Array, size?: number /*= -1*/, offsetFd?: number /*= -1*/): number;

/**
 * @method writeSync(fd,data[,offsetFd])
 * 
 * Write data to the file handle synchronously and encode the data using `utf-8`
 * 
 * @param fd {uint} Open file handles
 * @param data {string} The string to be written
 * @param offsetFd? {int} If not passed or `-1` is passed, the internal offset value of 
 * 	the file handle is used (it will advance after each write)
 * @return {uint} The actual size of the data written
*/
export declare function writeSync(fd: number, data: string, offsetFd?: number /*= -1*/): number;

/**
 * @method writeSync(fd,data,encoding[,offsetFd])
 * 
 * Synchronously write data to a file handle
 * 
 * @param fd {uint} Open file handles
 * @param data {string} Data to be written
 * @param encoding {Encoding} Encoding Type
 * @param offsetFd? {int} If not passed or `-1` is passed, the internal offset value of 
 * 	the file handle is used (it will advance after each write)
 * @return {uint} The actual size of the data written
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
 * Recursively create directories. This method will create 
 * 	a directory tree in sequence, and will not throw an exception if the directory exists.
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
	* Read directory listing information. Throws an exception if failed. 
	* 	Returns an [`Array`] of [`Dirent`] if successful.
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
 * Asynchronously recursively set the `mode` attribute of a file or directory
 *
 * @param path {string}
 * @param mode? {int} **[`defaultMode`]**
 * @return {AsyncTask}
 * 
 * Example:
 * 
 * ```ts
 * // `mypath`For the file path, it can be a file or a directory
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
 * Asynchronously recursively set the `owner` and `group` attributes of files or directories
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
 * Recursively delete files and directories
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
 * // The deletion task can be aborted by id
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
 * Recursively copy files
 *
 * The difference between `copyRecursion()` and `copy()` is that `copy()` can only copy a single file
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
 * Copy a single file
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
 * Read file contents using asynchronous streaming
 * 
 * @param path {string} Read the target path
 * @param cb {StreamResponseCallback} Asynchronous stream callback function
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
 * Force abort a running asynchronous task by `id`
 * 
 * If a meaningless `id` is passed in or the task to which 
 * 	the `id` belongs has been completed, no processing will be done
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
//!< @end

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