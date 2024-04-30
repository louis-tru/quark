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

#include "../js_.h"
#include "../_view.h"
#include "../../display-port.h"

/**
 * @ns qk::js
 */

Js_BEGIN

static cString change("Change");
static cString beforerender("Beforerender");
static cString render("Render");
static cString orientation_("Orientation");

class WrapDisplay: public WrapObject {
public:
	typedef Display Type;

	bool addEventListener(cString& name, cString& func, int id) {
		if ( name == change ) {
			self<Type>()->Js_Native_On(Change, func, id);
		}
		else if ( name == orientation_ ) {
			self<Type>()->Js_Native_On(Orientation, func, id);
		}
		else {
			return false;
		}
		return true;
	}

	bool removeEventListener(cString& name, int id) {
		if ( name == change ) {
			self<Type>()->Qk_Off(Change, id);
		}
		else if ( name == orientation_ ) {
			self<Type>()->Qk_Off(Orientation, id);
		}
		else {
			return false;
		}
		return true;
	}
	
	static void constructor(FunctionArgs args) {
		Js_ATTACH(args);
		Js_Worker(args);
		Js_Throw("Forbidden access");
	}

	/**
	 * @method lock_size([width[,height]])
	 * @param [width=0] {float}
	 * @param [height=0] {float}
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
	static void lock_size(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 || !args[0]->IsNumber(worker) ) {
			Js_Throw(
				"* @method lockSize([width[,height]])"
				"* @param [width=0] {float}"
				"* @param [height=0] {float}"
			);
		}
		Js_Self(Display);
		
		if (args.length() > 1 && args[1]->IsNumber(worker)) {
			self->lock_size( args[0]->ToNumberValue(worker), args[1]->ToNumberValue(worker) );
		} else {
			self->lock_size( args[0]->ToNumberValue(worker) );
		}
	}

	/**
	 * @method next_frame(cb)
	 * @param cb {Function}
	 */
	static void next_frame(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if (args.length() < 1 || !args[0]->isFunction(worker)) {
			Js_Throw(
				"* @method nextFrame(cb)\n"
				"* @param cb {Function}\n"
			);
		}
		Js_Self(Display);
		
		CopyablePersistentFunc func(worker, args[0].To<JSFunction>());

		self->next_frame(Cb([func, worker](Cb::Data& evt) {
			Qk_ASSERT(!func.IsEmpty());
			Js_Callback_Scope();
			func.local()->Call(worker);
			//const_cast<CopyablePersistentFunc*>(&func)->Reset();
		}));
	}
	
	/**
	 * @get width {float} 
	 */
	static void width(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Display);
		Js_Return( self->size().width() );
	}
	
	/**
	 * @get height {float} 
	 */
	static void height(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Display);
		Js_Return( self->size().height() );
	}
	
	/**
	 * @get phy_width {float} 
	 */
	static void phy_width(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Display);
		Js_Return( self->phy_size().width() );
	}
	
	/**
	 * @get phy_height {float} 
	 */
	static void phy_height(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Display);
		Js_Return( self->phy_size().height() );
	}
	
	/**
	 * @get best_scale {float} 
	 */
	static void best_scale(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Display);
		Js_Return( self->best_scale() );
	}
	
	/**
	 * @get scale {float} 
	 */
	static void scale(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Display);
		Js_Return( self->scale() );
	}
	
	/**
	 * @get scale_value {Vec2}
	 */
	static void scale_value(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Display);
		Js_Return( worker->values()->New(self->scale_value()) );
	}
	
	/**
	 * @get root_matrix {Mat4} 
	 */
	static void root_matrix(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Display);
		Js_Return( worker->values()->New(self->root_matrix()) );
	}
	
	/**
	 * @get atom_pixel {float} 
	 */
	static void atom_pixel(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Display);
		Js_Return( self->atom_pixel() );
	}
	
	/**
	 * @method keep_screen(keep)
	 */
	static void keep_screen(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 ) {
			Js_Throw(
										"* @method keepScreen(keep)\n"
										"* @param keep {bool}\n"
										);
		}
		Js_Self(Display);
		self->keep_screen( args[0]->ToBooleanValue(worker) );
	}
	
	/**
	 * @method status_bar_height()
	 */
	static void status_bar_height(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Display);
		Js_Return( self->status_bar_height() );
	}
	
	/**
	 * @method set_visible_status_bar(visible)
	 */
	static void set_visible_status_bar(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 ) {
			Js_Throw(
										"* @method setVisibleStatusBar(visible)\n"
										"* @param visible {bool}\n"
										);
		}
		Js_Self(Display);
		self->set_visible_status_bar( args[0]->ToBooleanValue(worker) );
	}
	
	/**
	 * @method set_status_bar_style(style)
	 */
	static void set_status_bar_style(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 || !args[0]->IsUint32() ) {
			Js_Throw(
										"* @method setStatusBarStyle(style)\n"
										"* @param style {StatusBarStyle}\n"
										);
		}
		Js_Self(Display);
		self->set_status_bar_style( Display::StatusBarStyle(args[0]->ToUint32Value(worker)) );
	}
	
	/**
	 * @method request_fullscreen(fullscreen)
	 */
	static void request_fullscreen(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 ) {
			Js_Throw(
										"* @method requestFullscreen(fullscreen)\n"
										"* @param fullscreen {bool}\n"
										);
		}
		Js_Self(Display);
		self->request_fullscreen( args[0]->ToBooleanValue(worker) );
	}
	
	/**
	 * @method orientation()
	 */
	static void orientation(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Display);
		Js_Return( self->orientation() );
	}
	
	/**
	 * @method set_orientation(orientation)
	 */
	static void set_orientation(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 || !args[0]->IsUint32(worker) ) {
			Js_Throw(
										"* @method setOrientation(orientation)\n"
										"* @param orientation {Orientation}\n"
										);
		}
		Js_Self(Display);
		self->set_orientation( Display::Orientation(args[0]->ToUint32Value(worker)) );
	}
	
	/**
	 * @method fsp()
	 */
	static void fsp(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Display);
		Js_Return( self->fsp() );
	}
	
	/**
	 * @method default_atom_pixel() {float} 
	 */
	static void default_atom_pixel(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_Return( Display::default_atom_pixel() );
	}
	
	/**
	 * @method default_status_bar_height() {float}
	 */
	static void default_status_bar_height(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_Return( Display::default_status_bar_height() );
	}
	
	static void binding(JSObject* exports, Worker* worker) {
		Js_Set_Method(defaultAtomPixel, default_atom_pixel);
		Js_Set_Method(defaultStatusBarHeight, default_status_bar_height);

		Js_Define_Class(Display, constructor, {
			Js_Set_Class_Method(lockSize, lock_size);
			Js_Set_Class_Method(nextFrame, next_frame);
			Js_Set_Class_Method(keepScreen, keep_screen);
			Js_Set_Class_Method(statusBarHeight, status_bar_height);
			Js_Set_Class_Method(setVisibleStatusBar, set_visible_status_bar);
			Js_Set_Class_Method(setStatusBarStyle, set_status_bar_style);
			Js_Set_Class_Method(requestFullscreen, request_fullscreen);
			Js_Set_Class_Method(orientation, orientation);
			Js_Set_Class_Method(setOrientation, set_orientation);
			Js_Set_Class_Method(fsp, fsp);
			Js_Set_Class_Accessor(width, width);
			Js_Set_Class_Accessor(height, height);
			Js_Set_Class_Accessor(phyWidth, phy_width);
			Js_Set_Class_Accessor(phyHeight, phy_height);
			Js_Set_Class_Accessor(bestScale, best_scale);
			Js_Set_Class_Accessor(scale, scale);
			Js_Set_Class_Accessor(scaleValue, scale_value);
			Js_Set_Class_Accessor(rootMatrix, root_matrix);
			Js_Set_Class_Accessor(atomPixel, atom_pixel);
		}, NULL);
	}
};

Js_REG_MODULE(_display_port, WrapDisplay);
Js_END
