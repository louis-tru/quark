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

#include "../../util/fs.h"
#include "../js.h"

/**
 * @ns qk::js
 */

Js_BEGIN

/**
 * @class NativePath
 */
class NativePath {
	public:

	/**
	 * @func executable()
	 * @ret {String}
	 */
	static void executable(FunctionCall args) {
		Js_Worker(args);
		Js_Return( fs_executable() );
	}
	
	/**
	 * @func documents([path])
	 * @arg path {String}
	 * @ret {String}
	 */
	static void documents(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			Js_Return( fs_documents() );
		}
		Js_Return( fs_documents( args[0]->ToStringValue(worker)) );
	}
	
	/**
	 * @func temp([path])
	 * @arg path {String}
	 * @ret {String}
	 */
	static void temp(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			Js_Return( fs_temp() );
		}
		Js_Return( fs_temp( args[0]->ToStringValue(worker)) );
	}
	
	/**
	 * @func resources([path])
	 * @arg path {String}
	 * @ret {String}
	 */
	static void resources(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			Js_Return( fs_resources() );
		}
		Js_Return( fs_resources( args[0]->ToStringValue(worker)) );
	}

	/**
	 * @func cwd()
	 * @ret {String}
	 */
	static void cwd(FunctionCall args) {
		Js_Worker(args);
		Js_Return( fs_cwd() );
	}
	
	/**
	 * @func chdir(path)
	 * @arg path {String}
	 * @ret {bool}
	 */
	static void chdir(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0 || !args[0]->IsString(worker)) {
			Js_Return( false );
		}
		Js_Return( fs_chdir(args[0]->ToStringValue(worker)) );
	}

	static void extname(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0 || !args[0]->IsString(worker))
			Js_Throw( "Bad argument." );
		Js_Return( fs_extname( args[0]->ToStringValue(worker)) );
	}

	static void dirname(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0 || !args[0]->IsString(worker))
			Js_Throw( "Bad argument." );
		Js_Return( fs_dirname( args[0]->ToStringValue(worker)) );
	}

	static void basename(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0 || !args[0]->IsString(worker))
			Js_Throw( "Bad argument." );
		Js_Return( fs_basename( args[0]->ToStringValue(worker)) );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		Js_Set_Method(executable, executable);
		Js_Set_Method(documents, documents);
		Js_Set_Method(temp, temp);
		Js_Set_Method(resources, resources);
		Js_Set_Method(cwd, cwd);
		Js_Set_Method(chdir, chdir);
		Js_Set_Method(extname, extname); //
		Js_Set_Method(dirname, dirname);
		Js_Set_Method(basename, basename);
	}
};

Js_REG_MODULE(_path, NativePath);
Js_END
