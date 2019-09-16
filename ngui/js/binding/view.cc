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

#include "ngui/js/js-1.h"
#include "ngui/js/ngui.h"
#include "ngui/js/str.h"
#include "ngui/app.h"
#include "ngui/action.h"
#include "ngui/css.h"
#include "ngui/button.h"
#include "ngui/js/value.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

// ================= View ================

/**
 * @class WrapView
 */
class WrapView: public WrapViewBase {
 public:
	
	/**
	 * @constructor() 
	 */
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_CHECK_APP();
		New<WrapView>(args, new View());
	}
	
	/**
	 * @func prepend(child) 
	 * @arg child {View}
	 */
	static void prepend(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if (args.Length() < 1 || ! worker->has_instance(args[0], View::VIEW)) {
			JS_THROW_ERR(
				"* @func prepend(child)\n"
				"* @arg child {View}\n"
			);
		}
		JS_SELF(View);
		View* child = Wrap<View>::unpack(args[0].To<JSObject>())->self();
		try { self->prepend(child); }
		catch (cError& err) { JS_THROW_ERR(err); }
	}

	/**
	 * @func append(child)
	 * @arg child {View}
	 */
	static void append(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if (args.Length() < 1 || ! worker->has_instance(args[0], View::VIEW)) {
			JS_THROW_ERR(
				"* @func append(child)\n"
				"* @arg child {View}\n"
			);
		}
		JS_SELF(View);
		View* child = unpack<View>(args[0].To<JSObject>())->self();
		try { self->append(child); }
		catch (cError& err) { JS_THROW_ERR(err); }
	}

	/**
	 * @func appendText(text)
	 * @arg text {String}
	 * @ret {View}
	 */
	static void append_text(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 ) {
			JS_THROW_ERR(
				"* @func appendText(text)\n"
				"* @arg text {String}\n"
				"* @ret {View}\n"
			);
		}
		JS_SELF(View);
		View* view = nullptr;
		
		JS_TRY_CATCH({
			view = self->append_text( args[0]->ToUcs2StringValue(worker) );
		}, Error);
		
		if (view) {
			Wrap<View>* wrap = Wrap<View>::pack(view, view->view_type());
			JS_RETURN( wrap->that() );
		} else {
			JS_RETURN( worker->NewNull() );
		}
	}

	/**
	 * @func before(prev)
	 * @arg prev {View}
	 */
	static void before(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if (args.Length() < 1 || ! worker->has_instance(args[0], View::VIEW)) {
			JS_THROW_ERR(
				"* @func before(prev)\n"
				"* @arg prev {View}\n"
			);
		}
		JS_SELF(View);
		View* brother = Wrap<View>::unpack(args[0].To())->self();
		try { self->before(brother); }
		catch (cError& err) { JS_THROW_ERR(err); }
	}

	/**
	 * @func after(next)
	 * @arg next {View}
	 */
	static void after(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if (args.Length() < 1 || !worker->has_view(args[0])) {
			JS_THROW_ERR(
				"* @func after(next)\n"
				"* @arg next {View}\n"
			);
		}
		JS_SELF(View);
		View* brother = Wrap<View>::unpack(args[0].To())->self();
		try { self->after(brother); }
		catch (cError& err) { JS_THROW_ERR(err); }
	}

	/**
	 * @func remove()
	 */
	static void remove(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		self->remove();
	}

	/**
	 * @func removeAllChild()
	 */
	static void remove_all_child(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		self->remove_all_child();
	}

	/**
	 * @func focus()
	 * @ret {bool}
	 */
	static void focus(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		JS_RETURN( self->focus() );
	}

	/**
	 * @func blur()
	 * @ret {bool}
	 */
	static void blur(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		JS_RETURN( self->blur() );
	}

	/**
	 * @func layoutOffset()
	 * @ret {Vec2}
	 */
	static void layout_offset(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		Vec2 rev = self->layout_offset();
		JS_RETURN( worker->values()->New(rev) );
	}
	
	/**
	 * @func layoutOffsetFrom([upper])
	 * @arg [upper=parent] {View}
	 * @ret {Vec2}
	 */
	static void layout_offset_from(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		View* target = nullptr;
		if ( args.Length() > 0 && worker->has_view(args[0]) ) {
			target = Wrap<View>::unpack(args[0].To())->self();
		}
		JS_SELF(View);
		Vec2 rev = self->layout_offset_from(target);
		JS_RETURN( worker->values()->New(rev) );
	}

	/**
	 * @func getAction()
	 * @ret {Action}
	 */
	static void get_action(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		Action* action = self->action();
		if ( action ) {
			JS_RETURN( Wrap<Action>::pack(action)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}

	/**
	 * @func setAction([action])
	 * @arg [action=null] {Action}
	 */
	static void set_action(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		Action* action = nullptr;
		
		if ( args.Length() > 0 ) {
			if (worker->has_instance<Action>(args[0])) {
				action = unpack<Action>(args[0].To())->self();
			} else if ( !args[0]->IsNull(worker) ) {
				JS_THROW_ERR(
											"* @func setAction([action])\n"
											"* @arg [action=null] {Action}\n"
											);
			}
		}
		
		JS_SELF(View);
		
		JS_TRY_CATCH({
			self->action(action);
		}, Error)
	}

	/**
	 * @func screenRect()
	 * @ret {Rect}
	 */
	static void screen_rect(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		JS_RETURN( worker->values()->New(self->screen_rect()) );
	}

	/**
	 * @func finalMatrix()
	 * @ret {Mat}
	 */
	static void final_matrix(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		JS_RETURN( worker->values()->New(self->final_matrix()) );
	}

	/**
	 * @func finalOpacity()
	 * @ret {float}
	 */
	static void final_opacity(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		JS_RETURN( self->final_opacity() );
	}

	/**
	 * @func position()
	 * @ret {Vec2}
	 */
	static void position(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		JS_RETURN( worker->values()->New(self->position()) );
	}

	/**
	 * @func overlapTest(point)
	 * @arg point {Vec2}
	 * @ret {bool}
	 */
	static void overlap_test(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 ) {
			JS_THROW_ERR(
				"* @func overlapTest(point)\n"
				"* @arg point {Vec2}\n"
				"* @ret {bool}\n"
			);
		}
		js_parse_value(Vec2, args[0], "View.overlapTest( %s )");
		JS_SELF(View);
		JS_RETURN( self->overlap_test(out) );
	}

	/**
	 * @func addClass(name)
	 * @arg name {String}
	 */
	static void add_class(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 || !args[0]->IsString(worker) ) {
			JS_THROW_ERR(
				"* @func addClass(name)\n"
				"* @arg name {String}\n"
			);
		}
		JS_SELF(View);
		self->add_class( args[0]->ToStringValue(worker) );
	}

	/**
	 * @func removeClass(name)
	 * @arg name {String}
	 */
	static void remove_class(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 || ! args[0]->IsString(worker) ) {
			JS_THROW_ERR(
				"* @func removeClass(name)\n"
				"* @arg name {String}\n"
			);
		}
		JS_SELF(View);
		self->remove_class( args[0]->ToStringValue(worker) );
	}

	/**
	 * @func toggleClass(name)
	 * @arg name {String}
	 */
	static void toggle_class(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 || ! args[0]->IsString(worker) ) {
			JS_THROW_ERR(
				"* @func toggleClass(name)\n"
				"* @arg name {String}\n"
			);
		}
		JS_SELF(View);
		self->toggle_class( args[0]->ToStringValue(worker) );
	}
	
	// ----------------------------- get --------------------------------

	/**
	 * @get innerText {String}
	 */
	static void inner_text(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->inner_text() );
	}

	/**
	 * @get parent {View}
	 */
	static void parent(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		View* view = self->parent();
		if ( ! view) JS_RETURN( worker->NewNull() );
		Wrap<View>* wrap = Wrap<View>::pack(view, view->view_type());
		JS_RETURN( wrap->that() );
	}

	/**
	 * @get prev {View}
	 */
	static void prev(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		View* view = self->prev();
		if ( ! view) JS_RETURN( worker->NewNull() );
		Wrap<View>* wrap = Wrap<View>::pack(view, view->view_type());
		JS_RETURN( wrap->that() );
	}

	/**
	 * @get next {View}
	 */
	static void next(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		View* view = self->next();
		if ( ! view) JS_RETURN( worker->NewNull() );
		Wrap<View>* wrap = Wrap<View>::pack(view, view->view_type());
		JS_RETURN( wrap->that() );
	}

	/**
	 * @get first {View}
	 */
	static void first(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		View* view = self->first();
		if ( ! view) JS_RETURN( worker->NewNull() );
		Wrap<View>* wrap = Wrap<View>::pack(view, view->view_type());
		JS_RETURN( wrap->that() );
	}

	/**
	 * @get last {View}
	 */
	static void last(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		View* view = self->last();
		if ( ! view) JS_RETURN( worker->NewNull() );
		Wrap<View>* wrap = Wrap<View>::pack(view, view->view_type());
		JS_RETURN( wrap->that() );
	}

	/**
	 * @get x {float}
	 */
	static void x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->x() );
	}

	/**
	 * @get y {float}
	 */
	static void y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->y() );
	}

	/**
	 * @get scaleX {float}
	 */
	static void scale_x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->scale_x() );
	}

	/**
	 * @get scaleY {float}
	 */
	static void scale_y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->scale_y() );
	}

	/**
	 * @get rotateZ {float}
	 */
	static void rotate_z(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->rotate_z() );
	}

	/**
	 * @get skewX {float}
	 */
	static void skew_x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->skew_x() );
	}

	/**
	 * @get skewY {float}
	 */
	static void skew_y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->skew_y() );
	}

	/**
	 * @get opacity {float}
	 */
	static void opacity(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->opacity() );
	}

	/**
	 * @get visible {bool}
	 */
	static void visible(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->visible() );
	}

	/**
	 * @get finalVisible {bool}
	 */
	static void final_visible(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->final_visible() );
	}

	/**
	 * @get drawVisible {bool}
	 */
	static void draw_visible(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->draw_visible() );
	}

	/**
	 * @get translate {Vec2}
	 */
	static void translate(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( worker->values()->New(self->translate()) );
	}

	/**
	 * @get scale {Vec2}
	 */
	static void scale(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( worker->values()->New(self->scale()) );
	}

	/**
	 * @get skew {Vec2}
	 */
	static void skew(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( worker->values()->New(self->skew()) );
	}

	/**
	 * @get originX {float}
	 */
	static void origin_x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->origin_x() );
	}

	/**
	 * @get originY {float}
	 */
	static void origin_y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->origin_y() );
	}

	/**
	 * @get origin {Vec2}
	 */
	static void origin(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( worker->values()->New(self->origin()) );
	}

	/**
	 * @get matrix {Mat}
	 */
	static void matrix(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		JS_RETURN( worker->values()->New(self->matrix()) );
	}

	/**
	 * @get level {uint}
	 */
	static void level(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->level() );
	}

	/**
	 * @get needDraw {bool}
	 */
	static void need_draw(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->need_draw() );
	}

	/**
	 * @get receive {bool}
	 */
	static void receive(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->receive() );
	}

	/**
	 * @get isFocus {bool}
	 */
	static void is_focus(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->is_focus() );
	}

	/**
	 * @get viewType {uint}
	 */
	static void view_type(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		JS_RETURN( self->view_type() );
	}

	/**
	 * @get classs {Object}
	 */
	static void classs(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		CSSViewClasss* classs = self->classs();
		if ( classs ) {
			Local<JSValue> rv = worker->NewObject();
			rv.To<JSObject>()->Set(worker, worker->strs()->name(), worker->New(classs->name()) );
			JS_RETURN( rv );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	// ------------------------------------ set ----------------------------------

	/**
	 * @set x {float}
	 */
	static void set_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(View);
		self->set_x( value->ToNumberValue(worker) );
	}

	/**
	 * @set y {float}
	 */
	static void set_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(View);
		self->set_y( value->ToNumberValue(worker) );
	}

	/**
	 * @set scaleX {float}
	 */
	static void set_scale_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(View);
		self->set_scale_x( value->ToNumberValue(worker) );
	}

	/**
	 * @set scaleY {float}
	 */
	static void set_scale_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(View);
		self->set_scale_y( value->ToNumberValue(worker) );
	}

	/**
	 * @set rotateZ {float}
	 */
	static void set_rotate_z(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(View);
		self->set_rotate_z( value->ToNumberValue(worker) );
	}

	/**
	 * @set skewX {float}
	 */
	static void set_skew_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(View);
		self->set_skew_x( value->ToNumberValue(worker) );
	}

	/**
	 * @set skewY {float}
	 */
	static void set_skew_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(View);
		self->set_skew_y( value->ToNumberValue(worker) );
	}

	/**
	 * @set opacity {float}
	 */
	static void set_opacity(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(View);
		self->set_opacity( value->ToNumberValue(worker) );
	}

	/**
	 * @set translate {Vec2}
	 */
	static void set_translate(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Vec2, value, "View.translate = %s");
		JS_SELF(View);
		self->set_translate( out );
	}

	/**
	 * @set scale {Vec2}
	 */
	static void set_scale(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Vec2, value, "View.scale = %s");
		JS_SELF(View);
		self->set_scale( out );
	}

	/**
	 * @set skew {Vec2}
	 */
	static void set_skew(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Vec2, value, "View.skew = %s");
		JS_SELF(View);
		self->set_skew( out );
	}

	/**
	 * @set originX {float}
	 */
	static void set_origin_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(View);
		self->set_origin_x( value->ToNumberValue(worker) );
	}

	/**
	 * @set originY {float}
	 */
	static void set_origin_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(View);
		self->set_origin_y( value->ToNumberValue(worker) );
	}

	/**
	 * @set origin {Vec2}
	 */
	static void set_origin(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Vec2, value, "View.origin = %s");
		JS_SELF(View);
		self->set_origin( out );
	}

	/**
	 * @set visible {bool}
	 */
	static void set_visible(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		self->set_visible( value->ToBooleanValue(worker) );
	}

	/**
	 * @set needDraw {bool}
	 */
	static void set_need_draw(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		self->set_need_draw( value->ToBooleanValue(worker) );
	}

	/**
	 * @set receive {bool}
	 */
	static void set_receive(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		self->set_receive( value->ToBooleanValue(worker) );
	}

	/**
	 * @get isFocus {bool}
	 */
	static void set_is_focus(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(View);
		self->set_is_focus( value->ToBooleanValue(worker) );
	}

	/**
	 * @set class {String}
	 */
	static void set_class(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( !value->IsString(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(View);
		self->set_class( value->ToStringValue(worker) );
	}
	
	/**
	 * @func firstButton
	 */
	static void first_button(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(View);
		Button* btn = self->first_button();
		if ( ! btn) JS_RETURN( worker->NewNull() );
		Wrap<Button>* wrap = Wrap<Button>::pack(btn, btn->view_type());
		JS_RETURN( wrap->that() );
	}
	
	/**
	 * @func hasChild(view)
	 */
	static void has_child(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() == 0 ) {
			JS_THROW_ERR("Bad argument.");
		}
		if ( !worker->has_instance(args[0], View::VIEW) ) {
			JS_RETURN_NULL();
		}
		JS_SELF(View);
		View* view = unpack<View>(args[0].To())->self();
		JS_RETURN( self->has_child(view) );
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
	 #define SET_FIELD(enum, class, name) JS_SET_PROPERTY(enum, View::enum);
		XX_EACH_VIEWS(SET_FIELD)
	 #undef SET_FIELD

		JS_DEFINE_CLASS(View, constructor, {
			// method
			JS_SET_CLASS_METHOD(prepend, prepend);
			JS_SET_CLASS_METHOD(append, append);
			JS_SET_CLASS_METHOD(append_text, append_text);
			JS_SET_CLASS_METHOD(before, before);
			JS_SET_CLASS_METHOD(after, after);
			JS_SET_CLASS_METHOD(remove, remove);
			JS_SET_CLASS_METHOD(removeAllChild, remove_all_child);
			JS_SET_CLASS_METHOD(focus, focus);
			JS_SET_CLASS_METHOD(blur, blur);
			JS_SET_CLASS_METHOD(layoutOffset, layout_offset);
			JS_SET_CLASS_METHOD(layoutOffsetFrom, layout_offset_from);
			JS_SET_CLASS_METHOD(getAction, get_action);
			JS_SET_CLASS_METHOD(setAction, set_action);
			JS_SET_CLASS_METHOD(screenRect, screen_rect);
			JS_SET_CLASS_METHOD(finalMatrix, final_matrix);
			JS_SET_CLASS_METHOD(finalOpacity, final_opacity);
			JS_SET_CLASS_METHOD(position, position);
			JS_SET_CLASS_METHOD(overlapTest, overlap_test);
			JS_SET_CLASS_METHOD(addClass, add_class);
			JS_SET_CLASS_METHOD(removeClass, remove_class);
			JS_SET_CLASS_METHOD(toggleClass, toggle_class);
			JS_SET_CLASS_METHOD(firstButton, first_button);
			JS_SET_CLASS_METHOD(hasChild, has_child);
			// property
			JS_SET_CLASS_ACCESSOR(innerText, inner_text);
			JS_SET_CLASS_ACCESSOR(parent, parent);
			JS_SET_CLASS_ACCESSOR(prev, prev);
			JS_SET_CLASS_ACCESSOR(next, next);
			JS_SET_CLASS_ACCESSOR(first, first);
			JS_SET_CLASS_ACCESSOR(last, last);
			JS_SET_CLASS_ACCESSOR(x, x, set_x);
			JS_SET_CLASS_ACCESSOR(y, y, set_y);
			JS_SET_CLASS_ACCESSOR(scaleX, scale_x, set_scale_x);
			JS_SET_CLASS_ACCESSOR(scaleY, scale_y, set_scale_y);
			JS_SET_CLASS_ACCESSOR(rotateZ, rotate_z, set_rotate_z);
			JS_SET_CLASS_ACCESSOR(skewX, skew_x, set_skew_x);
			JS_SET_CLASS_ACCESSOR(skewY, skew_y, set_skew_y);
			JS_SET_CLASS_ACCESSOR(opacity, opacity, set_opacity);
			JS_SET_CLASS_ACCESSOR(visible, visible, set_visible);
			JS_SET_CLASS_ACCESSOR(finalVisible, final_visible);
			JS_SET_CLASS_ACCESSOR(drawVisible, draw_visible);
			JS_SET_CLASS_ACCESSOR(translate, translate, set_translate);
			JS_SET_CLASS_ACCESSOR(scale, scale, set_scale);
			JS_SET_CLASS_ACCESSOR(skew, skew, set_skew);
			JS_SET_CLASS_ACCESSOR(originX, origin_x, set_origin_x);
			JS_SET_CLASS_ACCESSOR(originY, origin_y, set_origin_y);
			JS_SET_CLASS_ACCESSOR(origin, origin, set_origin);
			JS_SET_CLASS_ACCESSOR(matrix, matrix);
			JS_SET_CLASS_ACCESSOR(level, level);
			JS_SET_CLASS_ACCESSOR(needDraw, need_draw, set_need_draw);
			JS_SET_CLASS_ACCESSOR(receive, receive, set_receive);
			JS_SET_CLASS_ACCESSOR(isFocus, is_focus, set_is_focus);
			JS_SET_CLASS_ACCESSOR(viewType, view_type);
			JS_SET_CLASS_ACCESSOR(class, classs, set_class);
		}, nullptr);
		IMPL::js_class(worker)->set_class_alias(JS_TYPEID(View), View::VIEW);
	}
};

void binding_view(Local<JSObject> exports, Worker* worker) {
	WrapView::binding(exports, worker);
}

JS_END
