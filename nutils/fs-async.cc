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

#include "nutils/error.h"
#include "nutils/fs.h"

#if XX_WIN
	#include <io.h>
	#include <direct.h>
#else
	#include <unistd.h>
#endif
#include "nutils/uv-1.h"

XX_NS(ngui)

#define LOOP RunLoop::current()
#define LOOP2 RunLoop* loop = LOOP
#define BUFFER_SIZE (1024 * 16) // 16kb

extern void inl__set_file_stat(FileStat* stat, uv_stat_t* uv_stat);
extern int inl__file_flag_mask(int flag);

// --------------------------------- async -----------------------------------

template<class uv_req, class Data = Object>
class AsyncReqNonCtx: public UVRequestWrap<uv_req, Object, Data> {
public:
	AsyncReqNonCtx(cCb& cb, RunLoop* loop = LOOP, Data data = Data())
	: UVRequestWrap<uv_req, Object, Data>(nullptr, cb, move(data))
	, m_loop(loop)
	{ }
	static inline AsyncReqNonCtx* cast(uv_req* req) {
		return (AsyncReqNonCtx*)req->data;
	}
	inline PostMessage* loop() {
		return m_loop;
	}
	inline uv_loop_t* uv_loop() {
		return m_loop->uv_loop();
	}
private:
	RunLoop* m_loop;
};

typedef AsyncReqNonCtx<uv_fs_t, String> FileReq;

static Error uv_error(uv_fs_t* req, cchar* msg = nullptr) {
	return Error((int)req->result, "%s, %s, %s",
							 uv_err_name((int)req->result), uv_strerror((int)req->result), msg ? msg: "");
}

static void async_err_callback2(FileReq* req, cchar* msg = nullptr) {
	async_err_callback(req->cb(), uv_error(req->req(), msg));
}

static void async_err_callback2(cCb& cb, uv_fs_t* req, cchar* msg = nullptr) {
	async_err_callback(cb, uv_error(req, msg));
}

static void uv_fs_async_cb(uv_fs_t* req) {
	uv_fs_req_cleanup(req);
	Handle<FileReq> handle(FileReq::cast(req));
	if ( req->result == 0 ) { // ok
		async_callback(handle->cb());
	} else { // err
		async_err_callback2(*handle);
	}
}

static void uv_fs_access_cb(uv_fs_t* req) {
	uv_fs_req_cleanup(req);
	Handle<FileReq> handle(FileReq::cast(req));
	async_callback(handle->cb(), Bool(req->result == 0));
}

static void ls_cb(uv_fs_t* req) {
	Handle<FileReq> handle(FileReq::cast(req));
	Array<Dirent> ls;
	if ( req->result ) {
		String p =  handle->data() + '/'; // Path::format("%s", *handle->data()) + '/';
		uv_dirent_t ent;
		while ( uv_fs_scandir_next(req, &ent) == 0 ) {
			ls.push( Dirent(ent.name, p + ent.name, FileType(ent.type)) );
		}
	}
	uv_fs_req_cleanup(req);
	async_callback(handle->cb(), move(ls));
}

static void is_file_cb(uv_fs_t* req) {
	uv_fs_req_cleanup(req);
	Handle<FileReq> handle(FileReq::cast(req));
	if ( req->result == 0 ) { // ok
		async_callback(handle->cb(), Bool(!S_ISDIR(req->statbuf.st_mode)));
	} else { // err
		async_callback(handle->cb(), Bool(false));
	}
}

static void is_dir_cb(uv_fs_t* req) {
	uv_fs_req_cleanup(req);
	Handle<FileReq> handle(FileReq::cast(req));
	if ( req->result == 0 ) { // ok
		async_callback(handle->cb(), Bool(S_ISDIR(req->statbuf.st_mode)));
	} else { // err
		async_callback(handle->cb(), Bool(false));
	}
}

static void stat_cb(uv_fs_t* req) {
	uv_fs_req_cleanup(req);
	Handle<FileReq> handle(FileReq::cast(req));
	if ( req->result == 0 ) { // ok
		FileStat stat;
		inl__set_file_stat(&stat, &req->statbuf);
		async_callback(handle->cb(), move(stat));
	} else { // err
		async_err_callback2(*handle);
	}
}

static void exists2(cString& path, cCb& cb, RunLoop* loop) {
	uv_fs_access(loop->uv_loop(),
							 New<FileReq>(cb, loop)->req(),
							 Path::fallback_c(path), F_OK, &uv_fs_access_cb);
}

static void ls2(cString& path, cCb& cb, RunLoop* loop) {
	uv_fs_scandir(loop->uv_loop(),
								New<FileReq>(cb, loop, path)->req(), Path::fallback_c(path), 1, &ls_cb);
}

void file_helper_stat2(cString& path, cCb& cb, RunLoop* loop) {
	uv_fs_stat(loop->uv_loop(), New<FileReq>(cb, loop)->req(), Path::fallback_c(path), &stat_cb);
}

static void is_dir2(cString& path, cCb& cb, RunLoop* loop) {
	uv_fs_stat(loop->uv_loop(), New<FileReq>(cb, loop)->req(), Path::fallback_c(path), &is_dir_cb);
}

static AsyncIOTask* cp2(cString& source, cString& target, cCb& cb, RunLoop* loop) {
	
	class Task: public AsyncIOTask, AsyncFile::Delegate {
	public:
		
		Task(cString& source, cString& target, cCb& cb, RunLoop* loop)
		: AsyncIOTask(loop)
		, m_source_file(new AsyncFile(source, loop))
		, m_target_file(new AsyncFile(target, loop))
		, m_end(cb)
		, m_reading_count(0), m_writeing_count(0), m_read_end(false)
		{//
			m_buffer[0] = Buffer(BUFFER_SIZE);
			m_buffer[1] = Buffer(BUFFER_SIZE);
			m_source_file->set_delegate(this);
			m_target_file->set_delegate(this);
			m_source_file->open(FOPEN_R);
			m_target_file->open(FOPEN_W);
		}
		
		virtual ~Task() {
			release_file_handle();
		}
		
		void release_file_handle() {
			if ( m_source_file ) {
				Release(m_source_file); m_source_file = nullptr;
				Release(m_target_file); m_target_file = nullptr;
			}
		}
		
		Buffer alloc_buffer() {
			for ( int i = 0; i < 2; i++ ) {
				if (m_buffer[i].length()) {
					return m_buffer[i];
				}
			}
			return Buffer();
		}
		
		void release_buffer(Buffer buffer) {
			for ( int i = 0; i < 2; i++ ) {
				if (m_buffer[i].length() == 0) {
					m_buffer[i] = buffer.realloc(BUFFER_SIZE);
				}
			}
			XX_ASSERT(buffer.length() == 0);
		}
		
		void read_next() {
			XX_ASSERT(!m_read_end);
			Buffer buff = alloc_buffer();
			if ( buff.length() ) {
				m_reading_count++;
				m_source_file->read(buff, BUFFER_SIZE);
			}
		}
		
		virtual void abort() {
			release_file_handle();
			AsyncIOTask::abort();
		}
		
		virtual void trigger_async_file_error(AsyncFile* file, cError& error) {
			Handle<Task> handle(this); //
			abort();
			async_err_callback(m_end, Error(error));
		}
		
		virtual void trigger_async_file_open(AsyncFile* file) {
			if ( m_source_file->is_open() && m_target_file->is_open() ) {
				read_next();
			}
		}
		
		void copy_complete() {
			XX_ASSERT(m_reading_count == 0);
			XX_ASSERT(m_writeing_count == 0);
			XX_ASSERT(m_read_end);
			if ( !is_abort() ) { // copy complete
				//XX_DEBUG("-----copy_complete------");
				Handle<Task> handle(this);
				abort();
				async_callback(m_end);
			}
		}
		
		virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) {
			XX_ASSERT( file == m_source_file );
			XX_ASSERT( m_reading_count > 0 );
			m_reading_count--;
			if ( buffer.length() ) {
				m_writeing_count++;
				m_target_file->write(buffer, buffer.length());
				read_next();
			} else {
				XX_ASSERT(m_reading_count == 0);
				XX_ASSERT(!m_read_end);
				m_read_end = true;
				if ( m_writeing_count == 0 ) {
					copy_complete();
				}
			}
		}
		
		virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) {
			XX_ASSERT( file == m_target_file );
			XX_ASSERT( m_writeing_count > 0 );
			m_writeing_count--;
			release_buffer(buffer);
			if ( m_read_end ) {
				if ( m_writeing_count == 0 ) {
					copy_complete();
				}
			} else {
				if ( m_reading_count == 0 ) {
					read_next();
				}
			}
		}
		
		virtual void trigger_async_file_close(AsyncFile* file) { }
		
		AsyncFile* m_source_file;
		AsyncFile* m_target_file;
		Callback<>   m_end;
		Buffer     m_buffer[2];
		int        m_reading_count;
		int        m_writeing_count;
		bool       m_read_end;
	};
	
	return NewRetain<Task>(source, target, cb, loop);
}

static void mkdir2(cString& path, uint mode, cCb& cb, RunLoop* loop) {
	uv_fs_mkdir(loop->uv_loop(),
							New<FileReq>(cb, loop)->req(),
							Path::fallback_c(path), mode, &uv_fs_async_cb);
}

static void chmod2(cString& path, uint mode, cCb& cb, RunLoop* loop) {
	uv_fs_chmod(loop->uv_loop(),
							New<FileReq>(cb, loop)->req(),
							Path::fallback_c(path), mode, &uv_fs_async_cb);
}

static void chown2(cString& path, uint owner, uint group, cCb& cb, RunLoop* loop) {
	uv_fs_chown(loop->uv_loop(),
							New<FileReq>(cb, loop)->req(),
							Path::fallback_c(path), owner, group, &uv_fs_async_cb);
}

static void link2(cString& path, cString& newPath, cCb& cb, RunLoop* loop) {
	uv_fs_link(loop->uv_loop(),
						 New<FileReq>(cb, loop)->req(),
						 Path::fallback_c(path), Path::fallback_c(newPath), &uv_fs_async_cb);
}

static void unlink2(cString& path, cCb& cb, RunLoop* loop) {
	uv_fs_unlink(loop->uv_loop(),
							 New<FileReq>(cb, loop)->req(),
							 Path::fallback_c(path), &uv_fs_async_cb);
}

static void rmdir2(cString& path, cCb& cb, RunLoop* loop) {
	uv_fs_rmdir(loop->uv_loop(),
							New<FileReq>(cb, loop)->req(),
							Path::fallback_c(path), &uv_fs_async_cb);
}

/**
 * @class AsyncEach
 */
class AsyncEach: public AsyncIOTask {
 public:
	
	AsyncEach(cString& path, cCb& cb, cCb& end, bool internal = false)
	: m_path(Path::format(path))
	, m_cb(cb)
	, m_end(end)
	, m_dirent(nullptr)
	, m_last(nullptr), m_internal(internal)
	, m_start(false)
	{
	}
	
	void advance() {
		if ( is_abort() ) return;
		
		if ( m_last->index < int(m_last->dirents.length()) ) {
			m_dirent = &m_last->dirents[m_last->index];
			
			if ( m_internal ) { // 内部优先
				if ( m_dirent->type == FTYPE_DIR && m_last->mask == 0 ) { // 目录
					m_last->mask = 1;
					into(m_dirent->pathname);
				} else {
					m_last->index++; //
					m_last->mask = 0;
					sync_callback(m_cb, nullptr, this);
				}
			}
			else {
				if ( m_dirent->type == FTYPE_DIR ) {
					if ( m_last->mask == 0 ) {
						m_last->mask = 1;
						sync_callback(m_cb, nullptr, this);
					} else {
						m_last->index++;
						m_last->mask = 0;
						into(m_dirent->pathname);
					}
				} else {
					m_last->index++;
					m_last->mask = 0;
					sync_callback(m_cb, nullptr, this);
				}
			}
		} else { // end
			if ( m_stack.length() > 1 ) {
				m_stack.pop();
				m_last = &m_stack[m_stack.length() - 1];
				advance();
			} else {
				Handle<AsyncEach> handle(this); // retain scope
				abort();
				async_callback(m_end); // end exit
			}
		}
	}
	
	inline const Dirent& dirent() { return *m_dirent; }
	
	uint start() {
		if ( !m_start ) {
			m_start = true;
			file_helper_stat2(m_path, Cb(&AsyncEach::start_cb, this), loop());
		}
		return id();
	}
	
 private:
	
	inline void into(cString& path) {
		ls2(path, Cb(&AsyncEach::into_cb, this), nullptr);
	}
	
	void into_cb(Cbd& evt) {
		if ( !is_abort() ) {
			if ( evt.error ) { // err
				abort();
				async_err_callback(m_end, Error(*evt.error));
			} else {
				m_stack.push( { move(*static_cast<Array<Dirent>*>(evt.data)), 0, 0 } );
				m_last = &m_stack[m_stack.length() - 1];
				advance();
			}
		}
	}
	
	void start_cb(Cbd& evt) {
		if ( !is_abort() ) {
			if ( evt.error ) { // err
				abort();
				async_err_callback(m_end, Error(*evt.error));
			} else {
				FileStat* stat = static_cast<FileStat*>(evt.data);
				Array<Dirent> dirents;
				dirents.push(Dirent(Path::basename(m_path), m_path, stat->type()));
				m_stack.push( { move(dirents), 0, 0 } );
				m_last = &m_stack[m_stack.length() - 1];
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
	
	String m_path;
	Callback<> m_cb;
	Callback<> m_end;
	Array<DirentList> m_stack;
	Dirent* m_dirent;
	DirentList* m_last;
	bool m_internal;
	bool m_start;
};

void FileHelper::abort(uint id) {
	AsyncIOTask::safe_abort(id);
}

void FileHelper::chmod(cString& path, uint mode, cCb& cb) {
	chmod2(path, mode, cb, LOOP);
}

uint FileHelper::chmod_r(cString& path, uint mode, cCb& cb) {
	auto each = NewRetain<AsyncEach>(path, Cb([mode, cb](Cbd& evt) {
		auto each = static_cast<AsyncEach*>(evt.data);
		each->retain(); // chmod2 回调前都保持each不被释放
		const Dirent& dirent = each->dirent();
		String pathname = dirent.pathname;
		chmod2(dirent.pathname, mode, Cb([each, cb, pathname](Cbd& evt) {
			Handle<AsyncEach> handle(each); each->release();
			if ( !each->is_abort() ) {
				if ( evt.error ) {
					each->abort();
					sync_callback(cb, evt.error);
				} else {
					each->advance();
				}
			}
		}), each->loop());
	}), cb, true);
	return each->start();
}

void FileHelper::chown(cString& path, uint owner, uint group, cCb& cb) {
	chown2(path, owner, group, cb, LOOP);
}

uint FileHelper::chown_r(cString& path, uint owner, uint group, cCb& cb) {
	auto each = NewRetain<AsyncEach>(path, Cb([owner, group, cb](Cbd& evt) {
		auto each = static_cast<AsyncEach*>(evt.data);
		each->retain();
		chown2(each->dirent().pathname, owner, group, Cb([each, cb](Cbd& evt) {
			Handle<AsyncEach> handle(each); each->release();
			if ( !each->is_abort() ) {
				if ( evt.error ) {
					each->abort();
					sync_callback(cb, evt.error);
				} else {
					each->advance();
				}
			}
		}), each->loop());
	}), cb, true);
	return each->start();
}

void FileHelper::mkdir(cString& path, uint mode, cCb& cb) {
	mkdir2(path, mode, cb, LOOP);
}

void FileHelper::mkdir_p(cString& path, uint mode, cCb& cb) {
	exists2(path, Cb([=](Cbd& evt) {
		if ( static_cast<Bool*>(evt.data)->value ) { // ok
			sync_callback(cb);
		} else {
			try {
				mkdir_p_sync(path, mode);
			} catch(cError& err) {
				sync_callback(cb, &err);
				return;
			}
			sync_callback(cb);
		}
	}), LOOP);
}

void FileHelper::rename(cString& name, cString& new_name, cCb& cb) {
	LOOP2;
	uv_fs_rename(loop->uv_loop(),
							 New<FileReq>(cb, loop)->req(),
							 Path::fallback_c(name),
							 Path::fallback_c(new_name), &uv_fs_async_cb);
}

void FileHelper::link(cString& path, cString& newPath, cCb& cb) {
	link2(path, newPath, cb, LOOP);
}

void FileHelper::unlink(cString& path, cCb& cb) {
	unlink2(path, cb, LOOP);
}

void FileHelper::rmdir(cString& path, cCb& cb) {
	rmdir2(path, cb, LOOP);
}

uint FileHelper::remove_r(cString& path, cCb& cb) {
	auto each = NewRetain<AsyncEach>(path, Cb([cb](Cbd& evt) {
		auto each = static_cast<AsyncEach*>(evt.data);
		each->retain();

		Cb cb2([each, cb](Cbd& evt) {
			Handle<AsyncEach> handle(each); each->release();
			if ( !each->is_abort() ) {
				if ( evt.error ) {
					each->abort();
					sync_callback(cb, evt.error);
				} else {
					each->advance();
				}
			}
		});
		if ( each->dirent().type == FTYPE_DIR ) {
			each->loop();
			rmdir2(each->dirent().pathname, cb2, each->loop());
		} else {
			unlink2(each->dirent().pathname, cb2, each->loop());
		}
	}), cb, true);
	return each->start();
}

uint FileHelper::copy(cString& source, cString& target, cCb& cb) {
	return cp2(source, target, cb, LOOP)->id();
}

uint FileHelper::copy_r(cString& source, cString& target, cCb& cb) {
	
	class Task: public AsyncEach {
	public:
		Task(cString& source, cString& target, cCb& cb)
		: AsyncEach(source, Cb(&each_cb, (Object*)this/*这里避免循环引用使用Object*/), cb, false)
		, m_end(cb)
		, m_s_len(Path::format("%s", *source).length())
		, m_path(Path::format("%s", *target))
		, m_copy_task(nullptr)
		{ //
			is_dir2(Path::dirname(target), Cb([this](Cbd& ev) {
				if ( is_abort() ) return;
				if ( static_cast<Bool*>(ev.data)->value ) {
					start();
				} else {
					abort();
					Error err(ERR_COPY_TARGET_DIRECTORY_NOT_EXISTS, "Copy target directory not exists.");
					async_err_callback(m_end, move(err));
				}
			}, this), loop());
		}
		
		inline String target() {
			return m_path + dirent().pathname.substr(m_s_len); // 目标文件
		}
		
		static void each_cb(Cbd& d, Object* self) {
			Task* t = static_cast<Task*>(self);
			const Dirent& ent = t->dirent();
			
			switch (ent.type) {
				case FTYPE_DIR:
					exists2(t->target(), Cb(&Task::is_directory_cb, t), t->loop()); break;
				case FTYPE_FILE:
					t->m_copy_task = cp2(ent.pathname, t->target(), Cb([t](Cbd& ev) {
						t->m_copy_task = nullptr;
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
		
		void error(Cbd& ev) {
			abort();
			async_err_callback(m_end, Error(*ev.error));
		}
		
		void is_directory_cb(Cbd& evt) {
			if ( is_abort() ) return;
			if ( evt.error ) {
				error(evt);
			} else {
				if ( static_cast<Bool*>(evt.data)->value ) {
					advance(); return;
				}
				/* create dir */
				mkdir2(target(), default_mode, Cb([this](Cbd& ev) {
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
			if ( m_copy_task ) {
				m_copy_task->abort();
				m_copy_task = nullptr;
			}
			AsyncEach::abort();
		}
		
	private:
		
		Callback<> m_end;
		uint   m_s_len;
		String m_path;
		AsyncIOTask* m_copy_task;
	};
	
	return NewRetain<Task>(source, target, cb)->id();
}

void FileHelper::readdir(cString& path, cCb& cb) {
	ls2(path, cb, LOOP);
}

void FileHelper::stat(cString& path, cCb& cb) {
	file_helper_stat2(path, cb, LOOP);
}

void FileHelper::exists(cString& path, cCb& cb) {
	exists2(path, cb, LOOP);
}

void FileHelper::is_file(cString& path, cCb& cb) {
	LOOP2;
	uv_fs_stat(loop->uv_loop(),
						 New<FileReq>(cb, loop)->req(),
						 Path::fallback_c(path),
						 &is_file_cb);
}

void FileHelper::is_directory(cString& path, cCb& cb) {
	is_dir2(path, cb, LOOP);
}

void FileHelper::readable(cString& path, cCb& cb) {
	LOOP2;
	uv_fs_access(loop->uv_loop(),
							 New<FileReq>(cb, loop)->req(),
							 Path::fallback_c(path),
							 R_OK, &uv_fs_access_cb);
}

void FileHelper::writable(cString& path, cCb& cb) {
	LOOP2;
	uv_fs_access(loop->uv_loop(),
							 New<FileReq>(cb, loop)->req(),
							 Path::fallback_c(path),
							 W_OK, &uv_fs_access_cb);
}

void FileHelper::executable(cString& path, cCb& cb) {
	LOOP2;
	uv_fs_access(loop->uv_loop(),
							 New<FileReq>(cb, loop)->req(),
							 Path::fallback_c(path),
							 X_OK, &uv_fs_access_cb);
}

// --------------- file read write ---------------

// read stream

uint FileHelper::read_stream(cString& path, cCb& cb) {
	class Task;
	typedef UVRequestWrap<uv_fs_t, Task> FileReq;
	
	class Task: public AsyncIOTask, public SimpleStream {
	 public:
		String     m_path;
		int64      m_offset;
		int        m_fd;
		Callback<> m_cb;
		Buffer     m_buffer;
		bool       m_pause;
		int        m_read_count;
		FileReq*   m_req;
		int64      m_total;
		int64      m_size;
		
		Task(int64 offset, cCb& cb) {
			m_offset = offset;
			m_cb = cb;
			m_pause = false;
			m_read_count = 0;
			m_total = 0;
			m_size = 0;
		}
		
		~Task() { }
		
		uv_loop_t* uv_loop() {
			return loop()->uv_loop();
		}
		
		void abort() {
			AsyncIOTask::abort();
		}
		
		virtual void pause() {
			m_pause = true;
		}
		
		virtual void resume() {
			if ( m_pause ) {
				m_pause = false;
				read_advance(m_req);
			}
		}
		
		static void fs_close_cb(uv_fs_t* req) { // close
			Release(FileReq::cast(req)); // release req
		}
		
		static void fs_read_cb(uv_fs_t* uv_req) { // read data result
			uv_fs_req_cleanup(uv_req);
			auto req = FileReq::cast(uv_req);
			Task* ctx = req->ctx();
			
			ctx->m_read_count--;
			XX_ASSERT(ctx->m_read_count == 0);
			
			if ( uv_req->result < 0 ) { // error
				ctx->abort();
				async_err_callback2(req->cb(), uv_req, *ctx->m_path);
				uv_fs_close(ctx->uv_loop(), uv_req, ctx->m_fd, &fs_close_cb); // close
			} else {
				if ( uv_req->result ) {
					if ( ctx->m_offset > -1 ) {
						ctx->m_offset += uv_req->result;
					}
					ctx->m_size += uv_req->result;
					IOStreamData data(ctx->m_buffer.realloc((uint)uv_req->result),
														0, ctx->id(), ctx->m_size, ctx->m_total, ctx);
					async_callback(ctx->m_cb, move(data));
					ctx->read_advance(req);
				} else { // end
					ctx->abort();
					IOStreamData data(Buffer(), 1, ctx->id(), ctx->m_size, ctx->m_total, ctx);
					async_callback(ctx->m_cb, move(data));
					uv_fs_close(ctx->uv_loop(), uv_req, ctx->m_fd, &fs_close_cb); // close
				}
			}
		}
		
		void read_advance(FileReq* req) {
			if ( is_abort() ) {
				uv_fs_close(uv_loop(), req->req(), m_fd, &fs_close_cb); // close
			} else {
				if ( !m_pause && m_read_count == 0 ) {
					m_read_count++;
					
					if ( !m_buffer.length() ) {
						m_buffer = Buffer(BUFFER_SIZE);
					}
					uv_buf_t buf;
					buf.base = m_buffer.value();
					buf.len = m_buffer.length();
					uv_fs_read(uv_loop(), req->req(), m_fd, &buf, 1, m_offset, &fs_read_cb);
				}
			}
		}
		
		static void fs_fstat_cb(uv_fs_t* uv_req) { // read size
			uv_fs_req_cleanup(uv_req);
			FileReq* req = FileReq::cast(uv_req);
			if ( uv_req->result == 0 ) {
				req->ctx()->m_total = uv_req->statbuf.st_size;
				if ( req->ctx()->m_offset > 0 ) {
					req->ctx()->m_total -= req->ctx()->m_offset;
					req->ctx()->m_total = XX_MAX(req->ctx()->m_total, 0);
				}
				req->ctx()->read_advance(req);
			} else { // err
				req->ctx()->abort();
				async_err_callback2(req->ctx()->m_cb, uv_req, *req->ctx()->m_path);
				uv_fs_close(req->ctx()->uv_loop(), uv_req, req->ctx()->m_fd, &fs_close_cb); // close
			}
		}
		
		static void fs_open_cb(uv_fs_t* uv_req) { // open file
			uv_fs_req_cleanup(uv_req);
			auto req = FileReq::cast(uv_req);
			if ( uv_req->result > 0 ) {
				req->ctx()->m_fd = (int)uv_req->result;
				uv_fs_fstat(req->ctx()->uv_loop(), uv_req, (uv_file)uv_req->result, &fs_fstat_cb);
			} else { // open file fail
				Handle<FileReq> handle(req);
				req->ctx()->abort();
				async_err_callback2(req->ctx()->m_cb, uv_req, *req->ctx()->m_path);
			}
		}
		
		static void start(FileReq* req) {
			uv_fs_open(req->ctx()->uv_loop(), req->req(),
								 Path::fallback_c(req->ctx()->m_path), O_RDONLY, 0, &fs_open_cb);
		}
		
	};
	
	auto task = NewRetain<Task>(-1, cb);
	auto req = new FileReq(task);
	task->m_req = req;
	task->m_path = path;
	Task::start(req);
	return task->id();
}

// read file

void FileHelper::read_file(cString& path, cCb& cb, int64 size) {
	int64 offset = -1;
	struct Data;
	typedef AsyncReqNonCtx<uv_fs_t, Data> FileReq;
	
	struct Data {
		String path;
		int64  size;
		int64  offset;
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
				async_err_callback2(req->cb(), uv_req, *req->data().path);
				uv_fs_close(req->uv_loop(), uv_req, req->data().fd, &fs_close_cb); // close
			}
		}
		
		static void fs_read_cb(uv_fs_t* uv_req) { // read data result
			uv_fs_req_cleanup(uv_req);
			FileReq* req = FileReq::cast(uv_req);
			if ( uv_req->result < 0 ) { // error
				async_err_callback2(req->cb(), uv_req, *req->data().path);
			} else {
				Buffer& buff = req->data().buff;
				buff.value()[uv_req->result] = '\0';
				async_callback(req->cb(), buff.realloc((uint)uv_req->result));
			}
			uv_fs_close(req->uv_loop(), uv_req, req->data().fd, &fs_close_cb); // close
		}
		
		static void start_read(FileReq* req) {
			int64 size = req->data().size;
			char* buffer = (char*)::malloc(size + 1); // 为兼容C字符串多加1位0
			if ( buffer ) {
				req->data().buff = Buffer(buffer, uint(size));
				uv_buf_t buf;
				buf.base = buffer;
				buf.len = size;
				uv_fs_read(req->uv_loop(), req->req(),
									 req->data().fd, &buf, 1, req->data().offset, &fs_read_cb);
			} else {
				async_err_callback(req->cb(), Error(ERR_ALLOCATE_MEMORY_FAIL, "allocate memory fail"));
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
				async_err_callback2(req->cb(), uv_req, *req->data().path);
			}
		}
		
		static void start(FileReq* req) {
			uv_fs_open(req->uv_loop(), req->req(),
								 Path::fallback_c(req->data().path), O_RDONLY, 0, &fs_open_cb);
		}
		
	};
	
	Data::start(new FileReq(cb, LOOP, { path, size, offset }));
}

// write file

void FileHelper::write_file(cString& path, Buffer buffer, cCb& cb) {
	struct Data;
	typedef AsyncReqNonCtx<uv_fs_t, Data> FileReq;
	
	struct Data {
		String  path;
		uint64  size;
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
				async_callback(req->cb(), uv_error(uv_req, *req->data().path), move(buff));
			} else {
				async_callback(req->cb(), move(buff));
			}
			uv_fs_close(req->uv_loop(), uv_req, req->data().fd, &fs_close_cb); // close
		}
		
		static void fs_open_cb(uv_fs_t* uv_req) { // open file
			uv_fs_req_cleanup(uv_req);
			FileReq* req = FileReq::cast(uv_req);
			if ( uv_req->result > 0 ) { // open ok
				req->data().fd = (int)uv_req->result;
				uv_buf_t buf;
				buf.base = req->data().buff.value();
				buf.len = req->data().size;
				uv_fs_write(req->uv_loop(), uv_req, (uv_file)uv_req->result, &buf, 1, -1, &fs_write_cb);
			} else { // open file fail
				Handle<FileReq> handle(req);
				async_callback(req->cb(), uv_error(uv_req, *req->data().path), move(req->data().buff));
			}
		}
		
		static void start(FileReq* req) {
			uv_fs_open(req->uv_loop(), req->req(),
								 Path::fallback_c(req->data().path),
								 O_WRONLY | O_CREAT | O_TRUNC, default_mode, &fs_open_cb);
		}
		
	};
	
	uint size = buffer.length();
	Data::start(new FileReq(cb, LOOP, Data({ path, size, buffer })));
}

void FileHelper::write_file(cString& path, cString& str, cCb& cb) {
	write_file(path, String(str).collapse_buffer(), cb);
}

// open/close file fd
void FileHelper::open(cString& path, int flag, cCb& cb) {
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
				async_callback(req->cb(), Int(fd));
			} else { // open file fail
				async_err_callback2(req->cb(), uv_req, *req->data().path);
			}
		}
		
		static void start(FileReq* req) {
			Data& data = req->data();
			uv_fs_open(req->uv_loop(),
								 req->req(),
								 Path::fallback_c(data.path),
								 inl__file_flag_mask(data.flag),
								 default_mode,
								 &fs_open_cb);
		}
	};
	
	Data::start(new FileReq(cb, LOOP, { path, flag }));
}

void FileHelper::open(cString& path, cCb& cb) {
	open(path, FOPEN_R, cb);
}

void FileHelper::close(int fd, cCb& cb) {
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
				async_err_callback2(req->cb(), uv_req);
			}
		}
		
		static void start(FileReq* req) {
			uv_fs_close(req->uv_loop(), req->req(), req->data().fd, &fs_close_cb); // close
		}
	};
	
	Data::start(new FileReq(cb, LOOP, { fd }));
}

// read with fd
void FileHelper::read(int fd, Buffer buffer, cCb& cb) {
	read(fd, buffer, -1, cb);
}

void FileHelper::read(int fd, Buffer buffer, int64 offset, cCb& cb) {
	struct Data;
	typedef AsyncReqNonCtx<uv_fs_t, Data> FileReq;
	
	struct Data {
		int fd;
		int64  offset;
		Buffer buffer;
		
		static void fs_read_cb(uv_fs_t* uv_req) {
			uv_fs_req_cleanup(uv_req);
			auto req = FileReq::cast(uv_req);
			Handle<FileReq> handle(req);
			Buffer& buff = req->data().buffer;
			if ( uv_req->result < 0 ) { // error
				async_callback(req->cb(), uv_error(uv_req), move(buff));
			} else {
				buff.value()[uv_req->result] = '\0';
				async_callback(req->cb(), buff.realloc((uint)uv_req->result));
			}
		}
		
		static void start(FileReq* req) {
			Data& data = req->data();
			uv_buf_t buf;
			buf.base = req->data().buffer.value();
			buf.len = req->data().buffer.length();
			uv_fs_read(req->uv_loop(), req->req(),
								 data.fd, &buf, 1, data.offset, &fs_read_cb);
		}
	};
	
	Data::start(new FileReq(cb, LOOP, { fd, offset, buffer }));
}

void FileHelper::write(int fd, Buffer buffer, cCb& cb) {
	write(fd, buffer, -1, cb);
}

void FileHelper::write(int fd, Buffer buffer, int64 offset, cCb& cb) {
	struct Data;
	typedef AsyncReqNonCtx<uv_fs_t, Data> FileReq;
	
	struct Data {
		int fd;
		int64  offset;
		Buffer buffer;
		
		static void fs_write_cb(uv_fs_t* uv_req) {
			uv_fs_req_cleanup(uv_req);
			auto req = FileReq::cast(uv_req);
			Handle<FileReq> handle(req);
			Buffer& buff = req->data().buffer;
			if ( uv_req->result < 0 ) {
				async_callback(req->cb(), uv_error(uv_req), move(buff));
			} else {
				async_callback(req->cb(), move(buff));
			}
		}
		
		static void start(FileReq* req) {
			Data& data = req->data();
			uv_buf_t buf;
			buf.base = req->data().buffer.value();
			buf.len = req->data().buffer.length();
			uv_fs_write(req->uv_loop(), req->req(), data.fd, &buf, 1, data.offset, &fs_write_cb);
		}
	};
	
	Data::start(new FileReq(cb, LOOP, { fd, offset, buffer }));
}

XX_END
