// @private head
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


#ifndef __quark__util__jni__
#define __quark__util__jni__

#include "../macros.h"

#if Qk_ANDROID

#include "../object.h"
#include <jni.h>

namespace qk {

	/**
	* @class JNI
	*/
	class JNI {
	public:

		/**
		* @class ScopeENV
		*/
		class ScopeENV {
			Qk_HIDDEN_ALL_COPY(ScopeENV);
		public:
			ScopeENV();
			~ScopeENV();
			inline bool is_null() const { return _env == NULL; }
			inline JNIEnv* operator*() const { return _env; }
			inline JNIEnv* operator->() const { return _env; }
		 private:
			JNIEnv* _env;
			bool    _is_attach;
			// @end
		};

		/**
		* @class MethodInfo
		*/
		class MethodInfo {
		public:
			MethodInfo(cChar* clazz, cChar* method, cChar* param_code, bool is_static = false);
			MethodInfo(jclass clazz, cChar* method, cChar* param_code, bool is_static = false);
			inline jclass clazz() const { return _clazz; }
			inline jmethodID method() const { return _method; }
		private:
			jclass      _clazz;
			jmethodID   _method;
			// @end
		};

		/**
		* @method find_clazz
		* */
		static jclass find_clazz(cChar* clazz);

		/**
			* @method find_method
			*/
		inline static jmethodID find_method(jclass clazz, cChar* method, cChar* param_code) {
			return MethodInfo(clazz, method, param_code).method();
		}

		/**
		* @method find_static_method
		* */
		inline static jmethodID find_static_method(jclass clazz, cChar* method, cChar* param_code) {
			return MethodInfo(clazz, method, param_code, true).method();
		}

		/**
		* @method find_method
		* */
		inline static jmethodID find_method(cChar* clazz, cChar* method, cChar* param_code) {
			return MethodInfo(clazz, method, param_code).method();
		}

		/**
		* @method find_static_method
		* */
		static jmethodID find_static_method(cChar* clazz, cChar* method, cChar* param_code) {
			return MethodInfo(clazz, method, param_code, true).method();
		}

		/**
		* @method jvm
		*/
		static JavaVM* jvm();

		/**
		* @method jstring_to_string
		*/
		static String jstring_to_string(jstring jstr, JNIEnv* env = NULL);
	};

}
#endif
#endif