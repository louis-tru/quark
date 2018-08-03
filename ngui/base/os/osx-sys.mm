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

#import "../sys.h"
#import "../string.h"
#import "../array.h"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <mach/vm_statistics.h>
#import <mach/mach_host.h>
#import <mach/host_info.h>
#import <mach/vm_page_size.h>
#import <mach/mach_init.h>
#import <Reachability.h>

#import <sys/sysctl.h>
#import <mach/mach.h>

XX_NS(ngui)
XX_NS(sys)

String version() {
	return String();
}

String brand() {
	return "Apple";
}

String subsystem() {
	static String name("MacOSX");
	return name;
}

int network_status() {
	Reachability* reachability = [Reachability reachabilityWithHostName:@"www.apple.com"];
	int code = [reachability currentReachabilityStatus];
	if ( code == 1 ) { // wwan
		return 3; // mobile network
	}
	return code;
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
	return [NSProcessInfo processInfo].physicalMemory;
}

uint64 used_memory() {
	struct task_basic_info info;
	mach_msg_type_number_t size = TASK_BASIC_INFO_COUNT;
	kern_return_t kerr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &size);
	return (kerr == KERN_SUCCESS) ? info.resident_size : 0; // size in bytes
}

uint64 available_memory() {
	vm_statistics_data_t stats;
	mach_msg_type_number_t size = HOST_VM_INFO_COUNT;
	kern_return_t kerr = host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&stats, &size);
	if (kerr == KERN_SUCCESS) {
		return vm_page_size * stats.free_count; // + vm_page_size * stats.inactive_count;
	}
	return 0;
}

struct Languages {
	Array<String> values;
	String        string;
};

static Languages _languages([] {
	Languages r;
	NSArray* languages = [NSLocale preferredLanguages];
	for ( int i = 0; i < [languages count]; i++ ) {
		NSString* str = [languages objectAtIndex:0];
		r.values.push( [str UTF8String] );
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
