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

#include "./error.h"
#include "./fs.h"
#include "./uv.h"
#include <list>

namespace qk {

#if Qk_WIN
	const uint32_t fs_default_mode = 0;
#else
	const uint32_t fs_default_mode([]() {
		uint32_t mask = umask(0);
		umask(mask);
		return 0777 & ~mask;
	}());
#endif

	void file_stat_copy(uv_stat_t* src, FileStat* dest) {
		struct INL_FileStat: Object {
			uv_stat_t *_stat;
		};
		auto s = reinterpret_cast<INL_FileStat*>(dest);
		if ( !s->_stat ) { // alloc memory space
			s->_stat = (uv_stat_t*)malloc(sizeof(uv_stat_t));
		}
		memcpy(s->_stat, src, sizeof(uv_stat_t));
	}

	FileStat::FileStat(): _stat(nullptr) {}
	FileStat::FileStat(cString& path): FileStat(fs_stat_sync(path)) {}
	FileStat::FileStat(const FileStat& src): _stat(nullptr) {
		file_stat_copy((uv_stat_t*)src._stat, this);
	}

	FileStat& FileStat::operator=(const FileStat& src) {
		file_stat_copy((uv_stat_t*)src._stat, this);
		return *this;
	}

	FileStat::~FileStat() {
		free(_stat);
	}

	uv_stat_t stat;

	#define STAT (_stat ? (uv_stat_t*)_stat : &stat)

	bool FileStat::is_valid() const { return _stat; }
	bool FileStat::is_file() const { return S_ISREG(STAT->st_mode); }
	bool FileStat::is_dir() const { return S_ISDIR(STAT->st_mode); }
	bool FileStat::is_link() const { return S_ISLNK(STAT->st_mode); }
	bool FileStat::is_sock() const { return S_ISSOCK(STAT->st_mode); }
	uint64_t FileStat::mode() const { return STAT->st_mode; }

	FileType FileStat::type() const {
		if ( _stat ) {
			if ( S_ISREG(((uv_stat_t*)_stat)->st_mode) ) return FTYPE_FILE;
			if ( S_ISDIR(((uv_stat_t*)_stat)->st_mode) ) return FTYPE_DIR;
			if ( S_ISLNK(((uv_stat_t*)_stat)->st_mode) ) return FTYPE_LINK;
			if ( S_ISFIFO(((uv_stat_t*)_stat)->st_mode) ) return FTYPE_FIFO;
			if ( S_ISSOCK(((uv_stat_t*)_stat)->st_mode) ) return FTYPE_SOCKET;
			if ( S_ISCHR(((uv_stat_t*)_stat)->st_mode) ) return FTYPE_CHAR;
			if ( S_ISBLK(((uv_stat_t*)_stat)->st_mode) ) return FTYPE_BLOCK;
		}
		return FTYPE_UNKNOWN;
	}

	uint64_t FileStat::group() const { return STAT->st_gid; }
	uint64_t FileStat::owner() const { return STAT->st_uid; }
	uint64_t FileStat::nlink() const { return STAT->st_nlink; }
	uint64_t FileStat::ino() const { return STAT->st_ino; }
	uint64_t FileStat::blksize() const { return STAT->st_blksize; }
	uint64_t FileStat::blocks() const { return STAT->st_blocks; }
	uint64_t FileStat::flags() const { return STAT->st_flags; }
	uint64_t FileStat::gen() const { return STAT->st_gen; }
	uint64_t FileStat::dev() const { return STAT->st_dev; }
	uint64_t FileStat::rdev() const { return STAT->st_rdev; }
	uint64_t FileStat::size() const { return STAT->st_size; }
	uint64_t FileStat::atime() const {
		return (int64_t)STAT->st_atim.tv_sec * 1000000 + STAT->st_atim.tv_nsec / 1000;
	}
	uint64_t FileStat::mtime() const {
		return (int64_t)STAT->st_mtim.tv_sec * 1000000 + STAT->st_mtim.tv_nsec / 1000;
	}
	uint64_t FileStat::ctime() const {
		return (int64_t)STAT->st_ctim.tv_sec * 1000000 + STAT->st_ctim.tv_nsec / 1000;
	}
	uint64_t FileStat::birthtime() const {
		return (int64_t)STAT->st_birthtim.tv_sec * 1000000 + STAT->st_birthtim.tv_nsec / 1000;
	}

	#undef STAT

	/**
	 * @func get C file flga
	 */
	int file_flag_mask(int flag) {
#if Qk_POSIX
		return flag;
#else // Qk_WIN
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

	cChar* file_flag_str(int flag) {
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

	FileSync::FileSync(cString& path): _path(path), _fd(0) {}

	FileSync::~FileSync() {
		close();
	}

	bool FileSync::is_open() {
		return _fd;
	}

	int FileSync::open(int flag, uint32_t mode) {
		if ( _fd ) { // 文件已经打开
			Qk_Warn("File handle already open" );
			return 0;
		}
		uv_fs_t req;
		_fd = uv_fs_open(uv_default_loop(), &req,
											fs_fallback_c(_path),
											file_flag_mask(flag), mode, nullptr);
		if ( _fd > 0 ) {
			return 0;
		}
		return _fd;
	}

	int FileSync::close() {
		if ( !_fd ) return 0;
		uv_fs_t req;
		int r = uv_fs_close(uv_default_loop(), &req, _fd, nullptr);
		if ( r == 0 ) {
			_fd = 0;
		}
		return r;
	}

	int FileSync::read(void* buffer, int64_t size, int64_t offset) {
		uv_fs_t req;
		uv_buf_t buf;
		buf.base = (Char*)buffer;
		buf.len = size;
		return uv_fs_read(uv_default_loop(), &req, _fd, &buf, 1, offset, nullptr);
	}

	int FileSync::write(cVoid* buffer, int64_t size, int64_t offset) {
		uv_fs_t req;
		uv_buf_t buf;
		buf.base = (Char*)buffer;
		buf.len = size;
		return uv_fs_write(uv_default_loop(), &req, _fd, &buf, 1, offset, nullptr);
	}

	// ----------------------------------- FileAsync -----------------------------------------

	struct FileStreamData {
		Buffer buffer;
		int64_t offset;
		int flag;
	};

	typedef UVRequestWrap<uv_fs_t, File::Inl> FileReq;
	typedef UVRequestWrap<uv_fs_t, File::Inl, FileStreamData> FileStreamReq;

	class File::Inl: public Reference, public File::Delegate {
	public:
		typedef File::Delegate Delegate;
		virtual void trigger_file_open (File* file) override { }
		virtual void trigger_file_close(File* file) override { }
		virtual void trigger_file_error(File* file, cError& error) override { }
		virtual void trigger_file_read (File* file, Buffer &buffer, int flag) override { }
		virtual void trigger_file_write(File* file, Buffer &buffer, int flag) override { }

		void set_delegate(Delegate* delegate) {
			if ( delegate ) {
				_delegate = delegate;
			} else {
				_delegate = this;
			}
		}

		Inl(File* host, cString& path, RunLoop* loop)
			: _path(path)
			, _fd(0)
			, _opening(false)
			, _loop(loop)
			, _delegate(nullptr)
			, _host(host)
		{
			Qk_ASSERT(loop);
		}

		~Inl() override {
			if ( _fd ) {
				uv_fs_t req;
				int res = uv_fs_close(_loop->uv_loop(), &req, _fd, nullptr); // sync
				Qk_ASSERT( res == 0 );
			}
			clear_writeing();
		}

		inline String& path() { return _path; }
		inline bool is_open() { return _fd; }

		void open(int flag, uint32_t mode) {
			if (_fd) {
				async_reject(Cb(&Inl::fs_error_cb, this),
					Error(ERR_FILE_ALREADY_OPEN, "File already open"), _loop);
				return;
			}
			if (_opening) {
				async_reject(Cb(&Inl::fs_error_cb, this), Error(ERR_FILE_OPENING, "File opening..."), _loop);
				return;
			}
			_opening = true;
			auto req = new FileReq(this);
			uv_fs_open(uv_loop(), req->req(),
								 fs_fallback_c(_path),
								 file_flag_mask(flag), mode, &Inl::fs_open_cb);
		}

		void close() {
			if (_fd) {
				int fp = _fd;
				_fd = 0;
				auto req = new FileReq(this);
				uv_fs_close(uv_loop(), req->req(), fp, &File::Inl::fs_close_cb);
			}
			else {
				Error e(ERR_FILE_NOT_OPEN, "File not open");
				async_reject(Cb(&Inl::fs_error_cb, this), std::move(e), _loop);
			}
		}

		void read(Buffer& buffer, int64_t offset, int flag) {
			auto req = new FileStreamReq(this, 0, { buffer, flag });
			uv_buf_t buf;
			buf.base = *req->data().buffer;
			buf.len = req->data().buffer.length();
			uv_fs_read(uv_loop(), req->req(), _fd, &buf, 1, offset, &Inl::fs_read_cb);
		}

		void write(Buffer& buffer, int64_t offset, int flag) {
			_writeing.pushBack(new FileStreamReq(this, 0, { buffer, offset, flag }));
			if (_writeing.length() == 1) {
				continue_write();
			}
		}

		inline RunLoop* loop() { return _loop; }

	private:
		inline uv_loop_t* uv_loop() { return _loop->uv_loop(); }
		inline static RunLoop* loop(uv_fs_t* req) { return FileReq::cast(req)->ctx()->_loop; }
			
		void clear_writeing() {
			for(auto& i : _writeing) {
				i->release();
			}
			_writeing.clear();
		}

		void continue_write() {
			if (_writeing.length()) {
				auto req = _writeing.front();
				uv_buf_t buf;
				buf.base = req->data().buffer.val();
				buf.len = req->data().buffer.length();
				// Qk_Log("write_first-- %ld", req->data().offset);
				uv_fs_write(uv_loop(), req->req(), _fd, &buf, 1, req->data().offset, &Inl::fs_write_cb);
			}
		}

		void fs_error_cb(Cb::Data& evt) {
			_delegate->trigger_file_error(_host, *evt.error);
		}

		static inline Delegate* del(Object* ctx) {
			return static_cast<FileReq*>(ctx)->ctx()->_delegate;
		}

		static inline File* host(Object* ctx) {
			return static_cast<FileReq*>(ctx)->ctx()->_host;
		}

		static void fs_open_cb(uv_fs_t* uv_req) {
			uv_fs_req_cleanup(uv_req);
			FileReq* req = FileReq::cast(uv_req);
			Handle<FileReq> handle(req);
			Qk_ASSERT( req->ctx()->_opening );
			req->ctx()->_opening = false;
			if ( uv_req->result > 0 ) {
				if ( req->ctx()->_fd ) {
					uv_fs_t close_req;
					uv_fs_close(uv_req->loop, &close_req, (uv_file)uv_req->result, nullptr); // sync
					Error err(ERR_FILE_ALREADY_OPEN, "file already open");
					del(req)->trigger_file_error(host(req), err);
				} else {
					req->ctx()->_fd = (int)uv_req->result;
					del(req)->trigger_file_open(host(req));
				}
			} else {
				Error err((int)uv_req->result, "%s, %s",
									uv_err_name((int)uv_req->result), uv_strerror((int)uv_req->result));
				del(req)->trigger_file_error(host(req), err);
			}
		}

		static void fs_close_cb(uv_fs_t* uv_req) {
			uv_fs_req_cleanup(uv_req);
			FileReq* req = FileReq::cast(uv_req);
			Handle<FileReq> handle(req);
			if ( uv_req->result == 0 ) { // ok
				// ctx->ctx()->_fd = 0;
				del(req)->trigger_file_close(host(req));
			} else {
				Error err((int)uv_req->result, "%s, %s",
									uv_err_name((int)uv_req->result), uv_strerror((int)uv_req->result));
				del(req)->trigger_file_error(host(req), err);
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
				del(req)->trigger_file_error(host(req), err);
			} else {
				req->data().buffer.reset((uint32_t)uv_req->result);
				del(req)->trigger_file_read(host(req),
																					req->data().buffer,
																					req->data().flag );
			}
		}

		static void fs_write_cb(uv_fs_t* uv_req) {
			FileStreamReq* req = FileStreamReq::cast(uv_req);
			Handle<FileStreamReq> handle(req);
			auto self = req->ctx();
			uv_fs_req_cleanup(uv_req);
			
			Qk_ASSERT(self->_writeing.front() == req);
			self->_writeing.popFront();
			self->continue_write();

			if ( uv_req->result < 0 ) {
				Error err((int)uv_req->result, "%s, %s",
									uv_err_name((int)uv_req->result), uv_strerror((int)uv_req->result));
				del(req)->trigger_file_error(host(req), err);
			} else {
				del(req)->trigger_file_write(host(req), req->data().buffer, req->data().flag);
			}
		}

	private:
		String      _path;
		int         _fd;
		bool        _opening;
		RunLoop*    _loop;
		Delegate*   _delegate;
		File*  _host;
		List<FileStreamReq*> _writeing;
	};

	File::File(cString& path, RunLoop* loop)
	: _inl(NewRetain<File::Inl>(this, path, loop))
	{}

	File::~File() {
		Qk_ASSERT(_inl->loop() == RunLoop::current());
		_inl->set_delegate(nullptr);
		if (_inl->is_open())
			_inl->close();
		_inl->release();
		_inl = nullptr;
	}

	String File::path() const { return _inl->path(); }

	void File::set_delegate(Delegate* delegate) {
		_inl->set_delegate(delegate);
	}

	bool File::is_open() {
		return _inl->is_open();
	}

	void File::open(int flag, uint32_t mode) {
		_inl->open(flag, mode);
	}

	void File::close() {
		_inl->close();
	}

	void File::read(Buffer buffer, int64_t offset, int flag) {
		_inl->read(buffer, offset, flag);
	}

	void File::write(Buffer buffer, int64_t offset, int flag) {
		_inl->write(buffer, offset, flag);
	}

}
