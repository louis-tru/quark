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

#include "./css.h"
#include "../view/view.h"
#include "../window.h"

namespace qk {

	typedef StyleSheets::Property Property;

	template<typename T>
	T* copy_value_ptr(T* value) {
		Qk_Unreachable("");
		return nullptr;
	}

	template<typename T>
	inline void transition_value_ptr(T *v1, T *v2, float y, CssProp prop, View *target) {
		Qk_Unreachable("");
	}

	template<>
	BoxFilter* copy_value_ptr(BoxFilter* value) {
		return value ? value->copy(nullptr, true): nullptr;
	}

	template<>
	void transition_value_ptr(BoxFilter *v1, BoxFilter *v2, float y, CssProp prop, View *target) {
		auto acc = target->accessor() + prop;
		if (acc->set) {
			auto v = (target->*(BoxFilter* (View::*)())acc->get)();
			auto v_new = v1 && v2 ? v1->transition(v, v2, y, true): y < 1.0 ? v1: v2;
			(target->*(void (View::*)(BoxFilter*,bool))acc->set)(v_new,true);
		}
	}

	template<typename T>
	inline T transition_value(T v1, T v2, float y) {
		//Qk_DLog("transition_value, %d, %d, %f", v1, v2, y);
		return v1 + (v2 - v1) * y;
	}

	template<>
	Curve transition_value(Curve v1, Curve v2, float y) {
		return y < 1.0 ? v1 : v2;
	}

	template<>
	BoxSize transition_value(BoxSize v1, BoxSize v2, float y) {
		if ( v1.kind == v2.kind ) {
			return { v1.value - (v1.value - v2.value) * y, v1.kind };
		} else {
			return y < 1.0 ? v1 : v2;
		}
	}

	template<>
	BoxOrigin transition_value(BoxOrigin v1, BoxOrigin v2, float y) {
		if ( v1.kind == v2.kind ) {
			return { v1.value - (v1.value - v2.value) * y, v1.kind };
		} else {
			return y < 1.0 ? v1 : v2;
		}
	}

	template<>
	Color transition_value(Color v1, Color v2, float y) {
		return Color(v1.r() - (v1.r() - v2.r()) * y,
								v1.g() - (v1.g() - v2.g()) * y,
								v1.b() - (v1.b() - v2.b()) * y, v1.a() - (v1.a() - v2.a()) * y);
	}

	template<>
	TextSize transition_value(TextSize v1, TextSize v2, float y) {
		if ( v1.kind == TextValueKind::Value && v2.kind == TextValueKind::Value ) {
			return { v1.value - (v1.value - v2.value) * y, TextValueKind::Value };
		} else {
			return  y < 1.0 ? v1 : v2;
		}
	}

	template<>
	TextColor transition_value(TextColor v1, TextColor v2, float y) {
		if ( v1.kind == TextValueKind::Value && v2.kind == TextValueKind::Value ) {
			return { transition_value(v1.value, v2.value, y), TextValueKind::Value };
		} else {
			return  y < 1.0 ? v1 : v2;
		}
	}

	template<>
	Border transition_value(Border v1, Border v2, float y) {
		return {
			transition_value(v1.width, v2.width, y),
			transition_value(v1.color, v2.color, y),
		};
	}

	template<>
	TextStroke transition_value(TextStroke v1, TextStroke v2, float y) {
		if ( v1.kind == TextValueKind::Value && v2.kind == TextValueKind::Value ) {
			return { transition_value(v1.value, v2.value, y), TextValueKind::Value };
		} else {
			return  y < 1.0 ? v1 : v2;
		}
	}

	template<>
	Shadow transition_value(Shadow v1, Shadow v2, float y) {
		return {
			v1.x - (v1.x - v2.x) * y,
			v1.y - (v1.y - v2.y) * y,
			v1.size - (v1.size - v2.size) * y,
			transition_value(v1.color, v2.color, y),
		};
	}

	template<>
	TextShadow transition_value(TextShadow v1, TextShadow v2, float y) {
		if ( v1.kind == TextValueKind::Value && v2.kind == TextValueKind::Value ) {
			return { transition_value(v1.value, v2.value, y), TextValueKind::Value };
		} else {
			return y < 1.0 ? v1 : v2;
		}
	}

	#define _Define_Enum_transition(Type) template<>\
		Type transition_value(Type f1, Type f2, float t) { return t < 1.0 ? f1: f2; }
	_Define_Enum_transition(bool)
	//_Define_Enum_transition(int)
	_Define_Enum_transition(CascadeColor)
	_Define_Enum_transition(Align)
	_Define_Enum_transition(Direction)
	_Define_Enum_transition(ItemsAlign)
	_Define_Enum_transition(CrossAlign)
	_Define_Enum_transition(Wrap)
	_Define_Enum_transition(WrapAlign)
	_Define_Enum_transition(String)
	_Define_Enum_transition(TextAlign)
	_Define_Enum_transition(TextWeight)
	_Define_Enum_transition(TextSlant)
	_Define_Enum_transition(TextDecoration)
	_Define_Enum_transition(TextOverflow)
	_Define_Enum_transition(TextWhiteSpace)
	_Define_Enum_transition(TextWordBreak)
	_Define_Enum_transition(TextFamily)
	_Define_Enum_transition(KeyboardType)
	_Define_Enum_transition(KeyboardReturnType)
	_Define_Enum_transition(CursorStyle)
	#undef _Define_Enum_transition

	// ----------------------------------------------------------------------------------------------

	template<typename T>
	struct PropImpl: Property {
		inline PropImpl(CssProp prop, T value): _prop(prop), _value(value) {}
		void apply(View *view, bool isRt) override {
			auto set = (void (View::*)(T,bool))(view->accessor() + _prop)->set;
			if (set)
				(view->*set)(_value,isRt);
		}
		void fetch(View *view) override {
			auto get = (T (View::*)())(view->accessor() + _prop)->get;
			if (get)
				_value = (view->*get)();
		}
		void transition(View *view, Property *to, float y) override {
			Qk_ASSERT(static_cast<PropImpl*>(to)->_prop == _prop);
			auto set = (void (View::*)(T,bool))(view->accessor() + _prop)->set;
			if (set)
				(view->*set)(transition_value(_value, static_cast<PropImpl*>(to)->_value, y),true);
		}
		Property* copy() override {
			return new PropImpl<T>(_prop, _value);
		}
		CssProp _prop;
		T        _value;
	};

	// @template Object or BoxFilter
	template<typename T>
	struct PropImpl<T*>: Property {
		PropImpl(CssProp prop, T* value): _prop(prop), _value(value) {
			static_assert(object_traits<T>::is::obj, "Property value must be a object type");
			Retain(value);
		}
		~PropImpl() {
			Release(_value);
		}
		void apply(View *view, bool isRt) override {
			auto set = (void (View::*)(T*,bool))(view->accessor() + _prop)->set;
			if (set)
				(view->*set)(_value, isRt);
		}
		void fetch(View *view) override {
			auto get = (T* (View::*)())(view->accessor() + _prop)->get;
			if (get)
				_value = BoxFilter::assign(_value, (view->*get)(), nullptr, true);
		}
		void transition(View *target, Property *to, float t) override {
			Qk_ASSERT(static_cast<PropImpl*>(to)->_prop == _prop);
			transition_value_ptr(
				static_cast<BoxFilter*>(_value),
				static_cast<BoxFilter*>(static_cast<PropImpl*>(to)->_value),
				t, _prop, target
			);
		}
		Property* copy() override {
			return new PropImpl(_prop, copy_value_ptr(static_cast<BoxFilter*>(_value)));
		}
		CssProp _prop;
		T       *_value;
	};

	// ---- SetProp ----

	template<typename T>
	struct SetProp: StyleSheets {
		void set(CssProp key, T value) {
			Property *prop;
			if (_props.get(key, prop)) {
				static_cast<PropImpl<T>*>(prop)->_value = value;
			} else {
				onMake(key, _props.set(key, new PropImpl<T>(key, value)));
			}
		}
		template<CssProp key>
		void asyncSet(T value) {
			auto win = getWindowForAsyncSet();
			if (win) {
				win->preRender().async_call([](auto self, auto arg) {
					self->set(key, arg.arg);
				}, this, value);
			} else {
				set(key, value);
			}
		}
		template<CssProp key>
		void asyncSetLarge(T &value) {
			auto win = getWindowForAsyncSet();
			if (win) {
				win->preRender().async_call([](auto self, auto arg) {
					Sp<T> h(arg.arg);
					self->set(key, *arg.arg);
				}, this, new T(value));
			} else {
				set(key, value);
			}
		}
	};

	template<>
	template<CssProp key>
	void SetProp<String>::asyncSet(String value) {
		asyncSetLarge<key>(value);
	}

	template<>
	template<CssProp key>
	void SetProp<Curve>::asyncSet(Curve value) {
		asyncSetLarge<key>(value);
	}

	template<>
	template<>
	void SetProp<ArrayFloat>::asyncSet<kMARGIN_CssProp>(ArrayFloat val) {
		switch (val.length()) {
			case 1:
				set_margin_left(val[0]);
				set_margin_top(val[0]);
				set_margin_right(val[0]);
				set_margin_bottom(val[0]);
				break;
			case 2:
				set_margin_top(val[0]);
				set_margin_bottom(val[0]);
				set_margin_left(val[1]);
				set_margin_right(val[1]);
				break;
			case 3:
				set_margin_top(val[0]);
				set_margin_left(val[1]);
				set_margin_right(val[1]);
				set_margin_bottom(val[2]);
				break;
			case 4: // 4
				set_margin_top(val[0]);
				set_margin_right(val[1]);
				set_margin_bottom(val[2]);
				set_margin_left(val[3]);
				break;
			default: break;
		}
	}

	template<>
	template<>
	void SetProp<ArrayFloat>::asyncSet<kPADDING_CssProp>(ArrayFloat val) {
		switch (val.length()) {
			case 1:
				set_padding_left(val[0]);
				set_padding_top(val[0]);
				set_padding_right(val[0]);
				set_padding_bottom(val[0]);
				break;
			case 2:
				set_padding_top(val[0]);
				set_padding_bottom(val[0]);
				set_padding_left(val[1]);
				set_padding_right(val[1]);
				break;
			case 3:
				set_padding_top(val[0]);
				set_padding_left(val[1]);
				set_padding_right(val[1]);
				set_padding_bottom(val[2]);
				break;
			case 4: // 4
				set_padding_top(val[0]);
				set_padding_right(val[1]);
				set_padding_bottom(val[2]);
				set_padding_left(val[3]);
				break;
			default: break;
		}
	}

	template<>
	template<>
	void SetProp<ArrayFloat>::asyncSet<kBORDER_RADIUS_CssProp>(ArrayFloat val) {
		switch (val.length()) {
			case 1:
				set_border_radius_left_top(val[0]);
				set_border_radius_right_top(val[0]);
				set_border_radius_right_bottom(val[0]);
				set_border_radius_left_bottom(val[0]);
				break;
			case 2:
				set_border_radius_left_top(val[0]);
				set_border_radius_right_top(val[0]);
				set_border_radius_right_bottom(val[1]);
				set_border_radius_left_bottom(val[1]);
				break;
			case 3:
				set_border_radius_left_top(val[0]);
				set_border_radius_right_top(val[1]);
				set_border_radius_right_bottom(val[2]);
				set_border_radius_left_bottom(val[2]);
				break;
			case 4: // 4
				set_border_radius_left_top(val[0]);
				set_border_radius_right_top(val[1]);
				set_border_radius_right_bottom(val[2]);
				set_border_radius_left_bottom(val[3]);
				break;
			default: break;
		}
	}

	template<>
	template<>
	void SetProp<ArrayBorder>::asyncSet<kBORDER_CssProp>(ArrayBorder val) {
		switch (val.length()) {
			case 1:
				set_border_top(val[0]);
				set_border_right(val[0]);
				set_border_bottom(val[0]);
				set_border_left(val[0]);
				break;
			case 2:
				set_border_top(val[0]);
				set_border_bottom(val[0]);
				set_border_left(val[1]);
				set_border_right(val[1]);
				break;
			case 3:
				set_border_top(val[0]);
				set_border_left(val[1]);
				set_border_right(val[1]);
				set_border_bottom(val[2]);
				break;
			case 4: // 4
				set_border_top(val[0]);
				set_border_right(val[1]);
				set_border_bottom(val[2]);
				set_border_left(val[3]);
				break;
			default: break;
		}
	}

	template<>
	template<>
	void SetProp<Border>::asyncSet<kBORDER_TOP_CssProp>(Border val) {
		set_border_width_top(val.width);
		set_border_color_top(val.color);
	}

	template<>
	template<>
	void SetProp<Border>::asyncSet<kBORDER_RIGHT_CssProp>(Border val) {
		set_border_width_right(val.width);
		set_border_color_right(val.color);
	}

	template<>
	template<>
	void SetProp<Border>::asyncSet<kBORDER_BOTTOM_CssProp>(Border val) {
		set_border_width_bottom(val.width);
		set_border_color_bottom(val.color);
	}

	template<>
	template<>
	void SetProp<Border>::asyncSet<kBORDER_LEFT_CssProp>(Border val) {
		set_border_width_left(val.width);
		set_border_color_left(val.color);
	}

	template<>
	template<>
	void SetProp<ArrayFloat>::asyncSet<kBORDER_WIDTH_CssProp>(ArrayFloat val) {
		switch (val.length()) {
			case 1:
				set_border_width_top(val[0]);
				set_border_width_right(val[0]);
				set_border_width_bottom(val[0]);
				set_border_width_left(val[0]);
				break;
			case 2:
				set_border_width_top(val[0]);
				set_border_width_bottom(val[0]);
				set_border_width_left(val[1]);
				set_border_width_right(val[1]);
				break;
			case 3:
				set_border_width_top(val[0]);
				set_border_width_left(val[1]);
				set_border_width_right(val[1]);
				set_border_width_bottom(val[2]);
				break;
			case 4: // 4
				set_border_width_top(val[0]);
				set_border_width_right(val[1]);
				set_border_width_bottom(val[2]);
				set_border_width_left(val[3]);
				break;
			default: break;
		}
	}

	template<>
	template<>
	void SetProp<ArrayColor>::asyncSet<kBORDER_COLOR_CssProp>(ArrayColor val) {
		switch (val.length()) {
			case 1:
				set_border_color_top(val[0]);
				set_border_color_right(val[0]);
				set_border_color_bottom(val[0]);
				set_border_color_left(val[0]);
				break;
			case 2:
				set_border_color_top(val[0]);
				set_border_color_bottom(val[0]);
				set_border_color_left(val[1]);
				set_border_color_right(val[1]);
				break;
			case 3:
				set_border_color_top(val[0]);
				set_border_color_left(val[1]);
				set_border_color_right(val[1]);
				set_border_color_bottom(val[2]);
				break;
			case 4: // 4
				set_border_color_top(val[0]);
				set_border_color_right(val[1]);
				set_border_color_bottom(val[2]);
				set_border_color_left(val[3]);
				break;
			default: break;
		}
	}

	template<>
	template<>
	void SetProp<ArrayOrigin>::asyncSet<kORIGIN_CssProp>(ArrayOrigin val) {
		switch (val.length()) {
			case 1:
				set_origin_x(val[0]);
				set_origin_y(val[0]);
				break;
			case 2:
				set_origin_x(val[0]);
				set_origin_y(val[1]);
				break;
			default: break;
		}
	}


	template<>
	template<CssProp key>
	void SetProp<TextStroke>::asyncSet(TextStroke value) {
		asyncSetLarge<key>(value);
	}

	template<>
	template<CssProp key>
	void SetProp<TextShadow>::asyncSet(TextShadow value) {
		asyncSetLarge<key>(value);
	}

	template<>
	template<CssProp key>
	void SetProp<TextFamily>::asyncSet(TextFamily value) {
		asyncSetLarge<key>(value);
	}

	template<>
	struct SetProp<BoxFilter*>: StyleSheets {
		void set(CssProp key, BoxFilter* value, bool isRt) {
			Property *prop;
			if (_props.get(key, prop)) {
				auto p = static_cast<PropImpl<BoxFilter*>*>(prop);
				p->_value = BoxFilter::assign(p->_value, value, nullptr, isRt);
				if (value)
					p->_value->mark_public();
			} else if (value) {
				auto filter = BoxFilter::assign(nullptr, value, nullptr, isRt); // copy filter
				filter->mark_public();
				onMake(key, _props.set(key, new PropImpl<BoxFilter*>(key, filter)));
				filter->release(); // @BoxFilter::assign
			}
		}
		template<CssProp key>
		void asyncSet(BoxFilter* value) {
			auto win = getWindowForAsyncSet();
			if (win) {
				Retain(value); // retain the object before calling
				win->preRender().async_call([](auto self, auto arg) {
					self->set(key, arg.arg, true);
					Release(arg.arg);
				}, this, value);
			} else {
				set(key, value, false);
			}
		}
	};

	template<typename T>
	struct SetProp<T*>: StyleSheets {
		template<CssProp key>
		inline void asyncSet(T* val) {
			static_cast<SetProp<BoxFilter*>*>(static_cast<StyleSheets*>(this))->asyncSet<key>(val);
		}
	};

	void StyleSheets::set_frame_rt(uint32_t frame) {
		static_cast<SetProp<uint32_t>*>(this)->set(kFRAME_CssProp, frame);
	}

	#define _Fun(Enum, Type, Name, _) \
	void StyleSheets::set_##Name(Type val) {\
		static_cast<SetProp<Type>*>(this)->asyncSet<k##Enum##_CssProp>(val);\
	}
	Qk_Css_Props(_Fun)
	#undef _Fun

	cCurve& StyleSheets::curve() const {
		Property* prop = nullptr;
		if (_props.get(kCURVE_CssProp, prop)) {
			return static_cast<PropImpl<Curve>*>(prop)->_value;
		} else {
			return EASE;
		}
	}
}
