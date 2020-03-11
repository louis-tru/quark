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
#include "nxkit/android-log.h"
#include "nxkit/android-jni.h"
#include "android/android.h"
#include "nxkit/string.h"
#include "nxkit/loop.h"

static JavaVM* javavm = nullptr;

NX_NS(ngui)

// ------------------- JNI -------------------

// 类型签名
// javap -s -private classname
/*
	boolean Z
	byte	B
	char	C
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
get_static_method_info(info, "com/ngui/Helper", "get_package_code_path", "()Ljava/lang/String;");
}
 */

JNI::ScopeENV::ScopeENV(): m_env(NULL), m_is_attach(false) {
	NX_ASSERT( javavm );
	
	if (  javavm->GetEnv((void**)&m_env, JNI_VERSION_1_6) != JNI_OK ) {
		jint result;
		result = javavm->AttachCurrentThread(&m_env, NULL);
		NX_ASSERT( result == JNI_OK );
		m_is_attach = true;
	}
}

JNI::ScopeENV::~ScopeENV() {
	if ( m_is_attach ) {
		javavm->DetachCurrentThread();
	}
}

JavaVM* JNI::jvm() {
	return javavm;
}

JNI::MethodInfo::MethodInfo(cchar* clazz, cchar* method, cchar* param_code, bool is_static)
: m_clazz(NULL)
, m_method(NULL) {
	ScopeENV env;

	if ( !env.is_null() ) {
		m_clazz = env->FindClass(clazz);
		if ( is_static ) {
			m_method = env->GetStaticMethodID(m_clazz, method, param_code);
		} else {
			m_method = env->GetMethodID(m_clazz, method, param_code);
		}
	}
}

JNI::MethodInfo::MethodInfo(jclass clazz, cchar* method, cchar* param_code, bool is_static)
: m_clazz(clazz)
, m_method(NULL) {
	ScopeENV env;

	if ( !env.is_null() ) {
		if ( is_static ) {
			m_method = env->GetStaticMethodID(m_clazz, method, param_code);
		} else {
			m_method = env->GetMethodID(m_clazz, method, param_code);
		}
	}
}

/**
* @func find_clazz
* */
jclass JNI::find_clazz(cchar* clazz) {
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
		cchar* chars = env->GetStringUTFChars(jstr, NULL);
		String rv(chars);
		env->ReleaseStringUTFChars(jstr, chars);
		return rv;
	} else {
		
		ScopeENV env;
		if ( env.is_null() ) {
			return String();
		}
		cchar* chars = env->GetStringUTFChars(jstr, NULL);
		jsize len = env->GetStringUTFLength(jstr);
		String rv(chars, len);
		env->ReleaseStringUTFChars(jstr, chars);
		return rv;
	}
}

NX_END

extern "C" 
{
	NX_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
		javavm = vm;
		ngui::Android::initialize();
		(new ngui::AndroidConsole())->set_as_default();
		return JNI_VERSION_1_6;
	}
}