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

#include <noug/util/json.h>
#include <noug/util/string.h>
#include <noug/util/event.h>
#include <map>

using namespace noug;

void test_json(int argc, char **argv) {
	
	String str1("100");
	String str2 = str1;
	
	F_LOG("OK");
	
	cChar* json_str = "{ \"a\": \"ABCD\", \"b\": 100 }";
	
	JSON json = JSON::parse(json_str);
	
	F_LOG(JSON::stringify(json));
	
	F_LOG("short,%d", sizeof(short));
	F_LOG("int,%d", sizeof(int));
	F_LOG("long,%d", sizeof(long));
	F_LOG("long int,%d", sizeof(long int));
	F_LOG("long double,%d", sizeof(long double));
	
	F_LOG("\n");
	
	F_LOG("%d", sizeof(int));
	F_LOG("%d", sizeof(int*));
	F_LOG("%d", sizeof(long));
	//  LOG(sizeof(EventDelegate<>));
	
	std::map<String, cChar*> m;
	F_LOG("%s", m["a"]);
	cChar*& a = m["a"];
	a = "110";
	F_LOG("%s", m["a"]);
	
	const int* i = new int(101);
	
	F_LOG(*i);
	
	delete i;
	F_LOG(*i);
}
