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

#ifndef __qgr__js__qgr__
#define __qgr__js__qgr__

#include "qgr/js/js.h"
#include "qgr/js/wrap.h"
#include "qgr/js/value.h"
#include "qgr/app.h"
#include "qgr/view.h"

/**
 * @ns qgr::js
 */

JS_BEGIN

#define js_check_gui_app() if ( ! app() ) { \
	JS_WORKER(args); JS_THROW_ERR("Need to create a `new GUIApplication()`"); }

/**
 * @class WrapViewBase
 */
class XX_EXPORT WrapViewBase: public WrapObject {
 public:

	/**
	 * @func overwrite
	 */
	virtual void destroy();
	
	/**
	 * @func overwrite
	 */
	virtual bool add_event_listener(cString& name, cString& func, int id);
	
	/**
	 * @func overwrite
	 */
	virtual bool remove_event_listener(cString& name, int id);
	
	/**
	 * @func inherit_text_font
	 */
	static void inherit_text_font(Local<JSClass> cls, Worker* worker);
	
	/**
	 * @func inherit_text_layout
	 */
	static void inherit_text_layout(Local<JSClass> cls, Worker* worker);
	
	/**
	 * @func inherit_scroll
	 */
	static void inherit_scroll(Local<JSClass> cls, Worker* worker);
	
};

XX_EXPORT int start(cString& argv);
XX_EXPORT int start(const Array<String>& argv);
XX_EXPORT const Array<char*>* start_argv();

JS_END
#endif
