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

#ifndef __ftr__js__ftr__
#define __ftr__js__ftr__

#include "ftr-js/js.h"
#include "ftr-js/wrap.h"
#include "ftr-js/value.h"
#include "ftr/app.h"
#include "ftr/view.h"

/**
 * @ns ftr::js
 */

JS_BEGIN

#define JS_CHECK_APP() if ( ! app() ) { \
	JS_WORKER(args); JS_THROW_ERR("Need to create a `new GUIApplication()`"); }

/**
 * @class WrapViewBase
 */
class FX_EXPORT WrapViewBase: public WrapObject {
 public:

	/**
	 * @func overwrite
	 */
	virtual void destroy();
	
	/**
	 * @func overwrite
	 */
	virtual bool addEventListener(cString& name, cString& func, int id);
	
	/**
	 * @func overwrite
	 */
	virtual bool removeEventListener(cString& name, int id);
	
	/**
	 * @func inheritTextFont
	 */
	static void inheritTextFont(Local<JSClass> cls, Worker* worker);
	
	/**
	 * @func inheritTextLayout
	 */
	static void inheritTextLayout(Local<JSClass> cls, Worker* worker);
	
	/**
	 * @func inheritScroll
	 */
	static void inheritScroll(Local<JSClass> cls, Worker* worker);
	
};

FX_EXPORT int Start(cString& cmd);
FX_EXPORT int Start(const Array<String>& argv);
FX_EXPORT int Start(int argc, char** argv);

JS_END
#endif
