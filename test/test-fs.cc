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

#include <ngui/utils/fs.h>
#include <pwd.h>

using namespace ngui;

void test_fs() {
	
	String test_str = "ABCDEFG";
	
	LOG(test_str.last_index_of("G", 1));
	
	LOG(Path::executable());
	
	LOG(Path::cwd());
	
	if(Path::chdir("/home/louis/www/jsxdev/bin/") == 0){
		LOG("修改工作路径成功");
	}
	
	LOG(Path::cwd());
	
	String str = "/tmp/kkl/";
	
	LOG(str.length() - str.last_index_of('/') == 1);
	
	LOG(Path::documents());
	
	Path::chdir(Path::executable()); // 应该不能成功
	
	LOG(Path::cwd()); // echo "/"
	
	// ios 测试成功
	LOG("\nTEST FileSearch");
	Path::chdir( Path::dirname(Path::executable()) );
	LOG("设置工作目录,%s", *Path::cwd());
	
	FileSearch* search = FileSearch::shared();
	search->add_search_path("res/test");
	search->add_search_path("res/test/test");
	search->add_zip_search_path("res/test-fs.apk", "assets");
	search->add_zip_search_path("res/test-fs.apk", "assets/thk/res_r_hd");
	
	LOG("length, %d", search->get_search_paths().length());
	
	LOG("aa.txt,exist,%d", search->exists("aa.txt"));
	LOG("bb.txt,exist,%d", search->exists("bb.txt"));
	LOG("cc.txt,exist,%d", search->exists("cc.txt"));
	LOG("dd.txt,exist,%d", search->exists("dd.txt"));
	
	LOG("aa.txt,abs,%s", search->get_absolute_path("aa.txt").c());
	LOG("bb.txt,abs,%s", search->get_absolute_path("bb.txt").c());
	LOG("cc.txt,abs,%s", search->get_absolute_path("cc.txt").c());
	LOG("dd.txt,abs,%s", search->get_absolute_path("dd.txt").c());
	
	// The normal need to release the memory
	LOG("aa.txt,data,%s", search->read("aa.txt").value());
	LOG("bb.txt,data,%s", search->read("bb.txt").value());
	LOG("cc.txt,data,%s", search->read("cc.txt").value());
	LOG("dd.txt,data,%s", search->read("dd.txt").value());
	
	LOG("\nTEST zip\n");
	
	LOG("bgm/1.mp3,exist,%d", search->exists("bgm/1.mp3"));
	LOG("bin/_nmxOB.js_v274443,exist,%d", search->exists("bin/_nmxOB.js_v274443"));
	LOG("thk/res_r_hd/ad/39.jpg_v94859,exist,%d", search->exists("thk/res_r_hd/ad/39.jpg_v94859"));
	LOG("ad/39.jpg_v94859,exist,%d", search->exists("ad/39.jpg_v94859"));
	
	LOG("bgm/1.mp3,abs,%s", search->get_absolute_path("bgm/1.mp3").c());
	LOG("bin/_nmxOB.js_v274443,abs,%s", search->get_absolute_path("bin/_nmxOB.js_v274443").c());
	LOG("thk/res_r_hd/ad/39.jpg_v94859,abs,%s", search->get_absolute_path("thk/res_r_hd/ad/39.jpg_v94859").c());
	LOG("ad/39.jpg_v94859,abs,%s", search->get_absolute_path("ad/39.jpg_v94859").c());

	// The normal need to release the memory
	LOG("bgm/1.mp3,data,%s", search->read("bgm/1.mp3").value());
	
	
	Buffer s = search->read("bin/_nmxOB.js_v274443");
	LOG("Copy data");
	Buffer d = s;
	LOG("%i", s.is_null());
	LOG("bin/_nmxOB.js_v274443,data,%s", d.value());
	LOG("Copy string");
	String ss = move(d);
	LOG("%i", d.is_null());
	LOG("bin/_nmxOB.js_v274443,data,%s", ss.c());
	LOG("\n");
	
	LOG("Copy string 2");
	String s2 = search->read("bin/_nmxOB.js_v274443");
	LOG("bin/_nmxOB.js_v274443,data,%s", s2.c());
	LOG("\n");
	
	LOG("Copy string operator=, default call copy constructor ");
	String s3 = "String2";
	s3 = search->read("bin/_nmxOB.js_v274443").value();
	LOG("BB");
	LOG("bin/_nmxOB.js_v274443,data,%s", s3.c());
	LOG("\n");
	
	LOG("Copy data operator=");
	d = search->read("bin/_nmxOB.js_v274443");
	LOG("bin/_nmxOB.js_v274443,data,%s", d.value());
	LOG("\n");
	
	LOG("thk/res_r_hd/ad/39.jpg_v94859,data,%s", search->read("thk/res_r_hd/ad/39.jpg_v94859").value());
	LOG("ad/39.jpg_v94859,data,%s", search->read("ad/39.jpg_v94859").value());
	LOG("\n");

	LOG("Test zip path");
	LOG(search->get_absolute_path("bin/_nmxOB.js_v274443"));
	LOG("ad/39.jpg_v94859,data,%s", search->read(search->get_absolute_path("bin/_nmxOB.js_v274443")).value());
	LOG("\n");
	
	LOG("Test zip get_absolute_path");
	LOG(search->get_absolute_path("thk/../../assets/thk/../thk/res_r_hd/ad/39.jpg_v94859"));

	LOG("Very good, doen");

}
