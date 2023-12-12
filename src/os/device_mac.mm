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

#import "./device.h"
#import <quark/util/string.h>
#import <quark/util/handle.h>
#import <Foundation/Foundation.h>
#if Qk_iOS
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#import <IOKit/ps/IOPowerSources.h>
#import <IOKit/ps/IOPSKeys.h>
#endif
#import <mach/vm_statistics.h>
#import <mach/mach_host.h>
#import <mach/host_info.h>
#import <mach/vm_page_size.h>
#import <mach/mach_init.h>
#import <Reachability.h>
#import <sys/sysctl.h>
#import <mach/mach.h>

namespace qk {

	String device_brand() {
		return "Apple";
	}

#if Qk_iOS
	String device_system_version() {
		return UIDevice.currentDevice.systemVersion.UTF8String;
	}

	String device_model() {
		return UIDevice.currentDevice.model.UTF8String;
	}
#else
	String device_system_version() {
		return String();
	}

	String device_model() {
		static String name("MacOSX");
		return name;
	}
#endif

	void device_get_languages_mac(Array<String>& langs) {
		NSArray* languages = [NSLocale preferredLanguages];
		for ( int i = 0; i < [languages count]; i++ ) {
			NSString* str = [languages objectAtIndex:0];
			langs.push([str UTF8String]);
		}
	}

	int device_network_status() {
		Reachability* reachability = [Reachability reachabilityWithHostName:@"www.apple.com"];
		int code = [reachability currentReachabilityStatus];
		if ( code == 1 ) { // wwan
			return 3; // mobile network
		}
		return code;
	}

#if Qk_iOS
	bool device_is_ac_power() {
		[UIDevice currentDevice].batteryMonitoringEnabled = YES;
		UIDeviceBatteryState state = [UIDevice currentDevice].batteryState;
		if ( state == UIDeviceBatteryStateFull ||
				state == UIDeviceBatteryStateCharging ) { // 充电状态
			return 1;
		}
		return 0;
	}

	bool device_is_battery() {
		return 1;
	}

	float device_battery_level() {
		[UIDevice currentDevice].batteryMonitoringEnabled = YES;
		return [UIDevice currentDevice].batteryLevel;
	}
#else
	bool device_is_ac_power() {
		CFTypeRef blob = IOPSCopyPowerSourcesInfo();
		CFArrayRef sources = IOPSCopyPowerSourcesList(blob);
		CPointerHold<void> clear(nullptr, [=](void* _) {
			CFRelease(blob);
			CFRelease(sources);
		});

		int numOfSources = CFArrayGetCount(sources);
		//Calculating the remaining energy
		for (int i = 0 ; i < numOfSources ; i++) {
			CFDictionaryRef pSource =
				IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(sources, i));
			if (!pSource) {
				NSLog(@"Error in IOPSGetPowerSourceDescription");
				return 0;
			}
			const void *psValue;
			Char buf[32] = {0};
			psValue = CFDictionaryGetValue(pSource, CFSTR(kIOPSPowerSourceStateKey));
			CFStringGetCString((CFStringRef)psValue, buf, 31, kCFStringEncodingUTF8);
			return strcmp("AC Power", buf) == 0;
		}
		return 1;
	}

	bool device_is_battery() {
		CFTypeRef blob = IOPSCopyPowerSourcesInfo();
		CFArrayRef sources = IOPSCopyPowerSourcesList(blob);
		int numOfSources = CFArrayGetCount(sources);
		CFRelease(blob);
		CFRelease(sources);
		return numOfSources;
	}

	float device_battery_level() {
		//Returns a blob of Power Source information in an opaque CFTypeRef.
		CFTypeRef blob = IOPSCopyPowerSourcesInfo();
		//Returns a CFArray of Power Source handles, each of type CFTypeRef.
		CFArrayRef sources = IOPSCopyPowerSourcesList(blob);
		//Returns the number of values currently in an array.
		CPointerHold<void> clear(nullptr, [=](void* _) {
			CFRelease(blob);
			CFRelease(sources);
		});
		
		int numOfSources = CFArrayGetCount(sources);
		//Calculating the remaining energy
		for (int i = 0 ; i < numOfSources ; i++) {
			//Returns a CFDictionary with readable information about the specific power source.
			CFDictionaryRef pSource =
				IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(sources, i));
			if (!pSource) {
				NSLog(@"Error in IOPSGetPowerSourceDescription");
				return -1.0f;
			}
			const void *psValue;
			int curCapacity = 0, maxCapacity = 0;
#if DEBUG
			Char buf[32] = {0};
			psValue = (CFStringRef)CFDictionaryGetValue(pSource, CFSTR(kIOPSNameKey));
			CFStringGetCString((CFStringRef)psValue, buf, 31, kCFStringEncodingUTF8);
			NSLog(@"kIOPSNameKey, %s", buf);
#endif
			psValue = CFDictionaryGetValue(pSource, CFSTR(kIOPSCurrentCapacityKey));
			CFNumberGetValue((CFNumberRef)psValue, kCFNumberSInt32Type, &curCapacity);
			psValue = CFDictionaryGetValue(pSource, CFSTR(kIOPSMaxCapacityKey));
			CFNumberGetValue((CFNumberRef)psValue, kCFNumberSInt32Type, &maxCapacity);
			return (float)curCapacity / (float)maxCapacity;
		}
		return -1.0f;
	}
#endif

	uint64_t device_memory() {
		return [NSProcessInfo processInfo].physicalMemory;
	}

	uint64_t device_used_memory() {
		struct task_basic_info info;
		mach_msg_type_number_t size = TASK_BASIC_INFO_COUNT;
		kern_return_t kerr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &size);
		return (kerr == KERN_SUCCESS) ? info.resident_size : 0; // size in bytes
	}

	uint64_t device_available_memory() {
		vm_statistics_data_t stats;
		mach_msg_type_number_t size = HOST_VM_INFO_COUNT;
		kern_return_t kerr = host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&stats, &size);
		if (kerr == KERN_SUCCESS) {
			return vm_page_size * stats.free_count; // + vm_page_size * stats.inactive_count;
		}
		return 0;
	}

	float device_cpu_usage() {
		kern_return_t kr;
		thread_array_t         thread_list;
		mach_msg_type_number_t thread_count;
		
		// get threads in the task
		kr = task_threads(mach_task_self(), &thread_list, &thread_count);
		if (kr != KERN_SUCCESS) {
			return -1;
		}
		
		float cpu_usage = 0;
		
		for (int j = 0; j < thread_count; j++) {
			thread_info_data_t     thinfo;
			mach_msg_type_number_t thread_info_count = THREAD_INFO_MAX;
			
			kr = thread_info(thread_list[j], THREAD_BASIC_INFO,
												(thread_info_t)thinfo, &thread_info_count);
			if (kr != KERN_SUCCESS) {
				return -1;
			}
			
			thread_basic_info_t basic_info_th = (thread_basic_info_t)thinfo;
			
			if (!(basic_info_th->flags & TH_FLAGS_IDLE)) {
				cpu_usage += basic_info_th->cpu_usage;
			}
		} // for each thread
		
		cpu_usage = cpu_usage / (float)TH_USAGE_SCALE;
		
		kr = vm_deallocate(mach_task_self(), (vm_offset_t)thread_list,
												thread_count * sizeof(thread_t));
		assert(kr == KERN_SUCCESS);
		
		return cpu_usage;
	}

}
