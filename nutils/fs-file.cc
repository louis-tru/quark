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
#include "nutils/uv-1.h"

XX_NS(ngui)

XX_DEFINE_INLINE_MEMBERS(FileStat, Inl) {
public:
	void set_m_stat(uv_stat_t* stat) {
		if ( !m_stat ) {
			m_stat = malloc(sizeof(uv_stat_t));
		}
		memcpy(m_stat, stat, sizeof(uv_stat_t));
	}
};

void inl__set_file_stat(FileStat* stat, uv_stat_t* uv_stat) {
	Inl_FileStat(stat)->set_m_stat(uv_stat);
}

FileStat::FileStat(): m_stat(nullptr) { }
FileStat::FileStat(cString& path): FileStat(FileHelper::stat_sync(path)) { }
FileStat::FileStat(FileStat&& stat): m_stat(stat.m_stat) {
	stat.m_stat = nullptr;
}

FileStat& FileStat::operator=(FileStat&& stat) {
	if ( m_stat ) {
		free(m_stat);
	}
	m_stat = stat.m_stat;
	stat.m_stat = nullptr;
	return *this;
}

FileStat::~FileStat() {
	free(m_stat);
}

uv_stat_t stat;

#define STAT (m_stat ? (uv_stat_t*)m_stat : &stat)

bool FileStat::is_valid() const { return m_stat; }
bool FileStat::is_file() const { return S_ISREG(STAT->st_mode); }
bool FileStat::is_dir() const { return S_ISDIR(STAT->st_mode); }
bool FileStat::is_link() const { return S_ISLNK(STAT->st_mode); }
bool FileStat::is_sock() const { return S_ISSOCK(STAT->st_mode); }
uint64 FileStat::mode() const { return STAT->st_mode; }

FileType FileStat::type() const {
	if ( m_stat ) {
		if ( S_ISREG(((uv_stat_t*)m_stat)->st_mode) ) return FTYPE_FILE;
		if ( S_ISDIR(((uv_stat_t*)m_stat)->st_mode) ) return FTYPE_DIR;
		if ( S_ISLNK(((uv_stat_t*)m_stat)->st_mode) ) return FTYPE_LINK;
		if ( S_ISFIFO(((uv_stat_t*)m_stat)->st_mode) ) return FTYPE_FIFO;
		if ( S_ISSOCK(((uv_stat_t*)m_stat)->st_mode) ) return FTYPE_SOCKET;
		if ( S_ISCHR(((uv_stat_t*)m_stat)->st_mode) ) return FTYPE_CHAR;
		if ( S_ISBLK(((uv_stat_t*)m_stat)->st_mode) ) return FTYPE_BLOCK;
	}
	return FTYPE_UNKNOWN;
}

uint64 FileStat::group() const { return STAT->st_gid; }
uint64 FileStat::owner() const { return STAT->st_uid; }
uint64 FileStat::nlink() const { return STAT->st_nlink; }
uint64 FileStat::ino() const { return STAT->st_ino; }
uint64 FileStat::blksize() const { return STAT->st_blksize; }
uint64 FileStat::blocks() const { return STAT->st_blocks; }
uint64 FileStat::flags() const { return STAT->st_flags; }
uint64 FileStat::gen() const { return STAT->st_gen; }
uint64 FileStat::dev() const { return STAT->st_dev; }
uint64 FileStat::rdev() const { return STAT->st_rdev; }
uint64 FileStat::size() const { return STAT->st_size; }
uint64 FileStat::atime() const {
	return (int64)STAT->st_atim.tv_sec * 1000000 + STAT->st_atim.tv_nsec / 1000;
}
uint64 FileStat::mtime() const {
	return (int64)STAT->st_mtim.tv_sec * 1000000 + STAT->st_mtim.tv_nsec / 1000;
}
uint64 FileStat::ctime() const {
	return (int64)STAT->st_ctim.tv_sec * 1000000 + STAT->st_ctim.tv_nsec / 1000;
}
uint64 FileStat::birthtime() const {
	return (int64)STAT->st_birthtim.tv_sec * 1000000 + STAT->st_birthtim.tv_nsec / 1000;
}

#undef STAT

/**
 * @func get C file flga
 */
int inl__file_flag_mask(int flag) {
#if XX_POSIX || XX_UNIX
	return flag;
#else 
	int r_flag = flag & ~(O_ACCMODE | O_WRONLY | O_RDWR | 
		O_CREAT | O_EXCL | O_NOCTTY | O_TRUNC | O_APPEND | O_NONBLOCK);
	if (FOPEN_ACCMODE & flag) r_flag =| O_ACCMODE;
	if (FOPEN_WRONLY & flag) r_flag =| O_WRONLY;
	if (FOPEN_RDWR & flag) r_flag =| O_RDWR;
	if (FOPEN_CREAT & flag) r_flag =| O_CREAT;
	if (FOPEN_EXCL & flag) r_flag =| O_EXCL;
	if (FOPEN_NOCTTY & flag) r_flag =| O_NOCTTY;
	if (FOPEN_TRUNC & flag) r_flag =| O_TRUNC;
	if (FOPEN_APPEND & flag) r_flag =| O_APPEND;
	if (FOPEN_NONBLOCK & flag) r_flag =| O_NONBLOCK;
	return r_flag;
#endif 
}

cchar* inl__file_flag_str(int flag) {
	switch (flag) {
		default:
		case FOPEN_R: return "r";
		case FOPEN_W: return "w";
		case FOPEN_A: return "a";
		case FOPEN_RP: return "r+";
		case FOPEN_WP: return "w+";
		case FOPEN_AP: return "a+";
	}
}

File::~File() {
	close();
}

bool File::is_open() {
	return m_fp;
}

int File::open(int flag) {
	if ( m_fp ) { // 文件已经打开
		XX_WARN( "file already open" );
		return 0;
	}
	uv_fs_t req;
	m_fp = uv_fs_open(uv_default_loop(), &req,
										Path::fallback_c(m_path),
										inl__file_flag_mask(flag), FileHelper::default_mode, nullptr);
	if ( m_fp > 0 ) {
		return 0;
	}
	return m_fp;
}

int File::close() {
	if ( !m_fp ) return 0;
	uv_fs_t req;
	int r = uv_fs_close(uv_default_loop(), &req, m_fp, nullptr);
	if ( r == 0 ) {
		m_fp = 0;
	}
	return r;
}

int File::read(void* buffer, int64 size, int64 offset) {
	uv_fs_t req;
	uv_buf_t buf;
	buf.base = (char*)buffer;
	buf.len = size;
	return uv_fs_read(uv_default_loop(), &req, m_fp, &buf, 1, offset, nullptr);
}

int File::write(const void* buffer, int64 size, int64 offset) {
	uv_fs_t req;
	uv_buf_t buf;
	buf.base = (char*)buffer;
	buf.len = size;
	return uv_fs_write(uv_default_loop(), &req, m_fp, &buf, 1, offset, nullptr);
}

// ----------------------------------- FileAsync -----------------------------------------

struct FileStreamData {
	Buffer buffer;
	int64 offset;
	int mark;
};

typedef UVRequestWrap<uv_fs_t, AsyncFile::Inl> FileReq;
typedef UVRequestWrap<uv_fs_t, AsyncFile::Inl, FileStreamData> FileStreamReq;

class AsyncFile::Inl: public Reference, public AsyncFile::Delegate {
 public:
	typedef AsyncFile::Delegate Delegate;
	virtual void trigger_async_file_open(AsyncFile* file) { }
	virtual void trigger_async_file_close(AsyncFile* file) { }
	virtual void trigger_async_file_error(AsyncFile* file, cError& error) { }
	virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) { }
	virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) { }

	void set_delegate(Delegate* delegate) {
		if ( delegate ) {
			m_delegate = delegate;
		} else {
			m_delegate = this;
		}
	}
	
	Inl(AsyncFile* host, cString& path, RunLoop* loop)
	: m_path(path)
	, m_fp(0)
	, m_opening(false)
	, m_keep(loop->keep_alive("AsyncFile::Inl", false))
	, m_delegate(nullptr)
	, m_host(host)
	{
		XX_CHECK(m_keep);
	}
	
	virtual ~Inl() {
		if ( m_fp ) {
			uv_fs_t req;
			int res = uv_fs_close(m_keep->host()->uv_loop(), &req, m_fp, nullptr); // sync
			XX_ASSERT( res == 0 );
		}
		Release(m_keep); m_keep = nullptr;
		clear_writeing();
	}
	
	inline String& path() { return m_path; }
	inline bool is_open() { return m_fp; }
	
	void open(int flag) {
		if (m_fp) {
			Error e(ERR_FILE_ALREADY_OPEN, "File already open");
			async_err_callback(Cb(&Inl::fs_error_cb, this), move(e), loop());
			return;
		}
		if (m_opening) {
			Error e(ERR_FILE_OPENING, "File opening...");
			async_err_callback(Cb(&Inl::fs_error_cb, this), move(e), loop());
			return;
		}
		m_opening = true;
		auto req = new FileReq(this);
		uv_fs_open(uv_loop(), req->req(),
							 Path::fallback_c(m_path),
							 inl__file_flag_mask(flag), FileHelper::default_mode, &Inl::fs_open_cb);
	}
	
	void close() {
		if (m_fp) {
			int fp = m_fp;
			m_fp = 0;
			auto req = new FileReq(this);
			uv_fs_close(uv_loop(), req->req(), fp, &AsyncFile::Inl::fs_close_cb);
		}
		else {
			Error e(ERR_FILE_NOT_OPEN, "File not open");
			async_err_callback(Cb(&Inl::fs_error_cb, this), move(e), loop());
		}
	}

	void read(Buffer& buffer, int64 offset, int mark) {
		auto req = new FileStreamReq(this, 0, { buffer, mark });
		uv_buf_t buf;
		buf.base = req->data().buffer.value();
		buf.len = req->data().buffer.length();
		uv_fs_read(uv_loop(), req->req(), m_fp, &buf, 1, offset, &Inl::fs_read_cb);
	}

	void write(Buffer& buffer, int64 offset, int mark) {
		m_writeing.push(new FileStreamReq(this, 0, { buffer, offset, mark }));
		if (m_writeing.length() == 1) {
			continue_write();
		}
	}

	inline RunLoop* loop() { return m_keep->host(); }

 private:
	inline uv_loop_t* uv_loop() { return loop()->uv_loop(); }
	inline static RunLoop* loop(uv_fs_t* req) { return FileReq::cast(req)->ctx()->loop(); }
		
	void clear_writeing() {
		for(auto& i : m_writeing) {
			i.value()->release();
		}
		m_writeing.clear();
	}

	void continue_write() {
		if (m_writeing.length()) {
			auto req = m_writeing.first();
			uv_buf_t buf;
			buf.base = req->data().buffer.value();
			buf.len = req->data().buffer.length();
			// LOG("write_first-- %ld", req->data().offset);
			uv_fs_write(uv_loop(), req->req(), m_fp, &buf, 1, req->data().offset, &Inl::fs_write_cb);
		}
	}
		
	void fs_error_cb(Se& evt) {
		m_delegate->trigger_async_file_error(m_host, *evt.error);
	}
	
	static inline Delegate* del(Object* ctx) {
		return static_cast<FileReq*>(ctx)->ctx()->m_delegate;
	}
	
	static inline AsyncFile* host(Object* ctx) {
		return static_cast<FileReq*>(ctx)->ctx()->m_host;
	}

	static void fs_open_cb(uv_fs_t* uv_req) {
		uv_fs_req_cleanup(uv_req);
		FileReq* req = FileReq::cast(uv_req);
		Handle<FileReq> handle(req);
		XX_ASSERT( req->ctx()->m_opening );
		req->ctx()->m_opening = false;
		if ( uv_req->result > 0 ) {
			if ( req->ctx()->m_fp ) {
				uv_fs_t close_req;
				uv_fs_close(uv_req->loop, &close_req, (uv_file)uv_req->result, nullptr); // sync
				Error err(ERR_FILE_ALREADY_OPEN, "file already open");
				del(req)->trigger_async_file_error(host(req), err);
			} else {
				req->ctx()->m_fp = (int)uv_req->result;
				del(req)->trigger_async_file_open(host(req));
			}
		} else {
			Error err((int)uv_req->result, "%s, %s",
								uv_err_name((int)uv_req->result), uv_strerror((int)uv_req->result));
			del(req)->trigger_async_file_error(host(req), err);
		}
	}
	
	static void fs_close_cb(uv_fs_t* uv_req) {
		uv_fs_req_cleanup(uv_req);
		FileReq* req = FileReq::cast(uv_req);
		Handle<FileReq> handle(req);
		if ( uv_req->result == 0 ) { // ok
			// ctx->ctx()->m_fp = 0;
			del(req)->trigger_async_file_close(host(req));
		} else {
			Error err((int)uv_req->result, "%s, %s",
								uv_err_name((int)uv_req->result), uv_strerror((int)uv_req->result));
			del(req)->trigger_async_file_error(host(req), err);
		}
		req->ctx()->clear_writeing();
	}
	
	static void fs_read_cb(uv_fs_t* uv_req) {
		FileStreamReq* req = FileStreamReq::cast(uv_req);
		Handle<FileStreamReq> handle(req);
		uv_fs_req_cleanup(uv_req);
		if ( uv_req->result < 0 ) {
			Error err((int)uv_req->result, "%s, %s",
								uv_err_name((int)uv_req->result), uv_strerror((int)uv_req->result));
			del(req)->trigger_async_file_error(host(req), err);
		} else {
			del(req)->trigger_async_file_read(host(req),
																				req->data().buffer.realloc((uint)uv_req->result),
																				req->data().mark );
		}
	}
	
	static void fs_write_cb(uv_fs_t* uv_req) {
		FileStreamReq* req = FileStreamReq::cast(uv_req);
		Handle<FileStreamReq> handle(req);
		auto self = req->ctx();
		uv_fs_req_cleanup(uv_req);

		XX_ASSERT(self->m_writeing.first() == req);
		self->m_writeing.shift();
		self->continue_write();

		if ( uv_req->result < 0 ) {
			Error err((int)uv_req->result, "%s, %s",
								uv_err_name((int)uv_req->result), uv_strerror((int)uv_req->result));
			del(req)->trigger_async_file_error(host(req), err);
		} else {
			del(req)->trigger_async_file_write(host(req), req->data().buffer, req->data().mark);
		}
	}

 private:
	String      m_path;
	int         m_fp;
	bool        m_opening;
	KeepLoop*   m_keep;
	Delegate*   m_delegate;
	AsyncFile*  m_host;
	List<FileStreamReq*> m_writeing;
};

AsyncFile::AsyncFile(cString& path, RunLoop* loop)
: m_inl(NewRetain<AsyncFile::Inl>(this, path, loop))
{ }

AsyncFile::~AsyncFile() {
	XX_CHECK(m_inl->loop() == RunLoop::current());
	m_inl->set_delegate(nullptr);
	if (m_inl->is_open())
		m_inl->close();
	m_inl->release();
	m_inl = nullptr;
}

String AsyncFile::path() const { return m_inl->path(); }

void AsyncFile::set_delegate(Delegate* delegate) {
	m_inl->set_delegate(delegate);
}

bool AsyncFile::is_open() {
	return m_inl->is_open();
}

void AsyncFile::open(int flag) {
	m_inl->open(flag);
}

void AsyncFile::close() {
	m_inl->close();
}

void AsyncFile::read(Buffer buffer, int64 offset, int mark) {
	m_inl->read(buffer, offset, mark);
}

void AsyncFile::write(Buffer buffer, int64 offset, int mark) {
	m_inl->write(buffer, offset, mark);
}

XX_END
