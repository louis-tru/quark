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
#include "ngui/display-port.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

static cString change("Change");
static cString beforerender("Beforerender");
static cString render("Render");
static cString orientation_("Orientation");

class WrapDisplayPort: public WrapObject {
 public: 
	typedef DisplayPort Type;
	
	/**
	 * @func bind_event
	 */
	bool add_event_listener(cString& name, cString& func, int id) {
		if ( name == change ) {
			self<Type>()->js_bind_common_native_event(change);
		}
		else if ( name == orientation_ ) {
			self<Type>()->js_bind_common_native_event(orientation);
		}
		else {
			return false;
		}
		return true;
	}
	
	/**
	 * @func unbind_event
	 */
	bool remove_event_listener(cString& name, int id) {
		if ( name == change ) {
			self<Type>()->js_unbind_native_event(change);
		}
		else if ( name == orientation_ ) {
			self<Type>()->js_unbind_native_event(orientation);
		}
		else {
			return false;
		}
		return true;
	}
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Forbidden access");
	}

	/**
	 * @func lock_size([width[,height]])
	 * @arg [width=0] {float}
	 * @arg [height=0] {float}
	 *
	 * width与height都设置为0时自动设置一个最舒适的默认显示尺寸
	 *
	 * 设置锁定视口为一个固定的逻辑尺寸,这个值改变时会触发change事件
	 *
	 * 如果width设置为零表示不锁定宽度,系统会自动根据height值设置一个同等比例的宽度
	 * 如果设置为非零表示锁定宽度,不管display_port_size怎么变化对于编程者来说,这个值永远保持不变
	 *
	 * 如果height设置为零表示不锁定,系统会自动根据width值设置一个同等比例的高度
	 * 如果设置为非零表示锁定高度,不管display_port_size怎么变化对于编程者来说,这个值永远保持不变
	 *
	 */
	static void lock_size(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 || !args[0]->IsNumber(worker) ) {
			JS_THROW_ERR(
				"* @func lockSize([width[,height]])"
				"* @arg [width=0] {float}"
				"* @arg [height=0] {float}"
			);
		}
		JS_SELF(DisplayPort);
		
		if (args.Length() > 1 && args[1]->IsNumber(worker)) {
			self->lock_size( args[0]->ToNumberValue(worker), args[1]->ToNumberValue(worker) );
		} else {
			self->lock_size( args[0]->ToNumberValue(worker) );
		}
	}

	/**
	 * @func next_frame(cb)
	 * @arg cb {Function}
	 */
	static void next_frame(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if (args.Length() < 1 || !args[0]->IsFunction(worker)) {
			JS_THROW_ERR(
				"* @func nextFrame(cb)\n"
				"* @arg cb {Function}\n"
			);
		}
		JS_SELF(DisplayPort);
		
		CopyablePersistentFunc func(worker, args[0].To<JSFunction>());

		self->next_frame(Cb([func, worker](Se& evt) {
			XX_ASSERT(!func.IsEmpty());
			JS_HANDLE_SCOPE();
			func.strong()->Call(worker);
			//const_cast<CopyablePersistentFunc*>(&func)->Reset();
		}));
	}
	
	/**
	 * @get width {float} 
	 */
	static void width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(DisplayPort);
		JS_RETURN( self->size().width() );
	}
	
	/**
	 * @get height {float} 
	 */
	static void height(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(DisplayPort);
		JS_RETURN( self->size().height() );
	}
	
	/**
	 * @get phy_width {float} 
	 */
	static void phy_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(DisplayPort);
		JS_RETURN( self->phy_size().width() );
	}
	
	/**
	 * @get phy_height {float} 
	 */
	static void phy_height(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(DisplayPort);
		JS_RETURN( self->phy_size().height() );
	}
	
	/**
	 * @get best_scale {float} 
	 */
	static void best_scale(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(DisplayPort);
		JS_RETURN( self->best_scale() );
	}
	
	/**
	 * @get scale {float} 
	 */
	static void scale(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(DisplayPort);
		JS_RETURN( self->scale() );
	}
	
	/**
	 * @get scale_value {Vec2}
	 */
	static void scale_value(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(DisplayPort);
		JS_RETURN( worker->value_program()->New(self->scale_value()) );
	}
	
	/**
	 * @get root_matrix {Mat4} 
	 */
	static void root_matrix(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(DisplayPort);
		JS_RETURN( worker->value_program()->New(self->root_matrix()) );
	}
	
	/**
	 * @get atom_pixel {float} 
	 */
	static void atom_pixel(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(DisplayPort);
		JS_RETURN( self->atom_pixel() );
	}
	
	/**
	 * @func keep_screen(keep)
	 */
	static void keep_screen(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 ) {
			JS_THROW_ERR(
										"* @func keepScreen(keep)\n"
										"* @arg keep {bool}\n"
										);
		}
		JS_SELF(DisplayPort);
		self->keep_screen( args[0]->ToBooleanValue(worker) );
	}
	
	/**
	 * @func status_bar_height()
	 */
	static void status_bar_height(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(DisplayPort);
		JS_RETURN( self->status_bar_height() );
	}
	
	/**
	 * @func set_visible_status_bar(visible)
	 */
	static void set_visible_status_bar(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 ) {
			JS_THROW_ERR(
										"* @func setVisibleStatusBar(visible)\n"
										"* @arg visible {bool}\n"
										);
		}
		JS_SELF(DisplayPort);
		self->set_visible_status_bar( args[0]->ToBooleanValue(worker) );
	}
	
	/**
	 * @func set_status_bar_style(style)
	 */
	static void set_status_bar_style(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 || !args[0]->IsUint32() ) {
			JS_THROW_ERR(
										"* @func setStatusBarStyle(style)\n"
										"* @arg style {StatusBarStyle}\n"
										);
		}
		JS_SELF(DisplayPort);
		self->set_status_bar_style( DisplayPort::StatusBarStyle(args[0]->ToUint32Value(worker)) );
	}
	
	/**
	 * @func request_fullscreen(fullscreen)
	 */
	static void request_fullscreen(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 ) {
			JS_THROW_ERR(
										"* @func requestFullscreen(fullscreen)\n"
										"* @arg fullscreen {bool}\n"
										);
		}
		JS_SELF(DisplayPort);
		self->request_fullscreen( args[0]->ToBooleanValue(worker) );
	}
	
	/**
	 * @func orientation()
	 */
	static void orientation(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(DisplayPort);
		JS_RETURN( self->orientation() );
	}
	
	/**
	 * @func set_orientation(orientation)
	 */
	static void set_orientation(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		if ( args.Length() < 1 || !args[0]->IsUint32(worker) ) {
			JS_THROW_ERR(
										"* @func setOrientation(orientation)\n"
										"* @arg orientation {Orientation}\n"
										);
		}
		JS_SELF(DisplayPort);
		self->set_orientation( DisplayPort::Orientation(args[0]->ToUint32Value(worker)) );
	}
	
	/**
	 * @func fsp()
	 */
	static void fsp(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(DisplayPort);
		JS_RETURN( self->fsp() );
	}
	
	/**
	 * @static default_atom_pixel {float} 
	 */
	static void default_atom_pixel(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_RETURN( DisplayPort::default_atom_pixel() );
	}
	
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_SET_ACCESSOR(defaultAtomPixel, default_atom_pixel);
		
		// emun Orientation
		JS_SET_PROPERTY(ORIENTATION_INVALID, DisplayPort::ORIENTATION_INVALID);
		JS_SET_PROPERTY(ORIENTATION_PORTRAIT, DisplayPort::ORIENTATION_PORTRAIT);
		JS_SET_PROPERTY(ORIENTATION_LANDSCAPE, DisplayPort::ORIENTATION_LANDSCAPE);
		JS_SET_PROPERTY(ORIENTATION_REVERSE_PORTRAIT, DisplayPort::ORIENTATION_REVERSE_PORTRAIT);
		JS_SET_PROPERTY(ORIENTATION_REVERSE_LANDSCAPE, DisplayPort::ORIENTATION_REVERSE_LANDSCAPE);
		JS_SET_PROPERTY(ORIENTATION_USER, DisplayPort::ORIENTATION_USER);
		JS_SET_PROPERTY(ORIENTATION_USER_PORTRAIT, DisplayPort::ORIENTATION_USER_PORTRAIT);
		JS_SET_PROPERTY(ORIENTATION_USER_LANDSCAPE, DisplayPort::ORIENTATION_USER_LANDSCAPE);
		JS_SET_PROPERTY(ORIENTATION_USER_LOCKED, DisplayPort::ORIENTATION_USER_LOCKED);
		
		JS_DEFINE_CLASS(DisplayPort, constructor, {
			JS_SET_CLASS_METHOD(lockSize, lock_size);
			JS_SET_CLASS_METHOD(nextFrame, next_frame);
			JS_SET_CLASS_METHOD(keepScreen, keep_screen);
			JS_SET_CLASS_METHOD(statusBarHeight, status_bar_height);
			JS_SET_CLASS_METHOD(setVisibleStatusBar, set_visible_status_bar);
			JS_SET_CLASS_METHOD(setStatusBarStyle, set_status_bar_style);
			JS_SET_CLASS_METHOD(requestFullscreen, request_fullscreen);
			JS_SET_CLASS_METHOD(orientation, orientation);
			JS_SET_CLASS_METHOD(setOrientation, set_orientation);
			JS_SET_CLASS_METHOD(fsp, fsp);
			JS_SET_CLASS_ACCESSOR(width, width);
			JS_SET_CLASS_ACCESSOR(height, height);
			JS_SET_CLASS_ACCESSOR(phyWidth, phy_width);
			JS_SET_CLASS_ACCESSOR(phyHeight, phy_height);
			JS_SET_CLASS_ACCESSOR(bestScale, best_scale);
			JS_SET_CLASS_ACCESSOR(scale, scale);
			JS_SET_CLASS_ACCESSOR(scaleValue, scale_value);
			JS_SET_CLASS_ACCESSOR(rootMatrix, root_matrix);
			JS_SET_CLASS_ACCESSOR(atomPixel, atom_pixel);
		}, NULL);
	}
};

void binding_display(Local<JSObject> exports, Worker* worker) {
	WrapDisplayPort::binding(exports, worker);
}

JS_END
