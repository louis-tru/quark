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

#include "../sys.h"
#include "../string.h"
#include "../array.h"
#include <unistd.h>

XX_NS(ngui)
XX_NS(sys)

String version() {
	return String();
}

String brand() {
	return "Linux";
}

String subsystem() {
	static String name("Linux");
	return "";
}

int network_status() {
	return 1;
}

bool is_ac_power() {
	return 1;
}

bool is_battery() {
	return 0;
}

float battery_level() {
	return 0;
}

uint64 memory() {
	return 0;
}

uint64 used_memory() {
	return 0;
}

uint64 available_memory() {
	return 0;
}

float cpu_usage() {
	return 1;
}

struct Languages {
	Array<String> values;
	String				string;
};

static Languages _languages([] {
	Languages r;
	cchar* lang = getenv("LANG") ? getenv("LANG"): getenv("LC_ALL");
	if ( lang ) {
		r.values.push(String(lang).split('.')[0]);
	} else {
		r.values.push("en_US");
	}
	r.string = r.values.join(',');
	return r;
}());

const Array<String>& languages() {
	return _languages.values;
}

String languages_string() {
	return _languages.string;
}

String language() {
	return languages()[0];
}

// plus

String device_name() {
	return String();
}

XX_END XX_END
