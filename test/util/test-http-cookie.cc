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

#include "src/util/http.h"
#include "src/os/os.h"
#include "../test.h"

using namespace qk;

static int64_t future_time() { return time_microsecond() + 86400LL * 1e6; }

static void reset_cookie() {
	http_clear_cookie();
}

/*────────────────────────────────────────────
1. 初始化测试：打开数据库、sessionId 行为
────────────────────────────────────────────*/
void test_init() {
	reset_cookie();

	auto v = http_get_cookie("example.com", "any", "/", false);
	Qk_TEST_EQ(v, "");

	// 再次 open 不应 crash
	(void)http_get_all_cookie("example.com", "/", false);

	Qk_Log("test_init");
}

/*────────────────────────────────────────────
2. 基本 set/get 行为
────────────────────────────────────────────*/
void test_basic() {
	reset_cookie();

	http_set_cookie("example.com", "a", "100", future_time(), "/", false);
	auto v = http_get_cookie("example.com", "a", "/", false);

	Qk_TEST_EQ(v, "100");

	Qk_TEST_EQ(http_get_cookie("example.com", "nope", "/", false), "");

	// 不同 domain 不互通
	Qk_TEST_EQ(http_get_cookie("abc.com", "a", "/", false), "");

	Qk_Log("test_basic");
}

/*────────────────────────────────────────────
3. path 匹配行为（浏览器规则）
────────────────────────────────────────────*/
void test_path_rule() {
	reset_cookie();

	http_set_cookie("example.com", "A", "1", future_time(), "/a/", false);
	http_set_cookie("example.com", "B", "2", future_time(), "/a/b/", false);

	Qk_TEST_EQ(http_get_cookie("example.com","A","/a/",false), "1");
	Qk_TEST_EQ(http_get_cookie("example.com","B","/a/b/",false), "2");

	// deeper path also inherits
	Qk_TEST_EQ(http_get_cookie("example.com","A","/a/b/",false), "1");
	Qk_TEST_EQ(http_get_cookie("example.com","B","/a/b/c",false), "2");

	// root path should NOT match
	Qk_TEST_EQ(http_get_cookie("example.com","A","/",false), "");

	Qk_Log("test_path_rule");
}

/*────────────────────────────────────────────
4. domain 匹配行为（子域匹配规则）
────────────────────────────────────────────*/
void test_domain_rule() {
	reset_cookie();

	// domain=.example.com
	http_set_cookie("example.com", "D", "xxx", future_time(), "/", false);
	// 变成 .example.com 的行为由上层保证，测试点在查询端

	Qk_TEST_EQ(http_get_cookie("example.com","D","/",false), "xxx");
	Qk_TEST_EQ(http_get_cookie("www.example.com","D","/",false), "xxx");
	Qk_TEST_EQ(http_get_cookie("a.b.example.com","D","/",false), "xxx");

	// 不相干域名
	Qk_TEST_EQ(http_get_cookie("evil.com","D","/",false), "");

	Qk_Log("test_domain_rule");
}

/*────────────────────────────────────────────
5. secure 行为
────────────────────────────────────────────*/
void test_secure() {
	reset_cookie();

	http_set_cookie("example.com", "S", "SEC", future_time(), "/", true);

	Qk_TEST_EQ(http_get_cookie("example.com","S","/",false), "");
	Qk_TEST_EQ(http_get_cookie("example.com","S","/",true),  "SEC");

	Qk_Log("test_secure");
}

/*────────────────────────────────────────────
6. session cookie 行为（expires = -1）
────────────────────────────────────────────*/
namespace qk {
	extern int64_t _cookieSessionId;  // cookie 里的 sessionId
}

void test_session_cookie() {
	reset_cookie();

	http_set_cookie("example.com", "X", "123", -1, "/", false);

	Qk_TEST_EQ(http_get_cookie("example.com","X","/",false), "123");

	// 模拟重启（sessionId 改变）
	_cookieSessionId = _cookieSessionId + 1;

	Qk_TEST_EQ(http_get_cookie("example.com","X","/",false), "");

	Qk_Log("test_session_cookie");
}

/*────────────────────────────────────────────
7. expires / max-age 行为
────────────────────────────────────────────*/
void test_expires() {
	reset_cookie();

	// 已过期
	http_set_cookie("example.com", "a", "1", time_microsecond() - 1000, "/", false);
	Qk_TEST_EQ(http_get_cookie("example.com","a","/",false), "");

	// 未过期
	http_set_cookie("example.com", "b", "2", time_microsecond() + 10000000, "/", false);
	Qk_TEST_EQ(http_get_cookie("example.com","b","/",false), "2");

	Qk_Log("test_expires");
}

/*────────────────────────────────────────────
8. 删除 cookie 行为
────────────────────────────────────────────*/
void test_delete_cookie() {
	reset_cookie();

	http_set_cookie("example.com","a","100",future_time(),"/",false);
	Qk_TEST_EQ(http_get_cookie("example.com","a","/",false), "100");

	http_delete_cookie("example.com","a","/",false);
	Qk_TEST_EQ(http_get_cookie("example.com","a","/",false), "");

	Qk_Log("test_delete_cookie");
}

/*────────────────────────────────────────────
9. 删除某个目录下所有 cookie
────────────────────────────────────────────*/
void test_delete_all_cookie() {
	reset_cookie();

	http_set_cookie("example.com","a","1",future_time(),"/app/",false);
	http_set_cookie("example.com","b","2",future_time(),"/app/v1/",false);
	http_set_cookie("example.com","c","3",future_time(),"/app/v1/x/",false);

	String allStr0 = http_get_all_cookie_string("example.com","/app/v1/x/",false);
	Qk_TEST_EQ(allStr0, "a=1; b=2; c=3");

	// 删除 app/v1/*
	http_delete_all_cookie("example.com","/app/v1/",false);

	auto all1 = http_get_all_cookie("example.com","/app/v1/",false);
	Qk_TEST_EQ(all1.length(), 1);

	auto a = http_get_cookie("example.com","a","/app/",false);
	Qk_TEST_EQ(a, "1");

	Qk_Log("test_delete_all_cookie");
}

/*────────────────────────────────────────────
10. get_all_cookie & get_all_cookie_string
────────────────────────────────────────────*/
void test_get_all() {
	reset_cookie();

	http_set_cookie("example.com","u","aaa",future_time(),"/",false);
	http_set_cookie("example.com","t","bbb",future_time(),"/",false);

	auto dict = http_get_all_cookie("example.com","/",false);
	Qk_TEST_EQ(dict.length(), 2);

	Qk_TEST_EQ(dict.get("u"), "aaa");
	Qk_TEST_EQ(dict.get("t"), "bbb");

	auto s = http_get_all_cookie_string("example.com","/",false);
	Qk_TEST_EQ(s, "t=bbb; u=aaa");

	Qk_Log("test_get_all");
}

// Additional Tests for Cookie Handling
/*────────────────────────────────────────────*/
void test_cookie_basic_set_get() {
	reset_cookie();

	// Initially empty
	Qk_TEST_EQ(http_get_cookie("quarks.cc", "test"), "");

	http_set_cookie("quarks.cc", "test", "quarks.cc");

	// A: exact domain
	Qk_TEST_EQ(http_get_cookie("quarks.cc", "test"), "quarks.cc");

	// B: subdomain inherits parent
	Qk_TEST_EQ(http_get_cookie("www.quarks.cc", "test"), "quarks.cc");
}

void test_cookie_subdomain_override() {
	reset_cookie();

	http_set_cookie("quarks.cc", "test", "quarks.cc");
	http_set_cookie("www.quarks.cc", "test", "$");

	// B2: subdomain override
	Qk_TEST_EQ(http_get_cookie("www.quarks.cc", "test"), "$");
}

void test_cookie_second_key_basic() {
	reset_cookie();

	http_set_cookie("quarks.cc", "test2", "*");

	// D: exact domain
	Qk_TEST_EQ(http_get_cookie("quarks.cc", "test2"), "*");

	// E: subdomain inherits
	Qk_TEST_EQ(http_get_cookie("www.quarks.cc", "test2"), "*");
}

void test_cookie_path_specific() {
	reset_cookie();

	http_set_cookie("quarks.cc", "test2", "*");
	http_set_cookie("quarks.cc", "test2", "-----------------------------", -1, "/AA");

	// F: base path version cannot override /AA version
	Qk_TEST_EQ(http_get_cookie("quarks.cc", "test2"), "*");

	// H: path-specific cookie
	Qk_TEST_EQ(http_get_cookie("quarks.cc", "test2", "/AA"), "-----------------------------");

	// /AA for subdomain www.quarks.cc
	Qk_TEST_EQ(http_get_all_cookie_string("www.quarks.cc", "/AA"),
							"test2=-----------------------------");
}

void test_cookie_expression_basic() {
	reset_cookie();

	String exp = "test3=HHHH--l; path=/AA; max-age=60";
	http_set_cookie_with_expression("quarks.cc", exp);

	// exact domain but at root: root does not match /AA
	Qk_TEST_EQ(http_get_cookie("quarks.cc", "test3"), "");

	// correct path
	Qk_TEST_EQ(http_get_cookie("quarks.cc", "test3", "/AA"), "HHHH--l");

	Qk_TEST_EQ(
			http_get_all_cookie_string("quarks.cc", "/AA"),
			"test3=HHHH--l"
	);
}

void test_cookie_delete() {
	reset_cookie();

	http_set_cookie("quarks.cc", "test", "quarks.cc");
	http_delete_cookie("quarks.cc", "test");

	// deleted
	Qk_TEST_EQ(http_get_cookie("quarks.cc", "test"), "");

	// multiple cookies
	http_set_cookie("quarks.cc", "test", "quarks.cc2");
	http_set_cookie("quarks.cc", "test9", "quarks.cc3");
	http_set_cookie("quarks.cc", "test8", "quarks.cc4");
	http_set_cookie("www.quarks.cc", "test7", "quarks.cc5");

	Qk_TEST_EQ(http_get_cookie("quarks.cc", "test"), "quarks.cc2");

	// unrelated domain insert
	http_set_cookie("quark.orh", "test--------A", "quarks.cc%", -1, "KKK/MMM");

	String all = http_get_all_cookie_string("quarks.cc");
	// test, test9, test8 exist — but we don't enforce order here
	Qk_TEST_EXPECT(all.indexOf("test=") != -1);
	Qk_TEST_EXPECT(all.indexOf("test8=") != -1);
	Qk_TEST_EXPECT(all.indexOf("test9=") != -1);
}

void test_cookie_delete_all() {
	reset_cookie();

	http_set_cookie("quarks.cc", "test", "quarks.cc");
	http_delete_all_cookie("quarks.cc");

	// all deleted
	Qk_TEST_EQ(http_get_cookie("quarks.cc", "test"), "");

	http_set_cookie("quarks.cc", "test", "quarks.cc");

	// unrelated domain should not affect
	Qk_TEST_EQ(http_get_cookie("quarks.cc",
															"test--------A", "KKK/MMM", 1),
							"");
}

void test_cookie_clear() {
	reset_cookie();

	http_set_cookie("quarks.cc", "test", "quarks.cc");
	http_clear_cookie();

	// cleared
	Qk_TEST_EQ(http_get_cookie("quarks.cc", "test"), "");
}

void test_cookie_with_expire() {
	reset_cookie();

	http_set_cookie(
			"quarks.cc",
			"test",
			"END test cookie",
			time_microsecond() + 6e7   // expires 60 seconds later
	);

	// still valid
	Qk_TEST_EQ(http_get_cookie("quarks.cc", "test"), "END test cookie");
}

/*────────────────────────────────────────────
11. Set-Cookie 解析测试（完整表达式解析）
────────────────────────────────────────────*/
void test_cookie_expression() {
	reset_cookie();

	String exp = "token=xyz; path=/; max-age=1; domain=example.com; secure";
	http_set_cookie_with_expression("example.com", exp);

	Qk_TEST_EQ(http_get_cookie("example.com", "token", "/", true), "xyz");

	Qk_Log("test_cookie_expression");
}

Qk_TEST_Func(http_cookie) {
	test_init();
	test_basic();
	test_path_rule();
	test_domain_rule();
	test_secure();
	test_session_cookie();
	test_expires();
	test_delete_cookie();
	test_delete_all_cookie();
	test_get_all();
	test_cookie_expression();
	test_cookie_basic_set_get();
	test_cookie_subdomain_override();
	test_cookie_second_key_basic();
	test_cookie_path_specific();
	test_cookie_expression_basic();
	test_cookie_delete();
	test_cookie_delete_all();
	test_cookie_clear();
	test_cookie_with_expire();
}
