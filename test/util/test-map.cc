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

#include <src/util/array.h>
#include <src/util/list.h>
#include <src/util/string.h>
#include <src/util/dict.h>
#include <map>
#include "../test.h"

using namespace qk;

Qk_TEST_Func(map) {
	
	std::map<int, String> m;
	
	m[0] = "-100900978";
	m[100] = "-0";
	
	Qk_Log(m[0]);
	Qk_Log(m[100]);
	
	m.erase(100);
	
	Qk_Log(m[0]);
	
	Qk_Log("%d", m.size());
	
	auto begin = m.begin();
	
	Qk_Log("%d", sizeof(decltype(begin)) );
	
	Qk_Log(begin->second.c_str());
	
	begin++;
	
	Qk_Log(begin->second);
	
	Qk_Log(m[0]);
	Qk_Log(m[100]);
	
	Dict<String, String> map;
	
	map.find("AA");
	
	map.set("AA1", "BB");
	map.set("AA2", "BB1");
	map.set("AA3", "BB2");
	map.set("AA4", "BB3");
	map.set("AA5", "BB4");
	map.set("AA6", "BB5");
	map.set("AA7", "BB6");
	map.set("AA8", "BB7");
	map.set("AA9", "BB8");
	map.set("AA0", "BB9");
	
	Qk_Log(map["AA8"]);
	Qk_Log(map["AA4"]);
	Qk_Log(map["AA7"]);
	
	for (uint32_t i = 0; i < 10000; i++) {
		map.set(i, i);
	}
	
	Qk_Log(map.length());
	
	map.erase(String("AA1"));
	// map.mark("AAA7");

	auto i = map.begin();
	auto end = map.end();
	
	// map.mark(i);
	
	map.erase(i);
	
	i = map.begin();
	
	int j = 0;
	
	for (; i != end; i++) {
		Qk_Log(i->second);
		Qk_Log(j);
		j++;
	}
	
	Dict<String, String> map2(std::move(map));
	
	for ( i = map.begin(); i != end; i++) {
		Qk_Log(i->second);
	}
	
	i = map2.begin();
	end = map2.end();
	
	for ( ; i != end; i++) {
		Qk_Log(i->second);
	}
	
	map2 = map;
	
}
