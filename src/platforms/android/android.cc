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

#include "./android_api.h"
#include "../../util/jni.h"

namespace qk {

	typedef JNI::MethodInfo MethodInfo;
	typedef JNI::ScopeENV   ScopeENV;

	struct Api_INL {
		//jclass    clazz_build;
		jclass    clazz;
		jmethodID ime_keyboard_open;
		jmethodID ime_keyboard_can_backspace;
		jmethodID ime_keyboard_close;
		jmethodID prevent_screen_sleep;
		jmethodID get_status_bar_height;
		jmethodID set_visible_status_bar;
		jmethodID set_status_bar_style;
		jmethodID set_fullscreen;
		jmethodID get_orientation;
		jmethodID set_orientation;
		jmethodID get_display_scale;
		jmethodID is_screen_on;
		jmethodID set_volume_up;
		jmethodID set_volume_down;
		jmethodID open_url;
		jmethodID send_email;
		jmethodID start_cmd;
		jmethodID version;
		jmethodID brand;
		jmethodID model;
		jmethodID network_status;
		jmethodID is_ac_power;
		jmethodID is_battery;
		jmethodID battery_level;
		jmethodID language;
		jmethodID available_memory;
		jmethodID memory;
		jmethodID used_memory;
		Api_INL() {
			ScopeENV env;
			clazz              = JNI::find_clazz("org/quark/Android");
			//clazz_build      = JNI::find_clazz("android.os.Build");
			version            = JNI::find_static_method(clazz, "version", "()Ljava/lang/String;");
			brand              = JNI::find_static_method(clazz, "brand", "()Ljava/lang/String;");
			model              = JNI::find_static_method(clazz, "model", "()Ljava/lang/String;");
			ime_keyboard_open  = JNI::find_static_method(clazz, "ime_keyboard_open", "(ZII)V");
			ime_keyboard_can_backspace = JNI::find_static_method(clazz, "ime_keyboard_can_backspace", "(ZZ)V");
			ime_keyboard_close = JNI::find_static_method(clazz, "ime_keyboard_close", "()V");
			prevent_screen_sleep = JNI::find_static_method(clazz, "prevent_screen_sleep", "(Z)V");
			get_status_bar_height = JNI::find_static_method(clazz, "get_status_bar_height", "()I");
			set_visible_status_bar = JNI::find_static_method(clazz, "set_visible_status_bar", "(Z)V");
			set_status_bar_style  = JNI::find_static_method(clazz, "set_status_bar_style", "(I)V");
			set_fullscreen = JNI::find_static_method(clazz, "set_fullscreen", "(Z)V");
			get_orientation    = JNI::find_static_method(clazz, "get_orientation", "()I");
			set_orientation    = JNI::find_static_method(clazz, "set_orientation", "(I)V");
			get_display_scale  = JNI::find_static_method(clazz, "get_display_scale", "()F");
			is_screen_on       = JNI::find_static_method(clazz, "is_screen_on", "()Z");
			set_volume_up      = JNI::find_static_method(clazz, "set_volume_up", "()V");
			set_volume_down    = JNI::find_static_method(clazz, "set_volume_down", "()V");
			open_url           = JNI::find_static_method(clazz, "open_url", "(Ljava/lang/String;)V");
			send_email         = JNI::find_static_method(clazz, "send_email",
																										"(Ljava/lang/String;Ljava/lang/String;"
																										"Ljava/lang/String;Ljava/lang/String;"
																										"Ljava/lang/String;)V");
			start_cmd          = JNI::find_static_method(clazz, "start_cmd", "()Ljava/lang/String;");
			network_status     = JNI::find_static_method(clazz, "network_status", "()I");
			is_ac_power        = JNI::find_static_method(clazz, "is_ac_power", "()Z");
			is_battery         = JNI::find_static_method(clazz, "is_battery", "()Z");
			battery_level      = JNI::find_static_method(clazz, "battery_level", "()F");
			language           = JNI::find_static_method(clazz, "language", "()Ljava/lang/String;");
			available_memory   = JNI::find_static_method(clazz, "available_memory", "()J");
			memory             = JNI::find_static_method(clazz, "memory", "()J");
			used_memory        = JNI::find_static_method(clazz, "used_memory", "()J");
			clazz              = (jclass)env->NewGlobalRef(clazz);
			//clazz_build      = (jclass)env->NewGlobalRef(clazzbuild);
		}

		~Api_INL() {
			ScopeENV env;
			env->DeleteGlobalRef(clazz);
			// env->DeleteGlobalRef(clazz_build);
		}
	};

	static Api_INL* __inl = nullptr;

	Api_INL* _inl() {
		if ( !__inl )
			__inl = new Api_INL();
		return __inl;
	}

	#define api _inl()
	#define clazz _inl()->clazz

	void Android::ime_keyboard_open(bool clear, int type, int return_type) {
		ScopeENV env;
		env->CallStaticVoidMethod(clazz, api->ime_keyboard_open, clear, type, return_type);
	}
	void Android::ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
		ScopeENV env;
		env->CallStaticVoidMethod(clazz, api->ime_keyboard_can_backspace, can_backspace, can_delete);
	}
	void Android::ime_keyboard_close() {
		ScopeENV env;
		env->CallStaticVoidMethod(clazz, api->ime_keyboard_close);
	}
	void Android::prevent_screen_sleep(bool prevent) {
		ScopeENV env;
		env->CallStaticVoidMethod(clazz, api->prevent_screen_sleep, prevent);
	}
	int Android::get_status_bar_height() {
		ScopeENV env;
		return env->CallStaticIntMethod(clazz, api->get_status_bar_height);
	}
	void Android::set_visible_status_bar(bool visible) {
		ScopeENV env;
		env->CallStaticVoidMethod(clazz, api->set_visible_status_bar, visible);
	}
	void Android::set_status_bar_style(int style) {
		ScopeENV env;
		env->CallStaticVoidMethod(clazz, api->set_status_bar_style, style);
	}
	void Android::set_fullscreen(bool fullscreen) {
		ScopeENV env;
		env->CallStaticVoidMethod(clazz, api->set_fullscreen, fullscreen);
	}
	int Android::get_orientation() {
		ScopeENV env;
		return env->CallStaticIntMethod(clazz, api->get_orientation);
	}
	void Android::set_orientation(int orientation) {
		ScopeENV env;
		env->CallStaticVoidMethod(clazz, api->set_orientation, orientation);
	}
	float Android::get_display_scale() {
		ScopeENV env;
		return env->CallStaticFloatMethod(clazz, api->get_display_scale);
	}
	bool Android::is_screen_on() {
		ScopeENV env;
		return env->CallStaticBooleanMethod(clazz, api->is_screen_on);
	}
	void Android::set_volume_up() {
		ScopeENV env;
		env->CallStaticVoidMethod(clazz, api->set_volume_up);
	}
	void Android::set_volume_down() {
		ScopeENV env;
		env->CallStaticVoidMethod(clazz, api->set_volume_down);
	}
	void Android::open_url(cString& url) {
		ScopeENV env;
		env->CallStaticVoidMethod(clazz, api->open_url, env->NewStringUTF(*url));
	}
	void Android::send_email(cString& recipient,
													cString& subject,
													cString& cc, cString& bcc, cString& body) {
		ScopeENV env;
		env->CallStaticVoidMethod(clazz, api->send_email,
															env->NewStringUTF(*recipient),
															env->NewStringUTF(*subject),
															env->NewStringUTF(*cc),
															env->NewStringUTF(*bcc),
															env->NewStringUTF(*body)
		);
	}

	String Android::start_cmd() {
		ScopeENV env;
		jobject obj = env->CallStaticObjectMethod(clazz, api->start_cmd);
		return JNI::jstring_to_string((jstring)obj, *env);
	}
	String Android::version() {
		ScopeENV env;
		jobject obj = env->CallStaticObjectMethod(clazz, api->version);
		return JNI::jstring_to_string((jstring)obj, *env);
	}
	String Android::brand() {
		ScopeENV env;
		jobject obj = env->CallStaticObjectMethod(clazz, api->brand);
		return JNI::jstring_to_string((jstring)obj, *env);
	}
	String Android::model() {
		ScopeENV env;
		jobject obj = env->CallStaticObjectMethod(clazz, api->model);
		return JNI::jstring_to_string((jstring)obj, *env);
	}
	int Android::network_status() {
		ScopeENV env;
		return env->CallStaticIntMethod(clazz, api->network_status);
	}
	bool Android::is_ac_power() {
		ScopeENV env;
		return env->CallStaticBooleanMethod(clazz, api->is_ac_power);
	}
	bool Android::is_battery() {
		ScopeENV env;
		return env->CallStaticBooleanMethod(clazz, api->is_battery);
	}
	float Android::battery_level() {
		ScopeENV env;
		return env->CallStaticFloatMethod(clazz, api->battery_level);
	}
	String Android::language() {
		ScopeENV env;
		return JNI::jstring_to_string((jstring)env->CallStaticObjectMethod(clazz, api->language), *env);
	}
	uint64 Android::available_memory() {
		ScopeENV env;
		return env->CallStaticLongMethod(clazz, api->available_memory);
	}
	uint64 Android::memory() {
		ScopeENV env;
		return env->CallStaticLongMethod(clazz, api->memory);
	}
	uint64 Android::used_memory() {
		ScopeENV env;
		return env->CallStaticLongMethod(clazz, api->used_memory);
	}

}