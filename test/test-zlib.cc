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

#include <noug/util/zlib.h>
#include <zlib.h>
#include <map>

using namespace noug;

void test_zlib (int argc, char **argv) {
	
	std::map<String, int> map;
	
	String a("a");
	String b("b");
	
	map[a] = 100;
	map[b] = 200;
	map["c"] = 300;
	map["d"] = 400;
	
	N_LOG(map[a]);
	N_LOG(map[b]);
	N_LOG(map["c"]);
	N_LOG(map["d"]);
	N_LOG("");
	
	const char* hello = "g++ -pthread -rdynamic -m64 ";
	size_t len = strlen(hello) + 1;
	N_LOG("%d", len);
	N_LOG(hello);

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
	
	N_LOG("K\n");
	
	GZip gzip("/tmp/test.gz");
	N_LOG(gzip.open());
	
	N_LOG(gzip.write(hello, uint(len - 1)));
	N_LOG(gzip.write("hello", 5));
	
	N_LOG("a\n");
	gzip.close();
	N_LOG(gzip.open(FOPEN_R));
	char str[100] = { 0 };
	N_LOG(gzip.read(str, 100));
	N_LOG(str);
	
	gzip.close();
	
	N_LOG("\nTEST zip\n");
	
	{
		ZipWriter writer("/tmp/test.zip");
		writer.open();
		N_LOG(writer.add_file("aa.txt"));
		N_LOG(writer.write(WeakBuffer("aa.txt", 6)));
		N_LOG(writer.add_file("bb.txt"));
		N_LOG(writer.write(WeakBuffer("bb.txt", 6)));
		N_LOG(writer.add_file("cc.txt"));
		N_LOG(writer.write(WeakBuffer("cc.txt", 6)));
		N_LOG(writer.add_file("dd.txt"));
		N_LOG(writer.write(WeakBuffer("dd.txt", 6)));
	}

	{
		ZipReader reader("/tmp/test.zip");
		reader.open();
		char str2[101] = { 0 };
		N_LOG(reader.read(str2, 100));
		N_LOG(str2);
		reader.next();
		N_LOG(reader.read(str2, 100));
		N_LOG(str2);
		reader.next();
		N_LOG(reader.read(str2, 100));
		N_LOG(str2);
		reader.first();
		N_LOG(reader.read(str2, 100));
		N_LOG(str2);
		//
		reader.jump("cc.txt");
		char str3[101] = { 0 };
		N_LOG(reader.read(str3, 3));
		N_LOG(str3);
		N_LOG(reader.exists("dd.txt"));
		N_LOG(reader.read(str3, 3));
		N_LOG(str3);
		N_LOG(reader.exists("kk.txt"));
		//
		N_LOG("next, %d", reader.next());
		N_LOG(reader.read(str2, 100));
		N_LOG(str2);
		N_LOG("next, %d", reader.next());
		N_LOG(reader.read(str2, 100));
		N_LOG(str2);
		N_LOG("next, %d", reader.next());
		N_LOG(reader.read(str2, 100));
		N_LOG(str2);
		N_LOG(reader.current());
	}
}
