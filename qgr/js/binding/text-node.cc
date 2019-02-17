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

#include "qgr/js/js-1.h"
#include "qgr/js/qgr.h"
#include "qgr/text-node.h"

/**
 * @ns qgr::js
 */

JS_BEGIN

/**
 * @class WrapTextNode
 */
class WrapTextNode: public WrapViewBase {
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_CHECK_APP();
		New<WrapTextNode>(args, new TextNode());
	}
	
	static void length(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(TextNode);
		JS_RETURN( self->length() );
	}
	
	static void value(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(TextNode);
		JS_RETURN( self->value() );
	}
	
	static void set_value(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(TextNode);
		Ucs2String str = value->ToUcs2StringValue(worker);
		self->set_value(str);
	}

	static void text_hori_bearing(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(TextNode);
		JS_RETURN( self->text_hori_bearing() );
	}
	
	static void text_height(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(TextNode);
		JS_RETURN( self->text_height() );
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(TextNode, constructor, {
			JS_SET_CLASS_ACCESSOR(length, length);
			JS_SET_CLASS_ACCESSOR(value, value, set_value);
			JS_SET_CLASS_ACCESSOR(textHoriBearing, text_hori_bearing);
			JS_SET_CLASS_ACCESSOR(textHeight, text_height);
		}, Span);
		IMPL::js_class(worker)->set_class_alias(JS_TYPEID(TextNode), View::TEXT_NODE);
	}
};

void binding_text_node(Local<JSObject> exports, Worker* worker) {
	WrapTextNode::binding(exports, worker);
}

JS_END
