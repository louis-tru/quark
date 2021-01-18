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

#include "android.h"
#include "ftr/util/android-jni.h"

namespace ftr {

typedef JNI::MethodInfo MethodInfo;
typedef JNI::ScopeENV   ScopeENV;

class API {
 public:
	API() {
		ScopeENV env;
		clazz_              = JNI::find_clazz("org/ftr/Android");
		//clazz_build_        = JNI::find_clazz("android.os.Build");
		// utils
		version_            = JNI::find_static_method(clazz_, "version", "()Ljava/lang/String;");
		brand_              = JNI::find_static_method(clazz_, "brand", "()Ljava/lang/String;");
		subsystem_          = JNI::find_static_method(clazz_, "subsystem", "()Ljava/lang/String;");
		package_code_path_  = JNI::find_static_method(clazz_, "package_code_path", "()Ljava/lang/String;");
		cache_dir_path_     = JNI::find_static_method(clazz_, "cache_dir_path", "()Ljava/lang/String;");
		files_dir_path_     = JNI::find_static_method(clazz_, "files_dir_path", "()Ljava/lang/String;");
		// ftr
		ime_keyboard_open_  = JNI::find_static_method(clazz_, "ime_keyboard_open", "(ZII)V");
		ime_keyboard_can_backspace_ = JNI::find_static_method(clazz_, "ime_keyboard_can_backspace", "(ZZ)V");
		ime_keyboard_close_ = JNI::find_static_method(clazz_, "ime_keyboard_close", "()V");
		keep_screen_        = JNI::find_static_method(clazz_, "keep_screen", "(Z)V");
		get_status_bar_height_  = JNI::find_static_method(clazz_, "get_status_bar_height", "()I");
		set_visible_status_bar_ = JNI::find_static_method(clazz_, "set_visible_status_bar", "(Z)V");
		set_status_bar_style_  = JNI::find_static_method(clazz_, "set_status_bar_style", "(I)V");
		request_fullscreen_ = JNI::find_static_method(clazz_, "request_fullscreen", "(Z)V");
		get_orientation_    = JNI::find_static_method(clazz_, "get_orientation", "()I");
		set_orientation_    = JNI::find_static_method(clazz_, "set_orientation", "(I)V");
		get_display_scale_  = JNI::find_static_method(clazz_, "get_display_scale", "()F");
		is_screen_on_       = JNI::find_static_method(clazz_, "is_screen_on", "()Z");
		set_volume_up_      = JNI::find_static_method(clazz_, "set_volume_up", "()V");
		set_volume_down_    = JNI::find_static_method(clazz_, "set_volume_down", "()V");
		open_url_           = JNI::find_static_method(clazz_, "open_url", "(Ljava/lang/String;)V");
		send_email_         = JNI::find_static_method(clazz_, "send_email",
																									"(Ljava/lang/String;Ljava/lang/String;"
																									"Ljava/lang/String;Ljava/lang/String;"
																									"Ljava/lang/String;)V");
		start_cmd_         = JNI::find_static_method(clazz_, "start_cmd", "()Ljava/lang/String;");
		network_status_     = JNI::find_static_method(clazz_, "network_status", "()I");
		is_ac_power_        = JNI::find_static_method(clazz_, "is_ac_power", "()Z");
		is_battery_         = JNI::find_static_method(clazz_, "is_battery", "()Z");
		battery_level_      = JNI::find_static_method(clazz_, "battery_level", "()F");
		language_           = JNI::find_static_method(clazz_, "language", "()Ljava/lang/String;");
		available_memory_   = JNI::find_static_method(clazz_, "available_memory", "()J");
		memory_             = JNI::find_static_method(clazz_, "memory", "()J");
		used_memory_        = JNI::find_static_method(clazz_, "used_memory", "()J");
		clazz_ = (jclass)env->NewGlobalRef(clazz_);
		//clazz_build_ = (jclass)env->NewGlobalRef(clazz_build_);
	}

	~API() {
		ScopeENV env;
		env->DeleteGlobalRef(clazz_);
		// env->DeleteGlobalRef(clazz_build_);
	}

	//jclass    clazz_build_;
	jclass    clazz_;
	jmethodID ime_keyboard_open_;
	jmethodID ime_keyboard_can_backspace_;
	jmethodID ime_keyboard_close_;
	jmethodID keep_screen_;
	jmethodID get_status_bar_height_;
	jmethodID set_visible_status_bar_;
	jmethodID set_status_bar_style_;
	jmethodID request_fullscreen_;
	jmethodID get_orientation_;
	jmethodID set_orientation_;
	jmethodID get_display_scale_;
	jmethodID is_screen_on_;
	jmethodID set_volume_up_;
	jmethodID set_volume_down_;
	jmethodID open_url_;
	jmethodID send_email_;
	jmethodID start_cmd_;
	jmethodID package_code_path_;
	jmethodID files_dir_path_;
	jmethodID cache_dir_path_;
	jmethodID version_;
	jmethodID brand_;
	jmethodID subsystem_;
	jmethodID network_status_;
	jmethodID is_ac_power_;
	jmethodID is_battery_;
	jmethodID battery_level_;
	jmethodID language_;
	jmethodID available_memory_;
	jmethodID memory_;
	jmethodID used_memory_;
};

#define clazz_ _core->clazz_

static API* _core = nullptr;

void Android::initialize() {
	if ( !_core ) {
		_core = new API();
	}
}
// ------------------------------- gui -------------------------------
void Android::ime_keyboard_open(bool clear, int type, int return_type) {
	ScopeENV env;
	env->CallStaticVoidMethod(clazz_, _core->ime_keyboard_open_, clear, type, return_type);
}
void Android::ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
	ScopeENV env;
	env->CallStaticVoidMethod(clazz_, _core->ime_keyboard_can_backspace_, can_backspace, can_delete);
}
void Android::ime_keyboard_close() {
	ScopeENV env;
	env->CallStaticVoidMethod(clazz_, _core->ime_keyboard_close_);
}
void Android::keep_screen(bool value) {
	ScopeENV env;
	env->CallStaticVoidMethod(clazz_, _core->keep_screen_, value);
}
int Android::get_status_bar_height() {
	ScopeENV env;
	return env->CallStaticIntMethod(clazz_, _core->get_status_bar_height_);
}
void Android::set_visible_status_bar(bool visible) {
	ScopeENV env;
	env->CallStaticVoidMethod(clazz_, _core->set_visible_status_bar_, visible);
}
void Android::set_status_bar_style(int style) {
	ScopeENV env;
	env->CallStaticVoidMethod(clazz_, _core->set_status_bar_style_, style);
}
void Android::request_fullscreen(bool fullscreen) {
	ScopeENV env;
	env->CallStaticVoidMethod(clazz_, _core->request_fullscreen_, fullscreen);
}
int Android::get_orientation() {
	ScopeENV env;
	return env->CallStaticIntMethod(clazz_, _core->get_orientation_);
}
void Android::set_orientation(int orientation) {
	ScopeENV env;
	env->CallStaticVoidMethod(clazz_, _core->set_orientation_, orientation);
}
float Android::get_display_scale() {
	ScopeENV env;
	return env->CallStaticFloatMethod(clazz_, _core->get_display_scale_);
}
bool Android::is_screen_on() {
	ScopeENV env;
	return env->CallStaticBooleanMethod(clazz_, _core->is_screen_on_);
}
void Android::set_volume_up() {
	ScopeENV env;
	env->CallStaticVoidMethod(clazz_, _core->set_volume_up_);
}
void Android::set_volume_down() {
	ScopeENV env;
	env->CallStaticVoidMethod(clazz_, _core->set_volume_down_);
}
void Android::open_url(cString& url) {
	ScopeENV env;
	env->CallStaticVoidMethod(clazz_, _core->open_url_, env->NewStringUTF(*url));
}
void Android::send_email(cString& recipient,
												 cString& subject,
												 cString& cc, cString& bcc, cString& body) {
	ScopeENV env;
	env->CallStaticVoidMethod(clazz_, _core->send_email_,
														env->NewStringUTF(*recipient),
														env->NewStringUTF(*subject),
														env->NewStringUTF(*cc),
														env->NewStringUTF(*bcc),
														env->NewStringUTF(*body)
	);
}
// ------------------------------- util -------------------------------
String Android::start_cmd() {
	ScopeENV env;
	jobject obj = env->CallStaticObjectMethod(clazz_, _core->start_cmd_);
	return JNI::jstring_to_string((jstring)obj, *env);
}
String Android::package_code_path() {
	ScopeENV env;
	jobject obj = env->CallStaticObjectMethod(clazz_, _core->package_code_path_);
	return JNI::jstring_to_string((jstring)obj, *env);
}
String Android::files_dir_path() {
	ScopeENV env;
	jobject obj = env->CallStaticObjectMethod(clazz_, _core->files_dir_path_);
	return JNI::jstring_to_string((jstring)obj, *env);
}
String Android::cache_dir_path() {
	ScopeENV env;
	jobject obj = env->CallStaticObjectMethod(clazz_, _core->cache_dir_path_);
	return JNI::jstring_to_string((jstring)obj, *env);
}
String Android::version() {
	ScopeENV env;
	jobject obj = env->CallStaticObjectMethod(clazz_, _core->version_);
	return JNI::jstring_to_string((jstring)obj, *env);
}
String Android::brand() {
	ScopeENV env;
	jobject obj = env->CallStaticObjectMethod(clazz_, _core->brand_);
	return JNI::jstring_to_string((jstring)obj, *env);
}
String Android::subsystem() {
	ScopeENV env;
	jobject obj = env->CallStaticObjectMethod(clazz_, _core->subsystem_);
	return JNI::jstring_to_string((jstring)obj, *env);
}
int Android::network_status() {
	ScopeENV env;
	return env->CallStaticIntMethod(clazz_, _core->network_status_);
}
bool Android::is_ac_power() {
	ScopeENV env;
	return env->CallStaticBooleanMethod(clazz_, _core->is_ac_power_);
}
bool Android::is_battery() {
	ScopeENV env;
	return env->CallStaticBooleanMethod(clazz_, _core->is_battery_);
}
float Android::battery_level() {
	ScopeENV env;
	return env->CallStaticFloatMethod(clazz_, _core->battery_level_);
}
String Android::language() {
	ScopeENV env;
	return JNI::jstring_to_string((jstring)env->CallStaticObjectMethod(clazz_, _core->language_), *env);
}
uint64 Android::available_memory() {
	ScopeENV env;
	return env->CallStaticLongMethod(clazz_, _core->available_memory_);
}
uint64 Android::memory() {
	ScopeENV env;
	return env->CallStaticLongMethod(clazz_, _core->memory_);
}
uint64 Android::used_memory() {
	ScopeENV env;
	return env->CallStaticLongMethod(clazz_, _core->used_memory_);
}

}