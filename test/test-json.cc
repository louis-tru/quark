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

#include <shark/utils/json.h>
#include <shark/utils/string.h>
#include <shark/utils/event.h>
#include <map>

using namespace shark;

void test_json(int argc, char **argv) {
	
	String str1("100");
	String str2 = str1;
	
	LOG("OK");
	
	cchar* json_str = "{ \"a\": \"ABCD\", \"b\": 100 }";
	
	JSON json = JSON::parse(json_str);
	
	LOG(JSON::stringify(json));
	
	LOG("short,%d", sizeof(short));
	LOG("int,%d", sizeof(int));
	LOG("long,%d", sizeof(long));
	LOG("long int,%d", sizeof(long int));
	LOG("long double,%d", sizeof(long double));
	
	LOG("\n");
	
	LOG(sizeof(int));
	LOG(sizeof(int*));
	LOG(sizeof(long));
//  LOG(sizeof(EventDelegate<>));
	
	std::map<String, cchar*> m;
	LOG("%s", m["a"]);
	cchar*& a = m["a"];
	a = "110";
	LOG("%s", m["a"]);
	
	const int* i = new int(101);
	
	LOG(*i);
	
	delete i;
	LOG(*i);
}
