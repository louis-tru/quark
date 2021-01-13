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

typedef KeyframeAction::Frame Frame;
typedef KeyframeAction::Property Property;

/**
 * @class Property2
 */
template<class T> class Property2: public Property {
 public:
	typedef T    (View::*GetPropertyFunc)() const;
	typedef void (View::*SetPropertyFunc)(T value);
	
	inline Property2(uint frame_count)
	: _frames(frame_count)
	, _get_property_func(nullptr)
	, _set_property_func(nullptr) {}
	
	virtual ~Property2() {}
	
	inline void set_property(List<View*>& views) {
		for ( auto& i : views ) {
			if ( i.value() ) {
				(i.value()->*_set_property_func)(_transition);
			}
		}
	}
	
	inline T get_property(View* view) {
		return (view->*_get_property_func)();
	}
	
	virtual void transition(uint f1, Action* root) {
		if ( _set_property_func ) {
			_transition = _frames[f1];
			set_property(_inl_action(root)->views());
		}
	}
	
	virtual void transition(uint f1, uint f2, float x, float y, Action* root) {
		if ( _set_property_func ) {
			T v1 = _frames[f1], v2 = _frames[f2];
			_transition = v1 - (v1 - v2) * y;
			set_property(_inl_action(root)->views());
		}
	}
	
	virtual void add_frame() {
		_frames.push( T() );
	}
	
	virtual void fetch(uint frame, View* view) {
		if ( _get_property_func ) {
			_frames[frame] = get_property(view);
		}
	}
	
	virtual void default_value(uint frame) {
		_frames[frame] = T();
	}
	
	inline T operator[](uint index) {
		return _frames[index];
	}
	
	inline T frame(uint index) {
		return _frames[index];
	}
	
	inline void frame(uint index, T value) {
		_frames[index] = value;
	} 
	
 protected:
	
	Array<T>        _frames;
	T               _transition;
	GetPropertyFunc _get_property_func;
	SetPropertyFunc _set_property_func;
};

// set_property
template<> inline void Property2<TextColor>::set_property(List<View*>& views) {
	typedef void (TextFont::*Func)(TextColor);
	for ( auto& i : views ) {
		(i.value()->as_text_font()->*reinterpret_cast<Func>(_set_property_func))(_transition);
	}
}
template<> inline void Property2<TextSize>::set_property(List<View*>& views) {
	typedef void (TextFont::*Func)(TextSize);
	for ( auto& i : views ) {
		(i.value()->as_text_font()->*reinterpret_cast<Func>(_set_property_func))(_transition);
	}
}
template<> inline void Property2<TextStyle>::set_property(List<View*>& views) {
	typedef void (TextFont::*Func)(TextStyle);
	for ( auto& i : views ) {
		(i.value()->as_text_font()->*reinterpret_cast<Func>(_set_property_func))(_transition);
	}
}
template<> inline void Property2<TextFamily>::set_property(List<View*>& views) {
	typedef void (TextFont::*Func)(TextFamily);
	for ( auto& i : views ) {
		(i.value()->as_text_font()->*reinterpret_cast<Func>(_set_property_func))(_transition);
	}
}
template<> inline void Property2<TextLineHeight>::set_property(List<View*>& views) {
	typedef void (TextFont::*Func)(TextLineHeight);
	for ( auto& i : views ) {
		(i.value()->as_text_font()->*reinterpret_cast<Func>(_set_property_func))(_transition);
	}
}
template<> inline void Property2<TextShadow>::set_property(List<View*>& views) {
	typedef void (TextFont::*Func)(TextShadow);
	for ( auto& i : views ) {
		(i.value()->as_text_font()->*reinterpret_cast<Func>(_set_property_func))(_transition);
	}
}
template<> inline void Property2<TextDecoration>::set_property(List<View*>& views) {
	typedef void (TextFont::*Func)(TextDecoration);
	for ( auto& i : views ) {
		(i.value()->as_text_font()->*reinterpret_cast<Func>(_set_property_func))(_transition);
	}
}
template<> inline void Property2<TextOverflow>::set_property(List<View*>& views) {
	typedef void (TextLayout::*Func)(TextOverflow);
	for ( auto& i : views ) {
		(i.value()->as_text_layout()->*reinterpret_cast<Func>(_set_property_func))(_transition);
	}
}
template<> inline void Property2<TextWhiteSpace>::set_property(List<View*>& views) {
	typedef void (TextLayout::*Func)(TextWhiteSpace);
	for ( auto& i : views ) {
		(i.value()->as_text_layout()->*reinterpret_cast<Func>(_set_property_func))(_transition);
	}
}

// get_property
template<> inline TextColor Property2<TextColor>::get_property(View* view) {
	typedef TextColor (TextFont::*Func)() const;
	return (view->as_text_font()->*reinterpret_cast<Func>(_get_property_func))();
}
template<> inline TextSize Property2<TextSize>::get_property(View* view) {
	typedef TextSize (TextFont::*Func)() const;
	return (view->as_text_font()->*reinterpret_cast<Func>(_get_property_func))();
}
template<> inline TextStyle Property2<TextStyle>::get_property(View* view) {
	typedef TextStyle (TextFont::*Func)() const;
	return (view->as_text_font()->*reinterpret_cast<Func>(_get_property_func))();
}
template<> inline TextFamily Property2<TextFamily>::get_property(View* view) {
	typedef TextFamily (TextFont::*Func)() const;
	return (view->as_text_font()->*reinterpret_cast<Func>(_get_property_func))();
}
template<> inline TextLineHeight Property2<TextLineHeight>::get_property(View* view) {
	typedef TextLineHeight (TextFont::*Func)() const;
	return (view->as_text_font()->*reinterpret_cast<Func>(_get_property_func))();
}
template<> inline TextShadow Property2<TextShadow>::get_property(View* view) {
	typedef TextShadow (TextFont::*Func)() const;
	return (view->as_text_font()->*reinterpret_cast<Func>(_get_property_func))();
}
template<> inline TextDecoration Property2<TextDecoration>::get_property(View* view) {
	typedef TextDecoration (TextFont::*Func)() const;
	return (view->as_text_font()->*reinterpret_cast<Func>(_get_property_func))();
}
template<> inline TextOverflow Property2<TextOverflow>::get_property(View* view) {
	typedef TextOverflow (TextLayout::*Func)() const;
	return (view->as_text_layout()->*reinterpret_cast<Func>(_get_property_func))();
}
template<> inline TextWhiteSpace Property2<TextWhiteSpace>::get_property(View* view) {
	typedef TextWhiteSpace (TextLayout::*Func)() const;
	return (view->as_text_layout()->*reinterpret_cast<Func>(_get_property_func))();
}

// transition

template<>
void Property2<bool>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		_transition = t < 1.0 ? _frames[f1] : _frames[f2];
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<Vec2>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		Vec2 v1 = _frames[f1], v2 = _frames[f2];
		_transition = Vec2(v1.x() - (v1.x() - v2.x()) * t, v1.y() - (v1.y() - v2.y()) * t);
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<Color>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		Color v1 = _frames[f1], v2 = _frames[f2];
		_transition = Color(v1.r() - (v1.r() - v2.r()) * t,
												 v1.g() - (v1.g() - v2.g()) * t,
												 v1.b() - (v1.b() - v2.b()) * t, v1.a() - (v1.a() - v2.a()) * t);
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<TextAlign>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		_transition = t < 1.0 ? _frames[f1] : _frames[f2];
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<Align>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		_transition = t < 1.0 ? _frames[f1] : _frames[f2];
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<ContentAlign>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		_transition = t < 1.0 ? _frames[f1] : _frames[f2];
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<Repeat>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		_transition = t < 1.0 ? _frames[f1] : _frames[f2];
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<Border>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		Border v1 = _frames[f1], v2 = _frames[f2];
		float width = v1.width - (v1.width - v2.width) * t;
		Color color(v1.color.r() - (v1.color.r() - v2.color.r()) * t,
								v1.color.g() - (v1.color.g() - v2.color.g()) * t,
								v1.color.b() - (v1.color.b() - v2.color.b()) * t,
								v1.color.a() - (v1.color.a() - v2.color.a()) * t);
		_transition = Border(width, color);
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<Shadow>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		Shadow v1 = _frames[f1], v2 = _frames[f2];
		float offset_x = v1.offset_x - (v1.offset_x - v2.offset_x) * t;
		float offset_y = v1.offset_y - (v1.offset_y - v2.offset_y) * t;
		float size = v1.size - (v1.size - v2.size) * t;
		Color color(v1.color.r() - (v1.color.r() - v2.color.r()) * t,
								v1.color.g() - (v1.color.g() - v2.color.g()) * t,
								v1.color.b() - (v1.color.b() - v2.color.b()) * t,
								v1.color.a() - (v1.color.a() - v2.color.a()) * t);
		_transition = { offset_x, offset_y, size, color };
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<Value>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		Value v1 = _frames[f1], v2 = _frames[f2];
		if ( v1.type == v2.type ) {
			_transition = Value(v1.type, v1.value - (v1.value - v2.value) * t);
		} else {
			_transition = x < 1.0 ? v1 : v2;
		}
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<TextColor>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		TextColor v1 = _frames[f1], v2 = _frames[f2];
		if ( v1.type == TextValueType::VALUE && v2.type == TextValueType::VALUE ) {
			Color color(v1.value.r() - (v1.value.r() - v2.value.r()) * t,
									v1.value.g() - (v1.value.g() - v2.value.g()) * t,
									v1.value.b() - (v1.value.b() - v2.value.b()) * t,
									v1.value.a() - (v1.value.a() - v2.value.a()) * t);
			_transition = { TextValueType::VALUE, color };
		} else {
			_transition = x < 1.0 ? v1 : v2;
		}
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<TextSize>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		TextSize v1 = _frames[f1], v2 = _frames[f2];
		if ( v1.type == TextValueType::VALUE && v2.type == TextValueType::VALUE ) {
			_transition = { TextValueType::VALUE, v1.value - (v1.value - v2.value) * t };
		} else {
			_transition = x < 1.0 ? v1 : v2;
		}
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<TextStyle>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		_transition = x < 1.0 ? _frames[f1] : _frames[f2];
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<TextFamily>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		_transition = x < 1.0 ? _frames[f1] : _frames[f2];
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<TextLineHeight>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		TextLineHeight v1 = _frames[f1], v2 = _frames[f2];
		if (v1.type == TextValueType::VALUE && v2.type == TextValueType::VALUE) {
			_transition = {
				TextValueType::VALUE,
				v1.value.height - (v1.value.height - v2.value.height) * t
			};
		} else {
			_transition = x < 1.0 ? v1 : v2;
		}
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<TextShadow>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		TextShadow v1 = _frames[f1], v2 = _frames[f2];
		if ( v1.type == TextValueType::VALUE && v2.type == TextValueType::VALUE ) {
			float offset_x = v1.value.offset_x - (v1.value.offset_x - v2.value.offset_x) * t;
			float offset_y = v1.value.offset_y - (v1.value.offset_y - v2.value.offset_y) * t;
			float size = v1.value.size - (v1.value.size - v2.value.size) * t;
			Color color(v1.value.color.r() - (v1.value.color.r() - v2.value.color.r()) * t,
									v1.value.color.g() - (v1.value.color.g() - v2.value.color.g()) * t,
									v1.value.color.b() - (v1.value.color.b() - v2.value.color.b()) * t,
									v1.value.color.a() - (v1.value.color.a() - v2.value.color.a()) * t);
			_transition = { v1.type, offset_x, offset_y, size, color };
		} else {
			_transition = x < 1.0 ? v1 : v2;
		}
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<TextDecoration>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		_transition = x < 1.0 ? _frames[f1] : _frames[f2];
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<TextOverflow>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		_transition = x < 1.0 ? _frames[f1] : _frames[f2];
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<TextWhiteSpace>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		_transition = x < 1.0 ? _frames[f1] : _frames[f2];
		set_property(_inl_action(root)->views());
	}
}
template<>
void Property2<String>::transition(uint f1, uint f2, float x, float t, Action* root) {
	if ( _set_property_func ) {
		_transition = x < 1.0 ? _frames[f1] : _frames[f2];
		set_property(_inl_action(root)->views());
	}
}

//-------------------------------------------------------------------------------

template<>
Property2<BackgroundPtr>::~Property2() {
	Release(_transition);
	for (auto& i : _frames) {
		Release(i.value());
	}
}

template<>
void Property2<BackgroundPtr>::transition(uint f1, Action* root) {
	if ( _set_property_func ) {
		_transition = Background::assign(_transition, _frames[f1]);
		set_property(_inl_action(root)->views());
	}
}

static BackgroundPtr background_transition(BackgroundPtr tran,
																					 BackgroundPtr r1, BackgroundPtr r2, float x, float y) {
	if (!r1 || !r2) return tran;
	auto type = r1->type();
	if (type != r2->type()) return tran;
	
	if (type == Background::M_IMAGE) {
		auto img = static_cast<BackgroundImage*>(tran);
		auto img1 = r1->as_image();
		auto img2 = r2->as_image();
		
		if (!tran || tran->type() != type) {
			tran = img = new BackgroundImage();
		}
		
		auto pos1x = img1->position_x();
		auto pos1y = img1->position_y();
		auto pos2x = img2->position_x();
		auto pos2y = img2->position_y();
		auto size1x = img1->size_x();
		auto size1y = img1->size_y();
		auto size2x = img2->size_x();
		auto size2y = img2->size_y();

		if (x < 1.0) {
			img->set_texture(img1->texture());
			img->set_repeat(img1->repeat());
		} else {
			img->set_texture(img2->texture());
			img->set_repeat(img2->repeat());
		}
		
		if (pos1x.type != pos2x.type) {
			img->set_position_x(x < 1.0 ? pos1x : pos2x);
		} else {
			float value = pos1x.value - (pos1x.value - pos2x.value) * y;
			img->set_position_x(BackgroundPosition(pos1x.type, value));
		}
		if (pos1y.type != pos2y.type) {
			img->set_position_y(x < 1.0 ? pos1y : pos2y);
		} else {
			float value = pos1y.value - (pos1y.value - pos2y.value) * y;
			img->set_position_y(BackgroundPosition(pos1y.type, value));
		}
		if (size1x.type != size2x.type) {
			img->set_size_x(x < 1.0 ? size1x : size2x);
		} else {
			float value = size1x.value - (size1x.value - size2x.value) * y;
			img->set_size_x(BackgroundSize(size1x.type, value));
		}
		if (size1y.type != size2y.type) {
			img->set_size_y(x < 1.0 ? size1y : size2y);
		} else {
			float value = size1y.value - (size1y.value - size2y.value) * y;
			img->set_size_y(BackgroundSize(size1y.type, value));
		}
		
		auto next = background_transition(tran->next(), r1->next(), r2->next(), x, y);
		
		tran->set_next(next);
	} else {
		// TODO ...
	}
	return tran;
}

template<>
void Property2<BackgroundPtr>::transition(uint f1, uint f2, float x, float y, Action* root) {
	if ( _set_property_func ) {
		Background *v1 = _frames[f1], *v2 = _frames[f2];
		auto new_tran = background_transition(_transition, v1, v2, x, y);
		_transition = Background::assign(_transition, new_tran);
		set_property(_inl_action(root)->views());
	}
}

template<>
void Property2<BackgroundPtr>::fetch(uint frame, View* view) {
	if ( _get_property_func ) {
		_frames[frame] = Background::assign(_frames[frame], get_property(view));
	}
}

template<>
void Property2<BackgroundPtr>::default_value(uint frame) {
	_frames[frame] = Background::assign(_frames[frame], nullptr);
}

template<>
void Property2<BackgroundPtr>::frame(uint index, BackgroundPtr value) {
	_frames[index] = Background::assign(_frames[index], value);
}

//-------------------------------------------------------------------------------

/**
 * @class Property3
 */
template<class T, PropertyName Name> class Property3: public Property2<T> {
 public:
	
	typedef typename Property2<T>::GetPropertyFunc GetPropertyFunc;
	typedef typename Property2<T>::SetPropertyFunc SetPropertyFunc;
	
	inline Property3(uint frame_count): Property2<T>(frame_count) { }
	
	virtual void bind_view(int view_type) {
		typedef PropertysAccessor::Accessor PropertyFunc;
		PropertyFunc func = PropertysAccessor::shared()->accessor(view_type, Name);
		this->_get_property_func = reinterpret_cast<GetPropertyFunc>(func.get_accessor);
		this->_set_property_func = reinterpret_cast<SetPropertyFunc>(func.set_accessor);
	}
};

template<> void Property3<float, PROPERTY_X>::bind_view(int type) {
	this->_get_property_func = &View::x;
	this->_set_property_func = &View::set_x;
}
template<> void Property3<float, PROPERTY_Y>::bind_view(int type) {
	this->_get_property_func = &View::y;
	this->_set_property_func = &View::set_y;
}
template<> void Property3<float, PROPERTY_SCALE_X>::bind_view(int type) {
	this->_get_property_func = &View::scale_x;
	this->_set_property_func = &View::set_scale_x;
}
template<> void Property3<float, PROPERTY_SCALE_Y>::bind_view(int type) {
	this->_get_property_func = &View::scale_y;
	this->_set_property_func = &View::set_scale_y;
}
template<> void Property3<float, PROPERTY_SKEW_X>::bind_view(int type) {
	this->_get_property_func = &View::skew_x;
	this->_set_property_func = &View::set_skew_x;
}
template<> void Property3<float, PROPERTY_SKEW_Y>::bind_view(int type) {
	this->_get_property_func = &View::skew_y;
	this->_set_property_func = &View::set_skew_y;
}
template<> void Property3<float, PROPERTY_ORIGIN_X>::bind_view(int type) {
	this->_get_property_func = &View::origin_x;
	this->_set_property_func = &View::set_origin_x;
}
template<> void Property3<float, PROPERTY_ORIGIN_Y>::bind_view(int type) {
	this->_get_property_func = &View::origin_y;
	this->_set_property_func = &View::set_origin_y;
}
template<> void Property3<float, PROPERTY_ROTATE_Z>::bind_view(int type) {
	this->_get_property_func = &View::rotate_z;
	this->_set_property_func = &View::set_rotate_z;
}
template<> void Property3<float, PROPERTY_OPACITY>::bind_view(int type) {
	this->_get_property_func = &View::opacity;
	this->_set_property_func = &View::set_opacity;
}
template<> void Property3<bool, PROPERTY_VISIBLE>::bind_view(int type) {
	this->_get_property_func = &View::visible;
	this->_set_property_func = &View::set_visible_1;
}
