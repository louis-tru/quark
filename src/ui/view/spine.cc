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

#include <spine/spine.h>
#include "./spine.h"
#include "../../util/fs.h"

using namespace spine;

namespace qk {

	Spine::Spine(): SpriteView()
		, _atlas(nullptr)
		, _attachmentLoader(nullptr)
		, _skeleton(nullptr)
		, _clipper(nullptr)
		, _effect(nullptr)
	{
	}

	void Spine::destroy() {
		delete _skeleton->getData();
		delete _skeleton;
		delete _atlas;
		delete _attachmentLoader;
		delete _clipper;
		delete _effect;
		View::destroy(); // Call parent destroy
	}

	void Spine::set_skeleton(String val, bool isRt) {
		if (isRt)
			return;
		if (val != _skeleton) {
			auto name = fs_basename(val);
			_skeleton = val;
		}
	}

	void Spine::set_atlas(String val, bool isRt) {
		if (isRt)
			return;
		if (val != _atlas) {
			_atlas = val;
		}
	}

	ViewType Spine::viewType() const {
		return kSpine_ViewType;
	}

	Vec2 Spine::client_size() {
		// TODO ...
		return Vec2();
	}

}
