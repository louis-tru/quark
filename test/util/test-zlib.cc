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

#include <src/util/zlib.h>
#include <zlib.h>
#include <map>
#include "../test.h"

using namespace qk;

Qk_TEST_Func(zlib) {
	
	std::map<String, int> map;
	
	String a("a");
	String b("b");
	
	map[a] = 100;
	map[b] = 200;
	map["c"] = 300;
	map["d"] = 400;
	
	Qk_Log(map[a]);
	Qk_Log(map[b]);
	Qk_Log(map["c"]);
	Qk_Log(map["d"]);
	Qk_Log("");
	
	const char* hello = "g++ -pthread -rdynamic -m64 ";
	size_t len = strlen(hello) + 1;
	Qk_Log("%d", len);
	Qk_Log(hello);

	{
		uint8_t* compr = (uint8_t*)malloc(100);
		size_t comprLen = 100;
		//    int err = compress(compr, &comprLen, (const byte*)hello, len);
		//    LOG("START, %s, %u, %d, END", (char*)compr, comprLen, err);
		free(compr);
	}
	
	{
		char* compr = (char*)malloc(100);
		size_t comprLen = 100;
		//    int err = ZLib::compress(compr, &comprLen, hello, len);
		//    LOG("START, %s, %u, %d, END", (char*)compr, comprLen, err);
	
		char* compr2 = (char*)malloc(100);
		size_t comprLen2 = 100;
		//    err = ZLib::uncompress(compr2, &comprLen2, compr, comprLen);
		//    LOG("START, %s, %u, %d, END", (char*)compr2, comprLen2, err);
		free(compr);
		free(compr2);
	}
	
	Qk_Log("K\n");
	
	GZip gzip(fs_documents("test.gz"));
	Qk_Log(gzip.open());
	
	Qk_Log(gzip.write(hello, uint(len - 1)));
	Qk_Log(gzip.write("hello", 5));
	
	Qk_Log("a\n");
	gzip.close();
	Qk_Log(gzip.open(FOPEN_R));
	char str[100] = { 0 };
	Qk_Log(gzip.read(str, 100));
	Qk_Log(str);
	
	gzip.close();
	
	Qk_Log("\nTEST zip\n");
	
	{
		ZipWriter writer(fs_documents("test.zip"));
		writer.open();
		Qk_Log(writer.add_file("aa.txt"));
		Qk_Log(writer.write(WeakBuffer("------------- aa.txt", 21).buffer()));
		Qk_Log(writer.add_file("bb.txt"));
		Qk_Log(writer.write(WeakBuffer("------------- bb.txt", 21).buffer()));
		Qk_Log(writer.add_file("cc.txt"));
		Qk_Log(writer.write(WeakBuffer("------------- cc.txt", 21).buffer()));
		Qk_Log(writer.add_file("dd.txt"));
		Qk_Log(writer.write(WeakBuffer("------------- dd.txt", 21).buffer()));
	}

	{
		ZipReader reader(fs_documents("test.zip"));
		reader.open();
		char str2[101] = { 0 };
		Qk_Log(reader.read(str2, 100));
		Qk_Log(str2);
		reader.next();
		Qk_Log(reader.read(str2, 100));
		Qk_Log(str2);
		reader.next();
		Qk_Log(reader.read(str2, 100));
		Qk_Log(str2);
		reader.first();
		Qk_Log(reader.read(str2, 100));
		Qk_Log(str2);
		//
		reader.jump("cc.txt");
		char str3[101] = { 0 };
		Qk_Log(reader.read(str3, 3));
		Qk_Log(str3);
		Qk_Log(reader.exists("dd.txt"));
		Qk_Log(reader.read(str3, 3));
		Qk_Log(str3);
		Qk_Log(reader.exists("kk.txt"));
		//
		Qk_Log("next, %d", reader.next());
		Qk_Log(reader.read(str2, 100));
		Qk_Log(str2);
		Qk_Log("next, %d", reader.next());
		Qk_Log(reader.read(str2, 100));
		Qk_Log(str2);
		Qk_Log("next, %d", reader.next());
		Qk_Log(reader.read(str2, 100));
		Qk_Log(str2);
		Qk_Log(reader.current());
	}
}
