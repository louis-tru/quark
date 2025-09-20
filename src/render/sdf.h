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

#ifndef __quark_render__sdf__
#define __quark_render__sdf__

#include "./pixel.h"

namespace qk {

	/**
	 * Compute integer SDF (signed or unsigned)
	 *
	 * High-level interface for computing the distance field.
	 *
	 * @param bin       Pointer to input binary image (uint8_t), 
	 *                  where 0 = background, 255 = feature (gray values allowed)
	 * @param w         Width of the image
	 * @param h         Height of the image
	 * @param is_signed If true, compute signed distance field:
	 *                    positive outside feature, negative inside
	 *                  If false, compute unsigned distance field
	 *                  Default: false
	 * @return Array<int> containing integer distance values (Chamfer units)
	 *         ORTH = 128, DIAG = 181
	 */
	Qk_EXPORT Array<int> compute_distance(const uint8_t *bin, int w, int h, bool is_signed = false);

	/**
	 * Compute normalized float SDF (signed or unsigned)
	 *
	 * Computes the distance field and converts it to float for rendering or visualization.
	 *
	 * @param bin       Pointer to input binary image (uint8_t)
	 * @param w         Width of the image
	 * @param h         Height of the image
	 * @param is_signed If true, output signed float SDF in [-1,1]
	 *                  If false, output unsigned float SDF in [0,1]
	 *                  Default: false
	 * @return Pixel object containing normalized float distance field:
	 *         - Width/height same as input
	 *         - ColorType indicates signed or unsigned float SDF
	 *         - Buffer holds float array of size w*h (row-major order)
	 *
	 * Notes:
	 *   Each integer Chamfer distance is multiplied by 1/128 to normalize.
	 *   Suitable for direct rendering or shader usage.
	 */
	Qk_EXPORT Pixel compute_distance_f32(const uint8_t *bin, int w, int h, bool is_signed = false);

} // namespace qk


#endif