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

#include <android/api-level.h>
#include "./android-jni.h"
#include "./android-log.h"
#include "./string.h"
#include "./loop.h"

static JavaVM* javavm = nullptr;

namespace noug {

	// ------------------- JNI -------------------

	// 类型签名
	// javap -s -private classname
	/*
		boolean Z
		byte	B
		Char	C
		short	S
		int		I
		long	J
		float	F
		double	D
		void	V
		Object	Ljava/lang/String;
		Arrat	[Ljava/lang/String;
	{	
	JniMethodInfo info;
	get_static_method_info(info, "com/noug/Helper", "get_package_code_path", "()Ljava/lang/String;");
	}
	*/

	JNI::ScopeENV::ScopeENV(): _env(NULL), _is_attach(false) {
		N_Asset( javavm );
		
		if (  javavm->GetEnv((void**)&_env, JNI_VERSION_1_6) != JNI_OK ) {
			jint result;
			result = javavm->AttachCurrentThread(&_env, NULL);
			N_Asset( result == JNI_OK );
			_is_attach = true;
		}
	}

	JNI::ScopeENV::~ScopeENV() {
		if ( _is_attach ) {
			javavm->DetachCurrentThread();
		}
	}

	JavaVM* JNI::jvm() {
		return javavm;
	}

	JNI::MethodInfo::MethodInfo(cChar* clazz, cChar* method, cChar* param_code, bool is_static)
		: _clazz(NULL)
		, _method(NULL)
	{
		ScopeENV env;

		if ( !env.is_null() ) {
			_clazz = env->FindClass(clazz);
			if ( is_static ) {
				_method = env->GetStaticMethodID(_clazz, method, param_code);
			} else {
				_method = env->GetMethodID(_clazz, method, param_code);
			}
		}
	}

	JNI::MethodInfo::MethodInfo(jclass clazz, cChar* method, cChar* param_code, bool is_static)
		: _clazz(clazz)
		, _method(NULL)
	{
		ScopeENV env;

		if ( !env.is_null() ) {
			if ( is_static ) {
				_method = env->GetStaticMethodID(_clazz, method, param_code);
			} else {
				_method = env->GetMethodID(_clazz, method, param_code);
			}
		}
	}

	/**
	* @func find_clazz
	* */
	jclass JNI::find_clazz(cChar* clazz) {
		ScopeENV env;
		if (env.is_null()) {
			return NULL;
		} else {
			return env->FindClass(clazz);
		}
	}

	String JNI::jstring_to_string(jstring jstr, JNIEnv* env) {

		if (jstr == NULL) {
			return String();
		}

		if ( env ) {
			cChar* Chars = env->GetStringUTFChars(jstr, NULL);
			String rv(Chars);
			env->ReleaseStringUTFChars(jstr, Chars);
			return rv;
		} else {
			
			ScopeENV env;
			if ( env.is_null() ) {
				return String();
			}
			cChar* Chars = env->GetStringUTFChars(jstr, NULL);
			jsize len = env->GetStringUTFLength(jstr);
			String rv(Chars, len);
			env->ReleaseStringUTFChars(jstr, Chars);
			return rv;
		}
	}

}

extern "C" 
{
	N_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
		javavm = vm;
		return JNI_VERSION_1_6;
	}
}