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

#include "lutils/util.h"
#include "lutils/fs.h"

using namespace langou;

class AsyncFileRead: public AsyncFile, public AsyncFile::Delegate {
 public:
	
	AsyncFileRead(cString& src): AsyncFile(src) {
		set_delegate(this);
	}
	
	virtual ~AsyncFileRead() {
		LOG("Delete");
		FileHelper::read_file(Path::resources("res/bg.svg"), Cb([](Se& evt) {
			if ( evt.error ) {
				LOG("ERR, %s", evt.error->message().c());
			} else {
				LOG( static_cast<Buffer*>(evt.data)->collapse_string() );
			}
			RunLoop::current()->stop();
		}));
	}
	
	virtual void trigger_async_file_error(AsyncFile* file, cError& error) {
		LOG("Error, %s", error.message().c());
		delete this;
	}
	virtual void trigger_async_file_open(AsyncFile* file) {
		LOG("Open, %s", *path());
		read(Buffer(1024), 1024); // start read
	}
	virtual void trigger_async_file_close(AsyncFile* file) {
		LOG("Close");
		Release(this);
	}
	virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) {
		if ( buffer.length() ) {
			LOG( buffer.collapse_string() );
			read(buffer, 1024); // read
		} else {
			// read end
			LOG("Read END");
			close();
		}
	}
	
	virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) { }
	
};

class AsyncFileWrite: public AsyncFile, public AsyncFile::Delegate {
 public:
	
	AsyncFileWrite(cString& src): AsyncFile(src) {
		set_delegate(this);
	}
	
	virtual ~AsyncFileWrite() {
		LOG("Delete WriteFileAsync");
	}
	
	virtual void trigger_async_file_error(AsyncFile* file, cError& error) {
		LOG("Error, %s", error.message().c());
		RunLoop::current()->stop();
		Release(this);
	}
	virtual void trigger_async_file_open(AsyncFile* file) {
		LOG("Open, %s", *path());
		write(String("ABCDEFG-").collapse_buffer()); // start read
	}
	virtual void trigger_async_file_close(AsyncFile* file) {
		LOG("Close");
		Release(this);
	}
	virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) {
		LOG("Write ok");
		(new AsyncFileRead(path()))->open();
		close();
	}
	
	virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) {}
};

void test_file_async(int argc, char **argv) {
	New<AsyncFileWrite>(Path::documents("test2.txt"))->open(FOPEN_A);
	RunLoop::current()->run();
}
