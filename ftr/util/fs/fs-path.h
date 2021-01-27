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

#ifndef __ftr__util__fs__fs_path__
#define __ftr__util__fs__fs_path__

#include <ftr/util/string.h>

namespace ftr {

	/**
	* @class Path
	* @static
	*/
	class FX_EXPORT Path {
		public:
			/**
			* @func extname {String} # Get the path basename
			* @ret {String}
			*/
			static String basename(cString& path);
			
			/**
			* @func extname {String} # Get the path dirname
			* @arg path {cString&}
			* @ret {String}
			*/
			static String dirname(cString& path);
			
			/**
			* @func extname # Get the path extname
			* @arg path {cString&}
			* @ret {String}
			*/
			static String extname(cString& path);
			
			/**
			* @func executable_path # Get the executable path
			* @ret {cString&}
			*/
			static String executable();
			
			/**
			* @func documents_dir # Get the documents dir.
			* @ret {cString&} # The path that can be write/read a file in
			*/
			static SString documents(cString& child = String());
			
			/**
			* @func temp_dir # Get the temp dir.
			* @ret {cString&} # The path that can be write/read a file in
			*/
			static SString temp(cString& child = String());
			
			/**
			* @func resources_dir # Get the resoures dir
			* @ret {cString&}
			*/
			static SString resources(cString& child = String());
			
			/**
			* @func is_absolute # Is absolute path
			* @ret {bool}
			*/
			static bool is_local_absolute(cString& path);
			
			/**
			* @func is_local_zip
			*/
			static bool is_local_zip(cString& path);
			
			/**
			* @func is_local_file
			*/
			static bool is_local_file(cString& path);
			
			/**
			* @func format
			* @arg format {const char*}
			* @arg [...] {const char*}
			* @ret {String}
			*/
			static SString format(const char* path, ...);
			
			/**
			* @func format
			*/
			static SString format(cString& path);
			
			/**
			* @func fallback
			*/
			static String fallback(cString& path);
		
			/**
			* @func fallback_c
			*/
			static const char* fallback_c(cString& path);
			
			/**
			* @func cwd # Getting current working directory
			* @ret {String}
			* @static
			*/
			static String cwd();
			
			/**
			* @func chdir # Setting current working directory
			* @arg path {cString&}
			* @ret {bool}
			* @static
			*/
			static bool chdir(cString& path);
	};
}
#endif
