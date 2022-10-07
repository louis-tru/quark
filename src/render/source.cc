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

#include "../app.h"
#include "./source.h"
#include "../pre_render.h"
#include "../util/fs.h"

namespace quark {

	// -------------------- I m a g e . S o u r c e --------------------

	ImageSource::ImageSource(cString& uri)
		: Qk_Init_Event(State)
		, _uri(fs_reader()->format(uri))
		, _state(STATE_NONE)
		, _load_id(0), _size(0), _used(0)
		, _inl(nullptr)
	{
	}

	/**
		* @func ready() async ready
		*/
	bool ImageSource::ready() {
		_used++;
		if (is_ready()) {
			return true;
		}
		if (_state & STATE_DECODEING) {
			return false;
		}
		_state = State(_state | STATE_DECODEING);
		
		if (_state & STATE_LOAD_COMPLETE) {
			RunLoop::first()->post(Cb([this](CbData& e){
				Qk_Trigger(State, _state);
				_Decode();
			}, this));
		} else { // load and decode
			load();
		}
		return false;
	}

}
