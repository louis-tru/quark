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

#ifndef __quark__action__keyframe__
#define __quark__action__keyframe__

#include "./action.h"
#include "../view_prop.h"
#include "../css/css.h"
#include "../../render/bezier.h"

namespace qk {

	/**
	* @class Keyframe
	*/
	class Qk_EXPORT Keyframe: public StyleSheets {
	public:
		Qk_DEFINE_PROP_GET(uint32_t, index, Const);
		Qk_DEFINE_PROP_GET(uint32_t, time, Const);
		Qk_DEFINE_PROP_GET(Curve, curve, Const);
		// @overwrite
		virtual Window* getWindowForAsyncSet() override;
		/**
		 * @method destroy()
		 */
		virtual void destroy() override;
	private:
		// @constructor
		// @private
		Keyframe(KeyframeAction* host, cCurve& curve);
		/**
		 * @method onMake trigger new prop make
		 * @overwrite
		 * @safe Rt
		*/
		void onMake(ViewProp key, Property* prop) override;

		KeyframeAction* _host;
		friend class KeyframeAction;
	};

	/**
	* @class KeyframeAction
	*/
	class Qk_EXPORT KeyframeAction: public Action {
	public:
		// Props
		Qk_DEFINE_PROP_GET(uint32_t, time, Const); //@safe Rt get, play time
		Qk_DEFINE_PROP_GET(uint32_t, frame, Const); //@safe Rt get, play frame

		KeyframeAction(Window *win);

		/**
		* @method length
		*/
		inline uint32_t length() const {
			return _frames.length();
		}

		/**
		* @method operator[]
		*/
		inline Keyframe* operator[](uint32_t index) {
			return _frames[index];
		}

		/**
		* @method has_property
		* @safe Rt
		*/
		bool hasProperty(ViewProp name);

		/**
		* @method addFrame() add new frame to end
		*/
		Keyframe* addFrame(uint32_t timeMs, cCurve& curve = EASE);

		/**
		* @method addFrameWithCss() add new frame to end with css name exp
		*/
		Keyframe* addFrameWithCss(cString& cssExp, uint32_t *timeMs = nullptr, cCurve *curve = nullptr);

		/**
		 * @overwrite
		*/
		void clear() override;

		/**
		 * @method MakeSSTransition()
		*/
		static KeyframeAction* MakeSSTransition(
			View *view, StyleSheets *to, uint32_t time, bool isRt = false
		);

	private:
		virtual void append(Action *child) throw(Error);
		virtual uint32_t advance_Rt(uint32_t time_span, bool restart, Action* root);
		virtual void seek_time_Rt(uint32_t time, Action* root);
		virtual void seek_before_Rt(uint32_t time, Action* child);
		Keyframe* add_unsafe(uint32_t time, cCurve& curve, bool isRt);

		Array<Keyframe*> _frames;
		Array<Keyframe*> _frames_Rt;
		friend class Keyframe;
	};

}
#endif
