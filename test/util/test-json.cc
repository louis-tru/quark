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

#include <quark/util/json.h>
#include <quark/util/string.h>
#include <quark/util/event.h>
#include <map>

using namespace qk;

void test_json(int argc, char **argv) {
	
	String str1("100");
	String str2 = str1;
	
	Qk_Log("OK");
	
	cChar* json_str = "{ \"a\": \"ABCD\", \"b\": 100 }";
	
	JSON json = JSON::parse(json_str);
	
	Qk_Log(JSON::stringify(json));
	
	Qk_Log("short,%d", sizeof(short));
	Qk_Log("int,%d", sizeof(int));
	Qk_Log("long,%d", sizeof(long));
	Qk_Log("long int,%d", sizeof(long int));
	Qk_Log("long double,%d", sizeof(long double));
	
	Qk_Log("\n");
	
	Qk_Log("%d", sizeof(int));
	Qk_Log("%d", sizeof(int*));
	Qk_Log("%d", sizeof(long));
	//  LOG(sizeof(EventDelegate<>));
	
	std::map<String, cChar*> m;
	Qk_Log("%s", m["a"]);
	cChar*& a = m["a"];
	a = "110";
	Qk_Log("%s", m["a"]);
	
	const int* i = new int(101);
	
	Qk_Log(*i);
	
	delete i;
	Qk_Log(*i);
}
