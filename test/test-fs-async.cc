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

#include "quark/util/util.h"
#include "quark/util/fs.h"

using namespace qk;

class AsyncFileRead: public File, public File::Delegate {
 public:
	
	AsyncFileRead(cString& src): File(src) {
		set_delegate(this);
	}
	
	virtual ~AsyncFileRead() {
		Qk_LOG("Delete");
		fs_read_file(fs_resources("res/bg.svg"), Cb([](Cb::Data& evt) {
			if ( evt.error ) {
				Qk_LOG("ERR, %s", evt.error->message().c_str());
			} else {
				Qk_LOG( static_cast<Buffer*>(evt.data)->collapse_string() );
			}
			RunLoop::current()->stop();
		}));
	}
	
	virtual void trigger_file_error(File* file, cError& error) {
		Qk_LOG("Error, %s", error.message().c_str());
		delete this;
	}
	virtual void trigger_file_open(File* file) {
		Qk_LOG("Open, %s", *path());
		read(Buffer::alloc(1024), 1024); // start read
	}
	virtual void trigger_file_close(File* file) {
		Qk_LOG("Close");
		Release(this);
	}
	virtual void trigger_file_read(File* file, Buffer buffer, int mark) {
		if ( buffer.length() ) {
			Qk_LOG( buffer.collapse_string() );
			read(buffer, 1024); // read
		} else {
			// read end
			Qk_LOG("Read END");
			close();
		}
	}
	
	virtual void trigger_file_write(File* file, Buffer buffer, int mark) { }
	
};

class AsyncFileWrite: public File, public File::Delegate {
 public:
	
	AsyncFileWrite(cString& src): File(src) {
		set_delegate(this);
	}
	
	virtual ~AsyncFileWrite() {
		Qk_LOG("Delete WriteFileAsync");
	}
	
	virtual void trigger_file_error(File* file, cError& error) {
		Qk_LOG("Error, %s", error.message().c_str());
		RunLoop::current()->stop();
		Release(this);
	}
	virtual void trigger_file_open(File* file) {
		Qk_LOG("Open, %s", *path());
		write(String("ABCDEFG-").collapse()); // start read
	}
	virtual void trigger_file_close(File* file) {
		Qk_LOG("Close");
		Release(this);
	}
	virtual void trigger_file_write(File* file, Buffer buffer, int mark) {
		Qk_LOG("Write ok");
		(new AsyncFileRead(path()))->open();
		close();
	}
	
	virtual void trigger_file_read(File* file, Buffer buffer, int mark) {}
};

void test_file_async(int argc, char **argv) {
	New<AsyncFileWrite>(fs_documents("test2.txt"))->open(FOPEN_A);
	RunLoop::current()->run();
}
