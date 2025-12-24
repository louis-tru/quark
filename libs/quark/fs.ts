/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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
import type {Uint,Int} from './defs';

const _fs = __binding__('_fs');

/**
 * Opening file Flags
*/
export enum FileOpenFlag {
	FOPEN_ACCMODE = 0o3, //!<
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
 * @interface Dirent
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
 * @class FileStat
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
 * @interface Stream
*/
export interface Stream {
	/** Pause stream reading */
	pause(): void;
	/** Resume stream reading */
	resume(): void;
};

/**
 * @interface StreamResponse
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
	 * @param reason? If the parameter is passed in, an exception will be thrown
	*/
	abort(reason?: Error): void {
		_fs.abort(this._id);
		if (reason) {
			// ERR_READ_STREAM_ABORT: ErrnoCode = [-12001, 'ERR_READ_STREAM_ABORT']
			_util.nextTick(()=>this._err(reason));
		}
	}
}

/**
 * Synchronously change file permissions
 * 
 * ```ts
 * chmodSync('my_file.txt', 0o775)
 * ```
*/
export declare function chmodSync(path: string, mode?: Uint): void;

/**
 * Synchronize the `owner` and `group` of the configuration file
*/
export declare function chownSync(path: string, owner: Uint, group: Uint): void;

/**
 * Create a directory synchronously. If the path already exists, an exception will be thrown.
 * 
 * @param mode? **[`defaultMode`]**
*/
export declare function mkdirSync(path: string, mode?: Uint): void;

/**
 * Synchronously recursively create directories,
 * and no exception is thrown if the directory exists
 *
 * @param mode? **[`defaultMode`]**
 */
export declare function mkdirsSync(path: string, mode?: Uint): void;

/**
 * Synchronously rename files and directories
*/
export declare function renameSync(name: string, newName: string): void;

/**
 * Synchronously create file hard links
 * 
 * @param src Original file path
 * @param target Link target path
*/
export declare function linkSync(src: string, target: string): void;

/**
 * Synchronously delete the file hard link.
 * If the file has only one link, the file will be physically deleted.
*/
export declare function unlinkSync(path: string): void;

/**
 * Synchronously delete the file directory, the file directory must be empty
*/
export declare function rmdirSync(path: string): void;

/**
 * Synchronously read the file directory file list
*/
export declare function readdirSync(path: string): Dirent[];

/**
 * Synchronously read file status information
*/
export declare function statSync(path: string): FileStat;

/**
 * Synchronously check if a file exists
*/
export declare function existsSync(path: string): boolean;

/**
 * Synchronously checks if the path is a file, and returns `false` if the path does not exist
*/
export declare function isFileSync(path: string): boolean;

/**
 * Synchronously checks if the path is a directory, and returns `false` if the path does not exist
*/
export declare function isDirectorySync(path: string): boolean;

/**
 * Synchronously checks if the path is readable, returning `false` if the path does not exist
*/
export declare function readableSync(path: string): boolean;

/**
 * Synchronously checks if the path is writable, returning `false` if the path does not exist
*/
export declare function writableSync(path: string): boolean;

/**
 * Synchronously checks if it is executable, returns `false` if the path does not exist
*/
export declare function executableSync(path: string): boolean;

/**
 *
 * Synchronously recursively set the file permission attribute mode
 * 
 * (TODO: will block the calling thread, please use with caution)
 * 
 * @param mode? **[`defaultMode`]**
 * @example
 * 
 * ```ts
 * fs.chmodRecursionSync(mypath, 0755);
 * ```
*/
export declare function chmodRecursionSync(path: string, mode?: Uint): void;

/**
 * Synchronously recursively set file `owner` and `group` attributes
 * 
 * (TODO: will block the calling thread, please use with caution)
 *
 * @param owner System User ID
 * @param group System Group ID
 */
export declare function chownRecursionSync(path: string, owner: Uint, group: Uint): void;

/**
 * Synchronous recursive deletion of directories or files,
 * 
 * use this method with caution, it may cause the thread to be blocked for a long time
 */
export declare function removeRecursionSync(path: string): void;

/**
 * Synchronous recursive copy of files,
 * 
 * use this method with caution, it may cause the thread to be blocked for a long time
 */
export declare function copyRecursionSync(path: string, target: string): void;

/**
 * Copy files synchronously (TODO: will block the calling thread, please use with caution)
 * 
 * Same as: copy files and directories[`copyRecursionSync(path,target)`]
*/
export declare function copySync(path: string, target: string): void;

/**
 * Synchronously write data to a file
 *
 * @param size? Write all data when no parameters are passed
*/
export declare function writeFileSync(path: string, data: Uint8Array, size?: Uint): Uint;

/**
 * Synchronously write a string to a file
 *
 * @param encoding? If not passed in, the default encoding is `utf-8`
*/
export declare function writeFileSync(path: string, data: string, encoding?: Encoding): Uint;

/**
 * Reading files synchronously
*/
export declare function readFileSync(path: string): Uint8Array;

/**
 * Synchronously read a file as a string
 * 
 * @param encoding Decode data to string encoding
*/
export declare function readFileSync(path: string, encoding: Encoding): string;

/**
 * Opens a file by path synchronously and returns an open file handle
 * 
 * @param flags? Open file flags mask **Default** as FileOpenFlag.FOPEN_R
*/
export declare function openSync(path: string, flags?: FileOpenFlag): Uint;

/**
 * Close the file handle synchronously
*/
export declare function closeSync(fd: Uint): void;

/**
 * Synchronously read the file contents from the file handle
 * 
 * @param fd Open file handles
 * @param out Read the file and save it here
 * @param size? If not passed or passed in `-1`, the length of the `out` parameter is used
 * @param offsetFd? If not passed or -1 is passed,
 * 	the internal offset value of the file handle is used (it will advance after each read)
 * @return Returns the size of the data actually read
*/
export declare function readSync(fd: Uint, out: Uint8Array, size?: Int, offsetFd?: Int): Uint;

/**
 * Synchronously write data to a file handle
 * 
 * @param fd Open file handles
 * @param data Data to be written
 * @param size? If not passed or `-1` is passed, all data will be written
 * @param offsetFd? If not passed or `-1` is passed, the internal offset value of 
 * 	the file handle is used (it will advance after each write)
 * @return The actual size of the data written
*/
export declare function writeSync(fd: Uint, data: Uint8Array, size?: Int, offsetFd?: Int): Uint;

/**
 * Write data to the file handle synchronously and encode the data using `utf-8`
 * 
 * @param fd Open file handles
 * @param data The string to be written
 * @param offsetFd? If not passed or `-1` is passed, the internal offset value of 
 * 	the file handle is used (it will advance after each write)
 * @return The actual size of the data written
*/
export declare function writeSync(fd: Uint, data: string, offsetFd?: Int): Uint;

/**
 * Synchronously write data to a file handle
 * 
 * @param fd Open file handles
 * @param data Data to be written
 * @param encoding Encoding Type
 * @param offsetFd? If not passed or `-1` is passed, the internal offset value of 
 * 	the file handle is used (it will advance after each write)
 * @return The actual size of the data written
*/
export declare function writeSync(fd: Uint, data: string, encoding: Encoding, offsetFd?: Int): Uint;

Object.assign(exports, {..._fs, ...exports});

// async
/**
 * Please ref: sync method [`chmodSync(path,mode?)`]
 * @method chmod(path,mode?)
 * @param path:string
 * @param mode?:Uint
*/
export function chmod(path: string, mode: Uint = _fs.defaultMode) {
	return new Promise<void>(function(resolve, reject) {
		_fs.chown(path, mode, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * Please ref: sync method [`chownSync(path,owner,group)`]
*/
export function chown(path: string, owner: Uint, group: Uint) {
	return new Promise<void>(function(resolve, reject) {
		_fs.chown(path, owner, group, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * Please ref: sync method [`mkdirSync(path,mode?)`]
 * @method chmod(path,mode?)
 * @param path:string
 * @param mode?:Uint
*/
export function mkdir(path: string, mode: Uint = _fs.defaultMode) {
	return new Promise<void>(function(resolve, reject) {
		_fs.mkdir(path, mode, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * @method mkdirs(path,mode?)
 * 
 * Recursively create directories. This method will create 
 * 	a directory tree in sequence, and will not throw an exception if the directory exists.
 * 
 * Ref: sync method [`mkdirsSync(path[,mode])`]
 * 
 * @param path:string
 * @param mode?:Uint **[`defaultMode`]**
 * @return {Promise}
 * 
 * @example
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
	* Read directory listing information. Throws an exception if failed. 
	* 	Returns an [`Array`] of [`Dirent`] if successful.
	*
	*	@example
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
export function readdir(path: string): Promise<Dirent[]> {
	return new Promise<Dirent[]>(function(resolve, reject) {
		_fs.readdir(path, (err?: Error, r?: Dirent[])=>err?reject(err):resolve(r as Dirent[]));
	});
}

/**
 * Please ref: sync method [`statSync(path)`]
*/
export function stat(path: string): Promise<FileStat> {
	return new Promise<FileStat>(function(resolve, reject) {
		_fs.stat(path, (err?: Error, r?: FileStat)=>err?reject(err):resolve(r as FileStat));
	});
}

/**
 * Please ref: sync method [`existsSync(path)`]
*/
export function exists(path: string): Promise<boolean> {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.exists(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}

/**
 * Please ref: sync method [`isFileSync(path)`]
*/
export function isFile(path: string): Promise<boolean> {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.isFile(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}

/**
 * Please ref: sync method [`isDirectorySync(path)`]
*/
export function isDirectory(path: string): Promise<boolean> {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.isDirectory(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}

/**
 * Please ref: sync method [`readableSync(path)`]
*/
export function readable(path: string): Promise<boolean> {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.readable(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}

/**
 * Please ref: sync method [`writableSync(path)`]
*/
export function writable(path: string): Promise<boolean> {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.writable(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}

/**
 * Please ref: sync method [`executableSync(path)`]
*/
export function executable(path: string): Promise<boolean> {
	return new Promise<boolean>(function(resolve, reject) {
		_fs.executable(path, (err?: Error, r?: boolean)=>err?reject(err):resolve(r as boolean));
	});
}

/**
 * @method chmodRecursion(path,mode?)
 * 
 * Asynchronously recursively set the `mode` attribute of a file or directory
 *
 * @param path:string
 * @param mode?:Uint **[`defaultMode`]**
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
 * Asynchronously recursively set the `owner` and `group` attributes of files or directories
 * 
 * @example
 * 
 * ```ts
 * var task = chownRecursion(mypath, 501, 501);
 * fs.abort(task.id); // force abort task
 * ```
 */
export function chownRecursion(path: string, owner: Uint, group: Uint): AsyncTask<void> {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.chownRecursion(path, owner, group, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * Recursively delete files and directories
 *
 * @example
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
export function removeRecursion(path: string): AsyncTask<void> {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.removeRecursion(path, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * Recursively copy files
 *
 * The difference between `copyRecursion()` and `copy()` is that `copy()` can only copy a single file
 * 
 * @example
 * ```ts
 * fs.copy(source, target).then(()=>{
 * 	// Success
 * }).catch(err=>{
 * 	// Fail
 * });
 * ```
 */
export function copyRecursion(path: string, target: string): AsyncTask<void> {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.copyRecursion(path, target, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * Copy a single file
 * 
 * Ref: [`copyRecursion(path,target)`]
*/
export function copy(path: string, target: string): AsyncTask<void> {
	return new AsyncTask<void>(function(resolve, reject) {
		return _fs.copy(path, target, (err?: Error)=>err?reject(err):resolve());
	});
}

/**
 * Read file contents using asynchronous streaming
 * 
 * @param path Read the target path
 * @param cb   Asynchronous stream callback function
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
 * Force abort a running asynchronous task by `id`
 * 
 * If a meaningless `id` is passed in or the task to which 
 * 	the `id` belongs has been completed, no processing will be done
 * 
 * @example
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
export declare function abort(id: Int): void;

// async
/**
 * Please ref: sync method [`writeFileSync(path,data,size?)`]
*/
export declare function writeFile(path: string, data: Uint8Array, size?: Uint): Promise<Uint>;

/**
 * Please ref: sync method [`writeFileSync(path,data,encoding?)`]
*/
export declare function writeFile(path: string, data: string, encoding?: Encoding): Promise<Uint>;

/**
 * Please ref: sync method [`readFileSync(path)`]
*/
export declare function readFile(path: string): Promise<Uint8Array>;

/**
 * Please ref: sync method [`readFileSync(path,encoding)`]
*/
export declare function readFile(path: string, encoding: Encoding): Promise<string>;

/**
 * Please ref: sync method [`openSync(path,flags?)`]
*/
export declare function open(path: string, flags?: FileOpenFlag): Promise<Uint>;

/**
 * Please ref: sync method [`closeSync(fd)`]
*/
export declare function close(fd: Uint): Promise<void>;

/**
 * Please ref: sync method [`readSync(fd,out,size?,offsetFd?)`]
*/
export declare function read(fd: Uint, out: Uint8Array, size?: Int, offsetFd?: Int): Promise<Uint>;

/**
 * Please ref: sync method [`writeSync(fd,data,size?,offsetFd?)`]
*/
export declare function write(fd: Uint, data: Uint8Array, size?: Int, offsetFd?: Int): Promise<Uint>;

/**
 * Please ref: sync method [`writeSync(fd,data,offsetFd?)`]
*/
export declare function write(fd: Uint, data: string, offsetFd?: Int): Promise<Uint>;

/**
 * Please ref: sync method [`writeSync(fd,data,encoding,offsetFd?)`]
*/
export declare function write(fd: Uint, data: string, encoding: Encoding, offsetFd?: Int): Promise<Uint>;

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
	 * Reading file data
	 * 
	 * @example
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
	 * Read file data and decode it into a string
	*/
	readFile(path: string, encoding: Encoding): AsyncTask<string>;

	/**
	 * Read file data by asynchronous streaming
	 * 
	 * @example
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
	 * 同步测试文件或目录是否存在，如果文件存在会返回`false`
	 * 这个方法不能处理`http://`与`https://`类型的路径,如果传入这种路径立即返回`false`
	 * 
	 * Synchronously test whether a file or directory exists. If the file exists, it will return `false`
	 * This method cannot handle paths of the `http://` and `https://` type. If such a path is passed, it will immediately return `false`
	*/
	existsSync(path: string): boolean;

	/**
	 * 同步检查路径是否为文件，如果路径不存在返回 `false`
	 * 这个方法不能处理`http://`与`https://`类型的路径,如果传入这种路径立即返回`false`
	 * 
	 * Synchronously check if the path is a file, and return `false` if the path does not exist
	 * This method cannot handle `http://` and `https://` type paths, and immediately returns `false` if such a path is passed
	*/
	isFileSync(path: string): boolean;

	/**
	 * 同步检查路径是否为目录，如果路径不存在返回 `false`
	 * 这个方法不能处理`http://`与`https://`类型的路径,如果传入这种路径立即返回`false`
	 * 
	 * Synchronously check if the path is a directory, and return `false` if the path does not exist
	 * This method cannot handle `http://` and `https://` type paths, and immediately returns `false` if such a path is passed in
	*/
	isDirectorySync(path: string): boolean;

	/**
	 * 同步读取目录文件列表,
	 * 这个方法不能处理`http://`与`https://`类型的路径,如果传入这种路径立即返回一个空数组[`Array`],
	 * 这个方法也不会抛出异常，如果不能读取路径，只会返回空数组[`Array`]
	 * 
	 * Synchronously read the directory file list,
	 * This method cannot handle `http://` and `https://` type paths. If such a path is passed, an empty array [`Array`] will be immediately returned.
	 * This method will not throw an exception. If the path cannot be read, it will only return an empty array [`Array`]
	*/
	readdirSync(path: string): Dirent[];

	/**
	 * Abort an asynchronous task by its id
	*/
	abort(id: Uint): void;

	/**
	 * To clear cache of reader
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