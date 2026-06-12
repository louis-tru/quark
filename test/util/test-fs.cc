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
#include <trial/fs.h>
#include <pwd.h>
#include "../test.h"

using namespace qk;

Qk_TEST_Func(fs) {

	String test_str = "ABCDEFG";

	Qk_Log("%d", test_str.lastIndexOf("G", 1));

	Qk_Log("%s", *fs_executable());

	Qk_Log("%s", *fs_cwd());
	
	if(fs_chdir("/home/louis/www/jsxdev/bin/") == 0){
		Qk_Log("修改工作路径成功");
	}
	
	Qk_Log("%s", *fs_cwd());
	
	String str = "/tmp/kkl/";
	
	Qk_Log(str.length() - str.lastIndexOf('/') == 1);
	
	Qk_Log(fs_documents());
	
	fs_chdir(fs_executable()); // 应该不能成功
	
	Qk_Log(fs_cwd()); // echo "/"
	
	// ios 测试成功
	Qk_Log("\nTEST FileSearch");
	fs_chdir( fs_dirname(fs_executable()) );
	Qk_Log("设置工作目录,%s", *fs_cwd());
	
	FileSearch* search = FileSearch::shared();
	search->add_search_path(fs_resources("jsapi/res/test"));
	search->add_search_path(fs_resources("jsapi/res/test/test"));
	search->add_zip_search_path(fs_resources("jsapi/res/test-fs.apk"), "assets");
	search->add_zip_search_path(fs_resources("jsapi/res/test-fs.apk"), "assets/thk/res_r_hd");
	
	Qk_Log("length, %d", search->get_search_paths().length());
	
	Qk_Log("aa.txt,exist,%d", search->exists("aa.txt"));
	Qk_Log("bb.txt,exist,%d", search->exists("bb.txt"));
	Qk_Log("cc.txt,exist,%d", search->exists("cc.txt"));
	Qk_Log("dd.txt,exist,%d", search->exists("dd.txt"));
	
	Qk_Log("aa.txt,abs,%s", search->get_absolute_path("aa.txt").c_str());
	Qk_Log("bb.txt,abs,%s", search->get_absolute_path("bb.txt").c_str());
	Qk_Log("cc.txt,abs,%s", search->get_absolute_path("cc.txt").c_str());
	Qk_Log("dd.txt,abs,%s", search->get_absolute_path("dd.txt").c_str());
	
	// The normal need to release the memory
	Qk_Log("aa.txt,data,%s", search->read("aa.txt").val());
	Qk_Log("bb.txt,data,%s", search->read("bb.txt").val());
	Qk_Log("cc.txt,data,%s", search->read("cc.txt").val());
	Qk_Log("dd.txt,data,%s", search->read("dd.txt").val());
	
	Qk_Log("\nTEST zip\n");
	
	Qk_Log("bgm/1.mp3,exist,%d", search->exists("bgm/1.mp3"));
	Qk_Log("bin/_nmxOB.js_v274443,exist,%d", search->exists("bin/_nmxOB.js_v274443"));
	Qk_Log("thk/res_r_hd/ad/39.jpg_v94859,exist,%d", search->exists("thk/res_r_hd/ad/39.jpg_v94859"));
	Qk_Log("ad/39.jpg_v94859,exist,%d", search->exists("ad/39.jpg_v94859"));
	
	Qk_Log("bgm/1.mp3,abs,%s", search->get_absolute_path("bgm/1.mp3").c_str());
	Qk_Log("bin/_nmxOB.js_v274443,abs,%s", search->get_absolute_path("bin/_nmxOB.js_v274443").c_str());
	Qk_Log("thk/res_r_hd/ad/39.jpg_v94859,abs,%s", search->get_absolute_path("thk/res_r_hd/ad/39.jpg_v94859").c_str());
	Qk_Log("ad/39.jpg_v94859,abs,%s", search->get_absolute_path("ad/39.jpg_v94859").c_str());

	// The normal need to release the memory
	Qk_Log("bgm/1.mp3,data,%s", search->read("bgm/1.mp3").val());
	
	
	Buffer s = search->read("bin/_nmxOB.js_v274443");
	Qk_Log("Copy data");
	Buffer d = s;
	Qk_Log("%i", s.isNull());
	Qk_Log("bin/_nmxOB.js_v274443,data,%s", d.val());
	Qk_Log("Copy string");
	String ss = std::move(d);
	Qk_Log("%i", d.isNull());
	Qk_Log("bin/_nmxOB.js_v274443,data,%s", ss.c_str());
	Qk_Log("\n");
	
	Qk_Log("Copy string 2");
	String s2 = search->read("bin/_nmxOB.js_v274443");
	Qk_Log("bin/_nmxOB.js_v274443,data,%s", s2.c_str());
	Qk_Log("\n");
	
	Qk_Log("Copy string operator=, default call copy constructor ");
	String s3 = "String2";
	s3 = search->read("bin/_nmxOB.js_v274443").val();
	Qk_Log("BB");
	Qk_Log("bin/_nmxOB.js_v274443,data,%s", s3.c_str());
	Qk_Log("\n");
	
	Qk_Log("Copy data operator=");
	d = search->read("bin/_nmxOB.js_v274443");
	Qk_Log("bin/_nmxOB.js_v274443,data,%s", d.val());
	Qk_Log("\n");
	
	Qk_Log("thk/res_r_hd/ad/39.jpg_v94859,data,%s", search->read("thk/res_r_hd/ad/39.jpg_v94859").val());
	Qk_Log("ad/39.jpg_v94859,data,%s", search->read("ad/39.jpg_v94859").val());
	Qk_Log("\n");

	Qk_Log("Test zip path");
	Qk_Log(search->get_absolute_path("bin/_nmxOB.js_v274443"));
	Qk_Log("ad/39.jpg_v94859,data,%s", search->read(search->get_absolute_path("bin/_nmxOB.js_v274443")).val());
	Qk_Log("\n");
	
	Qk_Log("Test zip get_absolute_path");
	Qk_Log(search->get_absolute_path("thk/../../assets/thk/../thk/res_r_hd/ad/39.jpg_v94859"));

	Qk_Log("Very good, doen");
}
