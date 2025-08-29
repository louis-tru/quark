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

#ifndef __quark__view__sprite__
#define __quark__view__sprite__

#include "./matrix.h"

namespace qk {
	class KeyframeAction;

	/**
	 * @class SpriteView
	 * @brief SpriteView is inherited from View and MatrixView.
	*/
	class Qk_EXPORT SpriteView: public View, public MatrixView {
	public:
		SpriteView();
		MatrixView* asMatrixView() override;
		Vec2 layout_offset_inside() override;
		bool overlap_test(Vec2 point) override; // Check if the point overlaps with the sprite
		void solve_marks(const Mat &mat, View *parent, uint32_t mark) override;
		void trigger_listener_change(uint32_t name, int count, int change) override;
	private:
		Vec2 _vertex[4]; // The rect vertex of the sprite
		bool _vertex_ok; // is computed vertex
	};

	/**
	 * @class Sprite
	 * @brief Sprite is a view that can display an image or animation.
	 * It supports matrix transformations and can be used to create animated sprites.
	 * It can be used in a layout like this:
	 * @example
	 * ```tsx
	 * <sprite width={100} height={100} originX="auto" originY="100%">
	 *   <sprite src="path/to/image.png" width={100} height={100} frames={10} fsp={24} direction="row" />
	 *   <sprite src="path/to/another_image.png" width={200} height={200} frames={20} fsp={30} direction="column" />
	 * </sprite>
	 * ```
	*/
	class Qk_EXPORT Sprite: public SpriteView, public ImageSourceHold {
	public:
		Qk_DEFINE_VIEW_ACCESSOR(String, src, Const); // The source of the sprite image
		Qk_DEFINE_VIEW_PROPERTY(float, width, Const); // The width of the sprite frame
		Qk_DEFINE_VIEW_PROPERTY(float, height, Const); // The height of the sprite frame
		Qk_DEFINE_VIEW_PROPERTY(uint16_t, frame, Const); // The current frame of the sprite animation, default 0
		Qk_DEFINE_VIEW_PROPERTY(uint16_t, frames, Const); // The number of frames in the sprite animation, default 1
		Qk_DEFINE_VIEW_PROPERTY(uint16_t, item, Const); // The current item of the sprite animation, default 0
		Qk_DEFINE_VIEW_PROPERTY(uint16_t, items, Const); // The number of items in the sprite animation, default 1
		Qk_DEFINE_VIEW_PROPERTY(uint8_t, gap, Const); // The gap between frames in the sprite animation, default 0
		Qk_DEFINE_VIEW_PROPERTY(Direction, direction, Const); // The direction of the sprite animation, default horizontal row
		Qk_DEFINE_ACCESSOR(uint8_t, fsp, Const); // The frame per second of the sprite animation, default 25
		Qk_DEFINE_ACCESSOR(bool, playing, Const); // Whether the sprite is currently playing
		Sprite(); // Constructor
		void destroy() override;
		void play(bool all = false); // Play the sprite frames, play action of view together if the all equals true
		void stop(bool all = false); // Stop the sprite frames, stop action of view together if the all equals true
		ViewType viewType() const override;
		Vec2 client_size() override;
		void draw(UIDraw *render) override;
	protected:
		void onSourceState(Event<ImageSource, ImageSource::State>& evt) override;
		ImagePool* imgPool() override;
		View* init(Window* win) override;
	private:
		KeyframeAction *_keyAction; // The keyframe action for the sprite animation
		friend class UIDraw;
	};
}
#endif