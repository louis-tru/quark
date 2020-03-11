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

#include <nxkit/array.h>
#include <nxkit/list.h>
#include <nxkit/string.h>
#include <nxkit/map.h>
#include <map>

using namespace ngui;

void test_map(int argc, char **argv) {
	
	std::map<int, String> m;
	
	m[0] = "-100900978";
	m[100] = "-0";
	
	LOG(m[0]);
	LOG(m[100]);
	
	m.erase(100);
	
	LOG(m[0]);
	
	LOG(m.size());
	
	auto begin = m.begin();
	
	LOG( sizeof(decltype(begin)) );
	
	LOG(begin->second);
	
	begin++;
	
	LOG(begin->second);
	
	LOG(m[0]);
	LOG(m[100]);
	
	Map<String, String> map;
	
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
	
	LOG(map["AA8"]);
	LOG(map["AA4"]);
	LOG(map["AA7"]);
	
	for (uint i = 0; i < 10000; i++) {
		map.set(i, i);
	}
	
	LOG(map.length());
	
	map.del("AA1");
	map.mark("AAA7");
	
	auto i = map.begin();
	auto end = map.end();
	
	map.mark(i);
	
	map.del(i);
	
	i = map.begin();
	
	int j = 0;
	
	for (; i != end; i++) {
		LOG(i.value());
		LOG(j);
		j++;
	}
	
	Map<String, String> map2(move(map));
	
	for ( i = map.begin(); i != end; i++) {
		LOG(i.value());
	}
	
	i = map2.begin();
	end = map2.end();
	
	for ( ; i != end; i++) {
		LOG(i.value());
	}
	
	map2 = map;
	
}
