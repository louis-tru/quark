/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
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

#include <src/util/fs.h>
#include <src/util/thread.h>
#include "../test.h"

using namespace qk;

static String write_str;

class TestAsyncFile: public File, public File::Delegate {
 public:

	TestAsyncFile(cString& src): File(src) {
		set_delegate(this);
	}

	virtual ~TestAsyncFile() {
		Qk_Log("Delete TestAsyncFile");
	}

	virtual void trigger_file_error(File* file, cError& error) {
		Qk_Log("Error, %s", error.message().c_str());
	}

	virtual void trigger_file_open(File* file) {
		Qk_Log("Open, %s", *path());

		for ( i = 0; i < 30; i++ ) {
			write(write_str.copy().collapse(), 0);
		}
	}

	virtual void trigger_file_close(File* file) {
		Qk_Log("Close");
		Release(this);
	}

	virtual void trigger_file_write(File* file, Buffer& buffer, int mark) {
		i--;
		Qk_Log("Write ok, %d", i);

		if (i == 0) {
			String s = fs_reader()->read_file_sync(fs_documents("test_fs2.txt"));
			Qk_Log("Write count, %d", s.length());
			close();
		}
	}

	virtual void trigger_file_read(File* file, Buffer& buffer, int mark) {}

	int i = 0;

};

Qk_TEST_Func(fs2) {

	Qk_Log("START");
	
	write_str = fs_reader()->read_file_sync(fs_resources("jsapi/test.js"));

	TestAsyncFile* file = new TestAsyncFile(fs_documents("test_fs2.txt"));

	file->open(FileOpenFlag::FOPEN_W);

	RunLoop::current()->run();

	Qk_Log("END");
}
