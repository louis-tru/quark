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

#ifndef __ftr__action___property__
#define __ftr__action___property__

#include "./_action.h"

namespace ftr {

	// set_property
	template<> void Property2<TextColor>::set_property(List<View*>& views);
	template<> void Property2<TextSize>::set_property(List<View*>& views);
	template<> void Property2<TextStyle>::set_property(List<View*>& views);
	template<> void Property2<TextFamily>::set_property(List<View*>& views);
	template<> void Property2<TextLineHeight>::set_property(List<View*>& views);
	template<> void Property2<TextShadow>::set_property(List<View*>& views);
	template<> void Property2<TextDecoration>::set_property(List<View*>& views);
	template<> void Property2<TextOverflow>::set_property(List<View*>& views);
	template<> void Property2<TextWhiteSpace>::set_property(List<View*>& views);

	// get_property
	template<> TextColor Property2<TextColor>::get_property(View* view);
	template<> TextSize Property2<TextSize>::get_property(View* view);
	template<> TextStyle Property2<TextStyle>::get_property(View* view);
	template<> TextFamily Property2<TextFamily>::get_property(View* view);
	template<> TextLineHeight Property2<TextLineHeight>::get_property(View* view);
	template<> TextShadow Property2<TextShadow>::get_property(View* view);
	template<> TextDecoration Property2<TextDecoration>::get_property(View* view);
	template<> TextOverflow Property2<TextOverflow>::get_property(View* view);
	template<> TextWhiteSpace Property2<TextWhiteSpace>::get_property(View* view);

	// transition

	template<>
	void Property2<bool>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<Vec2>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<Color>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<TextAlign>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<Align>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<ContentAlign>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<Repeat>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<Border>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<Shadow>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<Value>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<TextColor>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<TextSize>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<TextStyle>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<TextFamily>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<TextLineHeight>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<TextShadow>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<TextDecoration>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<TextOverflow>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<TextWhiteSpace>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);
	template<>
	void Property2<String>::transition(uint32_t f1, uint32_t f2, float x, float t, Action* root);

	//-------------------------------------------------------------------------------

	template<>
	Property2<BackgroundPtr>::~Property2();

	template<>
	void Property2<BackgroundPtr>::transition(uint32_t f1, Action* root);

	template<>
	void Property2<BackgroundPtr>::transition(uint32_t f1, uint32_t f2, float x, float y, Action* root);

	template<>
	void Property2<BackgroundPtr>::fetch(uint32_t frame, View* view);

	template<>
	void Property2<BackgroundPtr>::default_value(uint32_t frame);

	template<>
	void Property2<BackgroundPtr>::frame(uint32_t index, BackgroundPtr value);

	//-----------------------------------Property3--------------------------------------------

	template<> void Property3<float, PROPERTY_X>::bind_view(int type);
	template<> void Property3<float, PROPERTY_Y>::bind_view(int type);
	template<> void Property3<float, PROPERTY_SCALE_X>::bind_view(int type);
	template<> void Property3<float, PROPERTY_SCALE_Y>::bind_view(int type);
	template<> void Property3<float, PROPERTY_SKEW_X>::bind_view(int type);
	template<> void Property3<float, PROPERTY_SKEW_Y>::bind_view(int type);
	template<> void Property3<float, PROPERTY_ORIGIN_X>::bind_view(int type);
	template<> void Property3<float, PROPERTY_ORIGIN_Y>::bind_view(int type);
	template<> void Property3<float, PROPERTY_ROTATE_Z>::bind_view(int type);
	template<> void Property3<float, PROPERTY_OPACITY>::bind_view(int type);
	template<> void Property3<bool, PROPERTY_VISIBLE>::bind_view(int type);

}

#endif
