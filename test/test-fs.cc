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
#include <trial/fs.h>
#include <pwd.h>

using namespace flare;

void test_fs(int argc, char **argv) {
	
	String test_str = "ABCDEFG";
	
	F_LOG(test_str.last_index_of("G", 1));
	
	F_LOG(fs_executable());
	
	F_LOG(fs_cwd());
	
	if(fs_chdir("/home/louis/www/jsxdev/bin/") == 0){
		F_LOG("修改工作路径成功");
	}
	
	F_LOG(fs_cwd());
	
	String str = "/tmp/kkl/";
	
	F_LOG(str.length() - str.last_index_of('/') == 1);
	
	F_LOG(fs_documents());
	
	fs_chdir(fs_executable()); // 应该不能成功
	
	F_LOG(fs_cwd()); // echo "/"
	
	// ios 测试成功
	F_LOG("\nTEST FileSearch");
	fs_chdir( fs_dirname(fs_executable()) );
	F_LOG("设置工作目录,%s", *fs_cwd());
	
	FileSearch* search = FileSearch::shared();
	search->add_search_path("res/test");
	search->add_search_path("res/test/test");
	search->add_zip_search_path("res/test-fs.apk", "assets");
	search->add_zip_search_path("res/test-fs.apk", "assets/thk/res_r_hd");
	
	F_LOG("length, %d", search->get_search_paths().length());
	
	F_LOG("aa.txt,exist,%d", search->exists("aa.txt"));
	F_LOG("bb.txt,exist,%d", search->exists("bb.txt"));
	F_LOG("cc.txt,exist,%d", search->exists("cc.txt"));
	F_LOG("dd.txt,exist,%d", search->exists("dd.txt"));
	
	F_LOG("aa.txt,abs,%s", search->get_absolute_path("aa.txt").c_str());
	F_LOG("bb.txt,abs,%s", search->get_absolute_path("bb.txt").c_str());
	F_LOG("cc.txt,abs,%s", search->get_absolute_path("cc.txt").c_str());
	F_LOG("dd.txt,abs,%s", search->get_absolute_path("dd.txt").c_str());
	
	// The normal need to release the memory
	F_LOG("aa.txt,data,%s", search->read("aa.txt").val());
	F_LOG("bb.txt,data,%s", search->read("bb.txt").val());
	F_LOG("cc.txt,data,%s", search->read("cc.txt").val());
	F_LOG("dd.txt,data,%s", search->read("dd.txt").val());
	
	F_LOG("\nTEST zip\n");
	
	F_LOG("bgm/1.mp3,exist,%d", search->exists("bgm/1.mp3"));
	F_LOG("bin/_nmxOB.js_v274443,exist,%d", search->exists("bin/_nmxOB.js_v274443"));
	F_LOG("thk/res_r_hd/ad/39.jpg_v94859,exist,%d", search->exists("thk/res_r_hd/ad/39.jpg_v94859"));
	F_LOG("ad/39.jpg_v94859,exist,%d", search->exists("ad/39.jpg_v94859"));
	
	F_LOG("bgm/1.mp3,abs,%s", search->get_absolute_path("bgm/1.mp3").c_str());
	F_LOG("bin/_nmxOB.js_v274443,abs,%s", search->get_absolute_path("bin/_nmxOB.js_v274443").c_str());
	F_LOG("thk/res_r_hd/ad/39.jpg_v94859,abs,%s", search->get_absolute_path("thk/res_r_hd/ad/39.jpg_v94859").c_str());
	F_LOG("ad/39.jpg_v94859,abs,%s", search->get_absolute_path("ad/39.jpg_v94859").c_str());

	// The normal need to release the memory
	F_LOG("bgm/1.mp3,data,%s", search->read("bgm/1.mp3").val());
	
	
	Buffer s = search->read("bin/_nmxOB.js_v274443");
	F_LOG("Copy data");
	Buffer d = s;
	F_LOG("%i", s.is_null());
	F_LOG("bin/_nmxOB.js_v274443,data,%s", d.val());
	F_LOG("Copy string");
	String ss = std::move(d);
	F_LOG("%i", d.is_null());
	F_LOG("bin/_nmxOB.js_v274443,data,%s", ss.c_str());
	F_LOG("\n");
	
	F_LOG("Copy string 2");
	String s2 = search->read("bin/_nmxOB.js_v274443");
	F_LOG("bin/_nmxOB.js_v274443,data,%s", s2.c_str());
	F_LOG("\n");
	
	F_LOG("Copy string operator=, default call copy constructor ");
	String s3 = "String2";
	s3 = search->read("bin/_nmxOB.js_v274443").val();
	F_LOG("BB");
	F_LOG("bin/_nmxOB.js_v274443,data,%s", s3.c_str());
	F_LOG("\n");
	
	F_LOG("Copy data operator=");
	d = search->read("bin/_nmxOB.js_v274443");
	F_LOG("bin/_nmxOB.js_v274443,data,%s", d.val());
	F_LOG("\n");
	
	F_LOG("thk/res_r_hd/ad/39.jpg_v94859,data,%s", search->read("thk/res_r_hd/ad/39.jpg_v94859").val());
	F_LOG("ad/39.jpg_v94859,data,%s", search->read("ad/39.jpg_v94859").val());
	F_LOG("\n");

	F_LOG("Test zip path");
	F_LOG(search->get_absolute_path("bin/_nmxOB.js_v274443"));
	F_LOG("ad/39.jpg_v94859,data,%s", search->read(search->get_absolute_path("bin/_nmxOB.js_v274443")).val());
	F_LOG("\n");
	
	F_LOG("Test zip get_absolute_path");
	F_LOG(search->get_absolute_path("thk/../../assets/thk/../thk/res_r_hd/ad/39.jpg_v94859"));

	F_LOG("Very good, doen");
}
