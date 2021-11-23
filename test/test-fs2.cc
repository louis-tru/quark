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

#include <flare/util/fs.h>
#include <flare/util/loop.h>

using namespace flare;

static String write_str;

class TestAsyncFile: public AsyncFile, public AsyncFile::Delegate {
 public:

	TestAsyncFile(cString& src): AsyncFile(src) {
		set_delegate(this);
	}

	virtual ~TestAsyncFile() {
		F_LOG("Delete TestAsyncFile");
	}

	virtual void trigger_async_file_error(AsyncFile* file, cError& error) {
		F_LOG("Error, %s", error.message().c_str());
	}

	virtual void trigger_async_file_open(AsyncFile* file) {
		F_LOG("Open, %s", *path());

		for ( i = 0; i < 30; i++ ) {
			write(write_str.copy().collapse(), 0);
		}
	}

	virtual void trigger_async_file_close(AsyncFile* file) {
		F_LOG("Close");
		Release(this);
	}

	virtual void trigger_async_file_write(AsyncFile* file, Buffer buffer, int mark) {
		i--;
		F_LOG("Write ok, %d", i);

		if (i == 0) {
			String s = fs_reader()->read_file_sync(Path::documents("test_fs2.txt"));
			F_LOG("Write count, %d", s.length());
			close();
		}
	}

	virtual void trigger_async_file_read(AsyncFile* file, Buffer buffer, int mark) {}

	int i = 0;

};

void test_fs2(int argc, char **argv) {

	F_LOG("START");
	
	write_str = fs_reader()->read_file_sync(Path::resources("flare/ctr.js"));

	TestAsyncFile* file = new TestAsyncFile(Path::documents("test_fs2.txt"));

	file->open(FileOpenFlag::FOPEN_W);

	RunLoop::current()->run();

	F_LOG("END");
}
