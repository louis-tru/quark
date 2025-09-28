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

#include "../error.h"
#include "../fs.h"
#include "../http.h"
#include "../uv.h"
#if Qk_WIN
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#define LOOP RunLoop::current()
#define LOOP2 auto loop = LOOP
#define BUFFER_SIZE (1024 * 16) // 16kb

namespace qk {
	void file_stat_copy(uv_stat_t* uv_stat, FileStat* stat);
	int  file_flag_mask(int flag);

	template<class uv_req, class Data = Object, class CbData = Object>
	class AsyncReqNonCtx: public UVRequestWrap<uv_req, Object, Data, CbData> {
	public:
		AsyncReqNonCtx(Callback<CbData> cb, RunLoop* loop = LOOP, Data data = Data())
		: UVRequestWrap<uv_req, Object, Data, CbData>(nullptr, cb, std::move(data))
		, _loop(loop)
		{}
		static inline AsyncReqNonCtx* cast(uv_req* req) {
			return (AsyncReqNonCtx*)req->data;
		}
		inline PostMessage* loop() {
			return _loop;
		}
		inline uv_loop_t* uv_loop() {
			return _loop->uv_loop();
		}
	private:
		RunLoop* _loop;
	};

	typedef AsyncReqNonCtx<uv_fs_t, String> FileReq;
	typedef AsyncReqNonCtx<uv_fs_t, String, Array<Dirent>> FileLsReq;

	static Error uv_error(uv_fs_t* req, cChar* msg = nullptr) {
		return Error((int)req->result, "%s, %s, %s",
								uv_err_name((int)req->result), uv_strerror((int)req->result), msg ? msg: "");
	}

	template<class Req>
	static void async_err_callback(Req* req, cChar* msg = nullptr) {
		async_reject(req->cb(), uv_error(req->req(), msg));
	}

	template<class Cb>
	static void async_err_callback(Cb cb, uv_fs_t* req, cChar* msg = nullptr) {
		async_reject(cb, uv_error(req, msg));
	}

	static void uv_fs_async_cb(uv_fs_t* req) {
		uv_fs_req_cleanup(req);
		Handle<FileReq> handle(FileReq::cast(req));
		if ( req->result == 0 ) { // ok
			async_callback(handle->cb());
		} else { // err
			async_err_callback(*handle);
		}
	}

	static void uv_fs_access_cb(uv_fs_t* req) {
		uv_fs_req_cleanup(req);
		Handle<FileReq> handle(FileReq::cast(req));
		async_resolve(handle->cb(), Bool(req->result == 0));
	}

	static void ls_cb(uv_fs_t* req) {
		Handle<FileLsReq> handle(FileLsReq::cast(req));
		Array<Dirent> ls;
		if ( req->result ) {
			String p =  handle->data() + '/'; // fs_format("%s", *handle->data()) + '/';
			uv_dirent_t ent;
			while ( uv_fs_scandir_next(req, &ent) == 0 ) {
				ls.push( Dirent{ent.name, p + ent.name, FileType(ent.type)} );
			}
		}
		uv_fs_req_cleanup(req);
		async_resolve(handle->cb(), std::move(ls));
	}

	static void is_file_cb(uv_fs_t* req) {
		uv_fs_req_cleanup(req);
		Handle<FileReq> handle(FileReq::cast(req));
		if ( req->result == 0 ) { // ok
			async_resolve(handle->cb(), Bool(!S_ISDIR(req->statbuf.st_mode)));
		} else { // err
			async_resolve(handle->cb(), Bool(false));
		}
	}

	static void is_dir_cb(uv_fs_t* req) {
		uv_fs_req_cleanup(req);
		Handle<FileReq> handle(FileReq::cast(req));
		if ( req->result == 0 ) { // ok
			async_resolve(handle->cb(), Bool(S_ISDIR(req->statbuf.st_mode)));
		} else { // err
			async_resolve(handle->cb(), Bool(false));
		}
	}

	static void stat_cb(uv_fs_t* req) {
		uv_fs_req_cleanup(req);
		Handle<FileReq> handle(FileReq::cast(req));
		if ( req->result == 0 ) { // ok
			FileStat stat;
			file_stat_copy(&req->statbuf, &stat);
			async_resolve(handle->cb(), std::move(stat));
		} else { // err
			async_err_callback(*handle);
		}
	}

	static void exists(cString& path, Cb cb, RunLoop* loop) {
		uv_fs_access(loop->uv_loop(),
								New<FileReq>(cb, loop)->req(),
								fs_fallback_c(path), F_OK, &uv_fs_access_cb);
	}

	static void ls(cString& path, Callback<Array<Dirent>> cb, RunLoop* loop) {
		uv_fs_scandir(loop->uv_loop(),
									New<FileLsReq>(cb, loop, path)->req(), fs_fallback_c(path), 1, &ls_cb);
	}

	static void file_stat(cString& path, Cb cb, RunLoop* loop) {
		uv_fs_stat(loop->uv_loop(), New<FileReq>(cb, loop)->req(), fs_fallback_c(path), &stat_cb);
	}

	static void is_dir(cString& path, Cb cb, RunLoop* loop) {
		uv_fs_stat(loop->uv_loop(), New<FileReq>(cb, loop)->req(), fs_fallback_c(path), &is_dir_cb);
	}

	static AsyncIOTask* cp(cString& source, cString& target, Cb cb, RunLoop* loop) {
		class Task: public AsyncIOTask, File::Delegate {
		public:
			Task(cString& source, cString& target, Cb cb, RunLoop* loop)
			: AsyncIOTask(loop)
			, _source_file(new File(source, loop))
			, _target_file(new File(target, loop))
			, _end(cb)
			, _reading_count(0), _writeing_count(0), _read_end(false)
			{//
				_buffer[0] = Buffer::alloc(BUFFER_SIZE);
				_buffer[1] = Buffer::alloc(BUFFER_SIZE);
				_source_file->set_delegate(this);
				_target_file->set_delegate(this);
				_source_file->open(FOPEN_R);
				_target_file->open(FOPEN_W);
			}
	
			virtual ~Task() {
				release_file_handle();
			}
	
			void release_file_handle() {
				if ( _source_file ) {
					Releasep(_source_file);
					Releasep(_target_file);
				}
			}
	
			Buffer alloc_buffer() {
				for ( int i = 0; i < 2; i++ ) {
					if (_buffer[i].length()) {
						return _buffer[i];
					}
				}
				return Buffer();
			}
	
			void release_buffer(Buffer& buffer) {
				for ( int i = 0; i < 2; i++ ) {
					if (_buffer[i].length() == 0) {
						 buffer.reset(BUFFER_SIZE);
						_buffer[i] = buffer;
						break;
					}
				}
				Qk_ASSERT(buffer.length() == 0);
			}
	
			void read_next() {
				Qk_ASSERT(!_read_end);
				Buffer buff = alloc_buffer();
				if ( buff.length() ) {
					_reading_count++;
					_source_file->read(buff);
				}
			}
	
			virtual void abort() {
				release_file_handle();
				AsyncIOTask::abort();
			}
	
			virtual void trigger_file_error(File* file, cError& error) {
				Handle<Task> handle(this); //
				abort();
				async_reject(_end, Error(error));
			}
	
			virtual void trigger_file_open(File* file) {
				if ( _source_file->is_open() && _target_file->is_open() ) {
					read_next();
				}
			}
	
			void copy_complete() {
				Qk_ASSERT(_reading_count == 0);
				Qk_ASSERT(_writeing_count == 0);
				Qk_ASSERT(_read_end);
				if ( !is_abort() ) { // copy complete
					//Qk_DLog("-----copy_complete------");
					Handle<Task> handle(this);
					abort();
					async_callback(_end);
				}
			}
	
			virtual void trigger_file_read(File* file, Buffer &buffer, int mark) {
				Qk_ASSERT( file == _source_file );
				Qk_ASSERT( _reading_count > 0 );
				_reading_count--;
				if ( buffer.length() ) {
					_writeing_count++;
					_target_file->write(buffer);
					read_next();
				} else {
					Qk_ASSERT(_reading_count == 0);
					Qk_ASSERT(!_read_end);
					_read_end = true;
					if ( _writeing_count == 0 ) {
						copy_complete();
					}
				}
			}

			virtual void trigger_file_write(File* file, Buffer &buffer, int mark) {
				Qk_ASSERT( file == _target_file );
				Qk_ASSERT( _writeing_count > 0 );
				_writeing_count--;
				release_buffer(buffer);
				if ( _read_end ) {
					if ( _writeing_count == 0 ) {
						copy_complete();
					}
				} else {
					if ( _reading_count == 0 ) {
						read_next();
					}
				}
			}
	
			virtual void trigger_file_close(File* file) { }
			
			File* _source_file;
			File* _target_file;
			Cb         _end;
			Buffer     _buffer[2];
			int        _reading_count;
			int        _writeing_count;
			bool       _read_end;
		};

		return NewRetain<Task>(source, target, cb, loop);
	}

	static void mkdir(cString& path, uint32_t mode, Cb cb, RunLoop* loop) {
		uv_fs_mkdir(loop->uv_loop(),
								New<FileReq>(cb, loop)->req(),
								fs_fallback_c(path), mode, &uv_fs_async_cb);
	}

	static void chmod(cString& path, uint32_t mode, Cb cb, RunLoop* loop) {
		uv_fs_chmod(loop->uv_loop(),
								New<FileReq>(cb, loop)->req(),
								fs_fallback_c(path), mode, &uv_fs_async_cb);
	}

	static void chown(cString& path, uint32_t owner, uint32_t group, Cb cb, RunLoop* loop) {
		uv_fs_chown(loop->uv_loop(),
								New<FileReq>(cb, loop)->req(),
								fs_fallback_c(path), owner, group, &uv_fs_async_cb);
	}

	static void link(cString& path, cString& newPath, Cb cb, RunLoop* loop) {
		uv_fs_link(loop->uv_loop(),
							New<FileReq>(cb, loop)->req(),
							fs_fallback_c(path), fs_fallback_c(newPath), &uv_fs_async_cb);
	}

	static void unlink(cString& path, Cb cb, RunLoop* loop) {
		uv_fs_unlink(loop->uv_loop(),
								New<FileReq>(cb, loop)->req(),
								fs_fallback_c(path), &uv_fs_async_cb);
	}

	static void rmdir(cString& path, Cb cb, RunLoop* loop) {
		uv_fs_rmdir(loop->uv_loop(),
								New<FileReq>(cb, loop)->req(),
								fs_fallback_c(path), &uv_fs_async_cb);
	}

	/**
	* @class AsyncEach
	*/
	class AsyncEach: public AsyncIOTask {
	public:

		AsyncEach(cString& path, Cb cb, Cb end, bool internal = false)
		: _path(fs_format(path))
		, _cb(cb)
		, _end(end)
		, _dirent(nullptr)
		, _last(nullptr), _internal(internal)
		, _start(false)
		{}

		void advance() {
			if ( is_abort() ) return;
			
			if ( _last->index < int(_last->dirents.length()) ) {
				_dirent = &_last->dirents[_last->index];
				
				if ( _internal ) { // 内部优先
					if ( _dirent->type == FTYPE_DIR && _last->mask == 0 ) { // 目录
						_last->mask = 1;
						into(_dirent->pathname);
					} else {
						_last->index++; //
						_last->mask = 0;
						_cb->resolve(this);
					}
				}
				else {
					if ( _dirent->type == FTYPE_DIR ) {
						if ( _last->mask == 0 ) {
							_last->mask = 1;
							//sync_callback(_cb, nullptr, this);
							_cb->resolve(this);
						} else {
							_last->index++;
							_last->mask = 0;
							into(_dirent->pathname);
						}
					} else {
						_last->index++;
						_last->mask = 0;
						_cb->resolve(this);
					}
				}
			} else { // end
				if ( _stack.length() > 1 ) {
					_stack.pop();
					_last = &_stack[_stack.length() - 1];
					advance();
				} else {
					Handle<AsyncEach> handle(this); // retain scope
					abort();
					async_callback(_end); // end exit
				}
			}
		}

		inline const Dirent& dirent() { return *_dirent; }

		uint32_t start() {
			if ( !_start ) {
				_start = true;
				file_stat(_path, Cb(&AsyncEach::start_cb, this), loop());
			}
			return id();
		}

	private:

		inline void into(cString& path) {
			ls(path, Callback<Array<Dirent>>(&AsyncEach::into_cb, this), loop());
		}

		void into_cb(CallbackData<Array<Dirent>>& evt) {
			if ( !is_abort() ) {
				if ( evt.error ) { // err
					abort();
					async_reject(_end, Error(*evt.error));
				} else {
					_stack.push({ std::move(*evt.data), 0, 0 });
					_last = &_stack[_stack.length() - 1];
					advance();
				}
			}
		}
		
		void start_cb(Cb::Data& evt) {
			if ( !is_abort() ) {
				if ( evt.error ) { // err
					abort();
					async_reject(_end, Error(*evt.error));
				} else {
					FileStat* stat = static_cast<FileStat*>(evt.data);
					Array<Dirent> dirents;
					dirents.push(Dirent{fs_basename(_path), _path, stat->type()});
					_stack.push( { std::move(dirents), 0, 0 } );
					_last = &_stack[_stack.length() - 1];
					advance();
				}
			}
		}
		
		struct DirentList {
			Array<Dirent> dirents;
			int index;
			int mask;
		};
		
	private:
		
		String _path;
		Cb _cb, _end;
		Array<DirentList> _stack;
		Dirent* _dirent;
		DirentList* _last;
		bool _internal;
		bool _start;
	};

	void fs_abort(uint32_t id) {
		AsyncIOTask::safe_abort(id);
	}

	void fs_chmod(cString& path, uint32_t mode, Cb cb) {
		chmod(path, mode, cb, LOOP);
	}

	uint32_t fs_chmod_recursion(cString& path, uint32_t mode, Cb cb) {
		// Qk_DLog("fs_chmod_recursion");
		auto each = NewRetain<AsyncEach>(path, Cb([mode, cb](Cb::Data& evt) {
			auto each = static_cast<AsyncEach*>(evt.data);
			each->retain(); // chmod2 回调前都保持each不被释放
			const Dirent& dirent = each->dirent();
			String pathname = dirent.pathname;
			chmod(dirent.pathname, mode, Cb([each, cb, pathname](Cb::Data& evt) {
				Handle<AsyncEach> handle(each);
				each->release();
				if ( !each->is_abort() ) {
					if ( evt.error ) {
						each->abort();
						cb->reject(evt.error);
					} else {
						each->advance();
					}
				}
			}), each->loop());
		}), cb, true);
		return each->start();
	}

	void fs_chown(cString& path, uint32_t owner, uint32_t group, Cb cb) {
		chown(path, owner, group, cb, LOOP);
	}

	uint32_t fs_chown_recursion(cString& path, uint32_t owner, uint32_t group, Cb cb) {
		auto each = NewRetain<AsyncEach>(path, Cb([owner, group, cb](Cb::Data& evt) {
			auto each = static_cast<AsyncEach*>(evt.data);
			each->retain();
			chown(each->dirent().pathname, owner, group, Cb([each, cb](Cb::Data& evt) {
				Handle<AsyncEach> handle(each); each->release();
				if ( !each->is_abort() ) {
					if ( evt.error ) {
						each->abort();
						cb->reject(evt.error);
					} else {
						each->advance();
					}
				}
			}), each->loop());
		}), cb, true);
		return each->start();
	}

	void fs_mkdir(cString& path, uint32_t mode, Cb cb) {
		mkdir(path, mode, cb, LOOP);
	}

	void fs_mkdirs(cString& path, uint32_t mode, Cb cb) {
		exists(path, Cb([=](Cb::Data& evt) {
			if ( static_cast<Bool*>(evt.data)->value ) { // ok
				async_callback(cb);
			} else {
				try {
					fs_mkdirs_sync(path, mode);
				} catch(Error& err) {
					cb->reject(&err);
					return;
				}
				async_callback(cb);
			}
		}), LOOP);
	}

	void fs_rename(cString& name, cString& new_name, Cb cb) {
		LOOP2;
		uv_fs_rename(loop->uv_loop(),
								New<FileReq>(cb, loop)->req(),
								fs_fallback_c(name),
								fs_fallback_c(new_name), &uv_fs_async_cb);
	}

	void fs_link(cString& path, cString& newPath, Cb cb) {
		link(path, newPath, cb, LOOP);
	}

	void fs_unlink(cString& path, Cb cb) {
		unlink(path, cb, LOOP);
	}

	void fs_rmdir(cString& path, Cb cb) {
		rmdir(path, cb, LOOP);
	}

	uint32_t fs_remove_recursion(cString& path, Cb cb) {
		auto each = NewRetain<AsyncEach>(path, Cb([cb](Cb::Data& evt) {
			auto each = static_cast<AsyncEach*>(evt.data);
			each->retain();

			Cb cb2([each, cb](Cb::Data& evt) {
				Handle<AsyncEach> handle(each); each->release();
				if ( !each->is_abort() ) {
					if ( evt.error ) {
						each->abort();
						cb->reject(evt.error);
					} else {
						each->advance();
					}
				}
			});
			if ( each->dirent().type == FTYPE_DIR ) {
				each->loop();
				rmdir(each->dirent().pathname, cb2, each->loop());
			} else {
				unlink(each->dirent().pathname, cb2, each->loop());
			}
		}), cb/*end*/, true);
		return each->start();
	}

	uint32_t fs_copy_recursion(cString& source, cString& target, Cb cb) {

		class Task: public AsyncEach {
		 public:
			Task(cString& source, cString& target, Cb cb)
			: AsyncEach(source, Cb(&each_cb, (Object*)this/*这里避免循环引用使用Object*/), cb, false)
			, _end(cb)
			, _s_len(fs_format("%s", *source).length())
			, _path(fs_format("%s", *target))
			, _copy_task(nullptr)
			{ //
				is_dir(fs_dirname(target), Cb([this](Cb::Data& ev) {
					if ( is_abort() ) return;
					if ( static_cast<Bool*>(ev.data)->value ) {
						start();
					} else {
						abort();
						async_reject(_end, Error(ERR_COPY_TARGET_DIRECTORY_NOT_EXISTS, "Copy target directory not exists."));
					}
				}, this), loop());
			}
			
			inline String target() {
				return _path + dirent().pathname.substr(_s_len); // 目标文件
			}
			
			static void each_cb(Cb::Data& d, Object* self) {
				Task* t = static_cast<Task*>(self);
				const Dirent& ent = t->dirent();
				
				switch (ent.type) {
					case FTYPE_DIR:
						exists(t->target(), Cb(&Task::is_directory_cb, t), t->loop()); break;
					case FTYPE_FILE:
						t->_copy_task = cp(ent.pathname, t->target(), Cb([t](Cb::Data& ev) {
							t->_copy_task = nullptr;
							if ( !t->is_abort() ) {
								if ( ev.error ) {
									t->error(ev);
								} else {
									t->advance();
								}
							}
						}, t), t->loop());
						break;
					default:
						t->advance(); break;
				}
			}
			
			void error(Cb::Data& ev) {
				abort();
				_end->reject(ev.error);
			}
			
			void is_directory_cb(Cb::Data& evt) {
				if ( is_abort() ) return;
				if ( evt.error ) {
					error(evt);
				} else {
					if ( static_cast<Bool*>(evt.data)->value ) {
						advance(); return;
					}
					/* create dir */
					mkdir(target(), fs_default_mode, Cb([this](Cb::Data& ev) {
						if ( !is_abort() ) {
							if ( ev.error ) {
								error(ev);
							} else {
								advance();
							}
						}
					}, this), loop());
				}
			}
			
			virtual void abort() {
				if ( _copy_task ) {
					_copy_task->abort();
					_copy_task = nullptr;
				}
				AsyncEach::abort();
			}
			
		 private:
			
			Cb _end;
			uint32_t _s_len;
			String _path;
			AsyncIOTask* _copy_task;
		};
		
		return NewRetain<Task>(source, target, cb)->id();
	}

	uint32_t fs_copy(cString& source, cString& target, Cb cb) {
		return cp(source, target, cb, LOOP)->id();
	}

	void fs_readdir(cString& path, Callback<Array<Dirent>> cb) {
		ls(path, cb, LOOP);
	}

	void fs_stat(cString& path, Callback<FileStat> cb) {
		file_stat(path, *reinterpret_cast<Cb*>(&cb), LOOP);
	}

	void fs_exists(cString& path, Callback<Bool> cb) {
		exists(path, *reinterpret_cast<Cb*>(&cb), LOOP);
	}

	void fs_is_file(cString& path, Callback<Bool> cb) {
		LOOP2;
		uv_fs_stat(loop->uv_loop(),
							New<FileReq>(*reinterpret_cast<Cb*>(&cb), loop)->req(),
							fs_fallback_c(path),
							&is_file_cb);
	}

	void fs_is_directory(cString& path, Callback<Bool> cb) {
		is_dir(path, *reinterpret_cast<Cb*>(&cb), LOOP);
	}

	void fs_readable(cString& path, Callback<Bool> cb) {
		LOOP2;
		uv_fs_access(loop->uv_loop(),
								New<FileReq>(*reinterpret_cast<Cb*>(&cb), loop)->req(),
								fs_fallback_c(path),
								R_OK, &uv_fs_access_cb);
	}

	void fs_writable(cString& path, Callback<Bool> cb) {
		LOOP2;
		uv_fs_access(loop->uv_loop(),
								New<FileReq>(*reinterpret_cast<Cb*>(&cb), loop)->req(),
								fs_fallback_c(path),
								W_OK, &uv_fs_access_cb);
	}

	void fs_executable(cString& path, Callback<Bool> cb) {
		LOOP2;
		uv_fs_access(loop->uv_loop(),
								New<FileReq>(*reinterpret_cast<Cb*>(&cb), loop)->req(),
								fs_fallback_c(path),
								X_OK, &uv_fs_access_cb);
	}

	// --------------- file read write ---------------

	// read stream

	uint32_t fs_read_stream(cString& path, Callback<StreamResponse> cb) {
		class Task;
		typedef UVRequestWrap<uv_fs_t, Task, Object, StreamResponse> FileReq;

		class Task: public AsyncIOTask, public Stream {
		public:
			String     _path;
			int64_t    _offset;
			int        _fd;
			Callback<StreamResponse> _cb;
			Buffer     _buffer;
			bool       _pause;
			int        _read_count;
			FileReq*   _req;
			int64_t    _total;
			int64_t    _size;
			
			Task(int64_t offset, Callback<StreamResponse> cb) {
				_offset = offset;
				_fd = 0;
				_cb = cb;
				_pause = false;
				_read_count = 0;
				_total = 0;
				_size = 0;
			}

			~Task() {
			}

			uv_loop_t* uv_loop() {
				return loop()->uv_loop();
			}

			void abort() {
				AsyncIOTask::abort();
			}

			virtual void pause() {
				_pause = true;
			}

			virtual void resume() {
				if ( _pause ) {
					_pause = false;
					read_advance(_req);
				}
			}

			static void fs_close_cb(uv_fs_t* uv_req) { // close
				Release(FileReq::cast(uv_req)); // release req
			}

			static void fs_read_cb(uv_fs_t* uv_req) { // read data result
				uv_fs_req_cleanup(uv_req);
				auto req = FileReq::cast(uv_req);
				Task* ctx = req->ctx();

				ctx->_read_count--;
				Qk_ASSERT(ctx->_read_count == 0);

				if ( uv_req->result < 0 ) { // error
					ctx->abort();
					async_reject(req->cb(), uv_error(req->req(), ctx->_path.c_str()));
					uv_fs_close(ctx->uv_loop(), uv_req, ctx->_fd, &fs_close_cb); // close
				} else {
					if ( uv_req->result ) {
						if ( ctx->_offset > -1 ) {
							ctx->_offset += uv_req->result;
						}
						ctx->_size += uv_req->result;
						ctx->_buffer.reset((uint32_t)uv_req->result);
						ctx->_cb->resolve(StreamResponse({ctx->_size, ctx->_total, ctx->_buffer, ctx, ctx->id(), false}));
						ctx->read_advance(req);
					} else { // end
						ctx->abort();
						Buffer buffer;
						ctx->_cb->resolve(StreamResponse({ctx->_size, ctx->_total, buffer, ctx, ctx->id(), true}));
						uv_fs_close(ctx->uv_loop(), uv_req, ctx->_fd, &fs_close_cb); // close
					}
				}
			}

			void read_advance(FileReq* req) {
				if ( is_abort() ) {
					uv_fs_close(uv_loop(), req->req(), _fd, &fs_close_cb); // close
				} else {
					if ( !_pause && _read_count == 0 ) {
						_read_count++;
						if ( _buffer.length() < BUFFER_SIZE ) {
							_buffer.reset(BUFFER_SIZE);
						}
						uv_buf_t buf;
						buf.base = *_buffer;
						buf.len = _buffer.length();
						uv_fs_read(uv_loop(), req->req(), _fd, &buf, 1, _offset, &fs_read_cb);
					}
				}
			}

			static void fs_fstat_cb(uv_fs_t* uv_req) { // read size
				uv_fs_req_cleanup(uv_req);
				FileReq* req = FileReq::cast(uv_req);
				if ( uv_req->result == 0 ) {
					req->ctx()->_total = uv_req->statbuf.st_size;
					if ( req->ctx()->_offset > 0 ) {
						req->ctx()->_total -= req->ctx()->_offset;
						req->ctx()->_total = Qk_Max(req->ctx()->_total, 0);
					}
					req->ctx()->read_advance(req);
				} else { // err
					req->ctx()->abort();
					async_err_callback(req->ctx()->_cb, uv_req, *req->ctx()->_path);
					uv_fs_close(req->ctx()->uv_loop(), uv_req, req->ctx()->_fd, &fs_close_cb); // close
				}
			}

			static void fs_open_cb(uv_fs_t* uv_req) { // open file
				uv_fs_req_cleanup(uv_req);
				auto req = FileReq::cast(uv_req);
				if ( uv_req->result > 0 ) {
					req->ctx()->_fd = (int)uv_req->result;
					uv_fs_fstat(req->ctx()->uv_loop(), uv_req, (uv_file)uv_req->result, &fs_fstat_cb);
				} else { // open file fail
					Handle<FileReq> handle(req); // release req
					req->ctx()->abort();
					async_err_callback(req->ctx()->_cb, uv_req, *req->ctx()->_path);
				}
			}

			static void start(FileReq* req) {
				uv_fs_open(req->ctx()->uv_loop(), req->req(),
									fs_fallback_c(req->ctx()->_path), O_RDONLY, 0, &fs_open_cb);
			}
		};
		
		auto task = NewRetain<Task>(-1, cb);
		auto req = new FileReq(task);
		task->_req = req;
		task->_path = path;
		Task::start(req);
		return task->id();
	}

	// read file

	void fs_read_file(cString& path, Callback<Buffer> cb, int64_t size) {
		int64_t offset = -1;
		struct Data;
		typedef AsyncReqNonCtx<uv_fs_t, Data> FileReq;

		struct Data {
			String path;
			int64_t  size;
			int64_t  offset;
			Buffer buff;
			int fd;

			static void fs_close_cb(uv_fs_t* uv_req) { // close
				Release(FileReq::cast(uv_req));
			}

			static void fs_fstat_cb(uv_fs_t* uv_req) { // read size
				uv_fs_req_cleanup(uv_req);
				FileReq* req = FileReq::cast(uv_req);
				if ( uv_req->result == 0 ) {
					req->data().size = uv_req->statbuf.st_size;
					start_read(req);
				} else { // err
					async_err_callback(req->cb(), uv_req, *req->data().path);
					uv_fs_close(req->uv_loop(), uv_req, req->data().fd, &fs_close_cb); // close
				}
			}

			static void fs_read_cb(uv_fs_t* uv_req) { // read data result
				uv_fs_req_cleanup(uv_req);
				FileReq* req = FileReq::cast(uv_req);
				if ( uv_req->result < 0 ) { // error
					async_err_callback(req->cb(), uv_req, *req->data().path);
				} else {
					char* buf = req->data().buff.collapse();
					buf[(uint32_t)uv_req->result] = '\0';
					req->data().buff = Buffer::from(buf, (uint32_t)uv_req->result);
					Buffer& buff = req->data().buff;
					async_resolve(req->cb(), buff);
				}
				uv_fs_close(req->uv_loop(), uv_req, req->data().fd, &fs_close_cb); // close
			}

			static void start_read(FileReq* req) {
				int64_t size = req->data().size;
				Char* buffer = (Char*)::malloc(size + 1); // 为兼容C字符串多加1位0
				if ( buffer ) {
					req->data().buff = Buffer::from(buffer, uint32_t(size));
					uv_buf_t buf;
					buf.base = buffer;
					buf.len = size;
					uv_fs_read(req->uv_loop(), req->req(),
										req->data().fd, &buf, 1, req->data().offset, &fs_read_cb);
				} else {
					async_reject(req->cb(), Error(ERR_ALLOCATE_MEMORY_FAIL, "allocate memory fail"));
					uv_fs_close(req->uv_loop(), req->req(), req->data().fd, &fs_close_cb); // close
				}
			}

			static void fs_open_cb(uv_fs_t* uv_req) { // open file
				uv_fs_req_cleanup(uv_req);
				FileReq* req = FileReq::cast(uv_req);
				if ( uv_req->result > 0 ) {
					req->data().fd = (int)uv_req->result;
					if ( req->data().size < 0 ) {
						uv_fs_fstat(req->uv_loop(), uv_req, (uv_file)uv_req->result, &fs_fstat_cb);
					} else {
						start_read(req);
					}
				} else { // open file fail
					Handle<FileReq> handle(req);
					async_err_callback(req->cb(), uv_req, *req->data().path);
				}
			}

			static void start(FileReq* req) {
				uv_fs_open(req->uv_loop(), req->req(),
					fs_fallback(req->data().path).c_str(), O_RDONLY, 0, &fs_open_cb);
			}
		};

		Data::start(new FileReq(*reinterpret_cast<Cb*>(&cb), LOOP, { path, size, offset }));
	}

	// write file

	void fs_write_file(cString& path, Buffer buffer, Callback<Buffer> cb) {
		struct Data;
		typedef AsyncReqNonCtx<uv_fs_t, Data> FileReq;
		
		struct Data {
			String  path;
			uint64_t  size;
			Buffer  buff;
			int fd;

			static void fs_close_cb(uv_fs_t* req) { // close
				Release(FileReq::cast(req));
			}

			static void fs_write_cb(uv_fs_t* uv_req) { // write cb
				uv_fs_req_cleanup(uv_req);
				FileReq* req = FileReq::cast(uv_req);
				Buffer& buff = req->data().buff;
				if ( uv_req->result < 0 ) {
					auto err = uv_error(uv_req, req->data().path.c_str());
					req->cb()->call(&err, &buff);
				} else {
					async_resolve(req->cb(), std::move(buff));
				}
				uv_fs_close(req->uv_loop(), uv_req, req->data().fd, &fs_close_cb); // close
			}

			static void fs_open_cb(uv_fs_t* uv_req) { // open file
				uv_fs_req_cleanup(uv_req);
				FileReq* req = FileReq::cast(uv_req);
				if ( uv_req->result > 0 ) { // open ok
					req->data().fd = (int)uv_req->result;
					uv_buf_t buf;
					buf.base = *req->data().buff;
					buf.len = req->data().size;
					uv_fs_write(req->uv_loop(), uv_req, (uv_file)uv_req->result, &buf, 1, -1, &fs_write_cb);
				} else { // open file fail
					Handle<FileReq> handle(req);
					auto err = uv_error(uv_req, req->data().path.c_str());
					req->cb()->call(&err, &req->data().buff);
				}
			}

			static void start(FileReq* req) {
				uv_fs_open(req->uv_loop(), req->req(),
									fs_fallback(req->data().path).c_str(),
									O_WRONLY | O_CREAT | O_TRUNC, fs_default_mode, &fs_open_cb);
			}
		};

		uint32_t size = buffer.length();
		Data::start(new FileReq(*reinterpret_cast<Cb*>(&cb), LOOP, Data({ path, size, buffer })));
	}

	void fs_write_file(cString& path, cString& str, Callback<Buffer> cb) {
		fs_write_file(path, str.copy().collapse(), cb);
	}

	// open/close file fd
	void fs_open(cString& path, int flag, Callback<Int32> cb) {
		struct Data;
		typedef AsyncReqNonCtx<uv_fs_t, Data> FileReq;

		struct Data {
			String path;
			int flag;

			static void fs_open_cb(uv_fs_t* uv_req) { // open file
				uv_fs_req_cleanup(uv_req);
				auto req = FileReq::cast(uv_req);
				Handle<FileReq> handle(req);
				if ( uv_req->result > 0 ) {
					int fd = (int)uv_req->result;
					async_resolve(req->cb(), Int32(fd));
				} else { // open file fail
					async_err_callback(req->cb(), uv_req, *req->data().path);
				}
			}

			static void start(FileReq* req) {
				Data& data = req->data();
				uv_fs_open(req->uv_loop(),
									req->req(),
									fs_fallback(data.path).c_str(),
									file_flag_mask(data.flag),
									fs_default_mode,
									&fs_open_cb);
			}
		};

		Data::start(new FileReq(*reinterpret_cast<Cb*>(&cb), LOOP, { path, flag }));
	}

	void fs_open(cString& path, Callback<Int32> cb) {
		fs_open(path, FOPEN_R, cb);
	}

	void fs_close(int fd, Cb cb) {
		struct Data;
		typedef AsyncReqNonCtx<uv_fs_t, Data> FileReq;

		struct Data {
			int fd;

			static void fs_close_cb(uv_fs_t* uv_req) { // close file
				uv_fs_req_cleanup(uv_req);
				auto req = FileReq::cast(uv_req);
				Handle<FileReq> handle(req);
				if ( uv_req->result == 0 ) {
					async_callback(req->cb());
				} else { // close file fail
					async_err_callback(req->cb(), uv_req);
				}
			}

			static void start(FileReq* req) {
				uv_fs_close(req->uv_loop(), req->req(), req->data().fd, &fs_close_cb); // close
			}
		};

		Data::start(new FileReq(cb, LOOP, { fd }));
	}

	// read with fd
	void fs_read(int fd, Buffer buffer, Callback<Buffer> cb) {
		fs_read(fd, buffer, -1, cb);
	}

	void fs_read(int fd, Buffer buffer, int64_t offset, Callback<Buffer> cb) {
		struct Data;
		typedef AsyncReqNonCtx<uv_fs_t, Data> FileReq;

		struct Data {
			int fd;
			int64_t  offset;
			Buffer buffer;

			static void fs_read_cb(uv_fs_t* uv_req) {
				uv_fs_req_cleanup(uv_req);
				auto req = FileReq::cast(uv_req);
				Handle<FileReq> handle(req);
				Buffer& buff = req->data().buffer;
				if ( uv_req->result < 0 ) { // error
					auto err = uv_error(uv_req);
					req->cb()->call(&err, &buff);
				} else {
					buff[uv_req->result] = '\0';
					auto capacity = buff.capacity();
					Buffer buff2(buff.collapse(), (uint32_t)uv_req->result, capacity);
					req->cb()->resolve(&buff2);
				}
			}

			static void start(FileReq* req) {
				Data& data = req->data();
				uv_buf_t buf;
				buf.base = *req->data().buffer;
				buf.len = req->data().buffer.length();
				uv_fs_read(req->uv_loop(), req->req(),
									data.fd, &buf, 1, data.offset, &fs_read_cb);
			}
		};
		
		Data::start(new FileReq(*reinterpret_cast<Cb*>(&cb), LOOP, { fd, offset, buffer }));
	}

	void fs_write(int fd, Buffer buffer, Callback<Buffer> cb) {
		fs_write(fd, buffer, -1, cb);
	}

	void fs_write(int fd, Buffer buffer, int64_t offset, Callback<Buffer> cb) {
		struct Data;
		typedef AsyncReqNonCtx<uv_fs_t, Data> FileReq;

		struct Data {
			int fd;
			int64_t  offset;
			Buffer buffer;

			static void fs_write_cb(uv_fs_t* uv_req) {
				uv_fs_req_cleanup(uv_req);
				auto req = FileReq::cast(uv_req);
				Handle<FileReq> handle(req);
				Buffer& buff = req->data().buffer;
				if ( uv_req->result < 0 ) {
					auto err = uv_error(uv_req);
					req->cb()->call(&err, &buff);
				} else {
					async_resolve(req->cb(), std::move(buff));
				}
			}

			static void start(FileReq* req) {
				Data& data = req->data();
				uv_buf_t buf;
				buf.base = *req->data().buffer;
				buf.len = req->data().buffer.length();
				uv_fs_write(req->uv_loop(), req->req(), data.fd, &buf, 1, data.offset, &fs_write_cb);
			}
		};

		Data::start(new FileReq(*reinterpret_cast<Cb*>(&cb), LOOP, { fd, offset, buffer }));
	}
}
