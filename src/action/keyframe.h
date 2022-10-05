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

#ifndef __quark__action__keyframe__
#define __quark__action__keyframe__

#include "./action.h"
#include "../util/dict.h"

Qk_NAMESPACE_START

/**
* @class KeyframeAction
*/
class Qk_EXPORT KeyframeAction: public Action {
public:
	
	class Qk_EXPORT Property {
	public:
		virtual ~Property() { }
		virtual void bind_view(int view_type) = 0;
		virtual void transition(uint32_t frame1, Action* root) = 0;
		virtual void transition(uint32_t frame1, uint32_t frame2, float x, float t, Action* root) = 0;
		virtual void add_frame() = 0;
		virtual void fetch(uint32_t frame, View* view) = 0;
		virtual void default_value(uint32_t frame) = 0;
	};
	
	class Qk_EXPORT Frame: public Object {
		Qk_HIDDEN_ALL_COPY(Frame);
	public:
		inline Frame(KeyframeAction* host, uint32_t index, const FixedCubicBezier& curve)
		: _host(host) , _index(index) , _curve(curve), _time(0) {}
		
		/**
		* @func index
		*/
		inline uint32_t index() const { return _index; }
		
		/**
		* @func time get
		*/
		inline uint64_t time() const { return _time; }
		
		/**
		* @func time set
		*/
		void set_time(uint64_t value);
		
		/*
		* @func host
		*/
		inline KeyframeAction* host() { return _host; }
		
		/**
		* @func curve get
		*/
		inline FixedCubicBezier& curve() { return _curve; }
		
		/**
		* @func curve get
		*/
		inline const FixedCubicBezier& curve() const { return _curve; }
		
		/**
		* @func curve set
		*/
		inline void set_curve(const FixedCubicBezier& value) { _curve = value; }
		
		/**
		* @func fetch property value
		*/
		void fetch(View* view = nullptr);
		
		/**
		* @func flush recovery default property value
		*/
		void flush();
		
		#define fx_def_property(ENUM, TYPE, NAME) \
			void set_##NAME(TYPE value); TYPE NAME();
		Qk_EACH_PROPERTY_TABLE(fx_def_property)
		#undef fx_def_property
	
	private:
		KeyframeAction*   _host;
		uint32_t          _index;
		FixedCubicBezier  _curve;
		uint64_t          _time;
		
		Qk_DEFINE_INLINE_CLASS(Inl);
		friend class KeyframeAction;
	};
	
	/**
	* @constructor
	*/
	inline KeyframeAction(): _frame(-1), _time(0), _bind_view_type(0) {}
	
	/**
	* @destructor
	*/
	virtual ~KeyframeAction();
	
	/**
	* @overwrite
	*/
	virtual KeyframeAction* as_keyframe() { return this; }
	
	/**
	* @func has_property
	*/
	bool has_property(PropertyName name);
	
	/**
	* @func match_property
	*/
	bool match_property(PropertyName name);
	
	/**
	* @func first
	*/
	inline Frame* first() { return _frames[0]; }
	
	/**
	* @func last
	*/
	inline Frame* last() { return _frames[_frames.size() - 1]; }
	
	/**
	* @func frame
	*/
	inline Frame* frame(uint32_t index) { return _frames[index]; }
	
	/**
	* @func operator[]
	*/
	inline Frame* operator[](uint32_t index) { return _frames[index]; }
	
	/**
	* @func length
	*/
	inline uint32_t length() const { return (uint32_t)_frames.size(); }
	
	/**
	* @func position get play frame position
	*/
	inline int position() const { return _frame; }
	
	/**
	* @func time get play time position
	*/
	inline int64_t time() const { return _time; }
	
	/**
	* @func add new frame
	*/
	Frame* add(uint64_t time, const FixedCubicBezier& curve = EASE);
	
	/**
	* @func clear all frame and property
	*/
	virtual void clear();
	
	/**
	* @func is_bind_view
	*/
	inline bool is_bind_view() { return _bind_view_type; }
		
private:
	/**
	* @overwrite
	*/
	virtual uint64_t advance(uint64_t time_span, bool restart, Action* root);
	virtual void seek_time(uint64_t time, Action* root);
	virtual void seek_before(int64_t time, Action* child);
	virtual void bind_view(View* view);
	
	typedef Dict<PropertyName, Property*> Propertys;
	
	int           _bind_view_type;
	int           _frame;
	int64_t       _time;
	Array<Frame*> _frames;
	
	Propertys     _property;

	Qk_DEFINE_INLINE_CLASS(Inl);
};

Qk_NAMESPACE_END
#endif