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

#include "./sdf.h"
#include "./math.h"
#include <algorithm>

using namespace std;
namespace qk {

	// =============================
	// Maximum integer value for distance initialization
	// =============================
	constexpr int INF = 0x3f3f3f3f;

	// =============================
	// Chamfer 8SSEDT distance transform parameters
	// =============================
	// Binary image: 1 = feature pixel, 0 = background
	// We compute integer distances to the nearest feature using Chamfer units.
	// Chamfer weights:
	//   ORTH = orthogonal step (up, down, left, right)
	//   DIAG = diagonal step
	// Principle: choose powers of 2 for fast integer addition; DIAG ≈ ORTH * sqrt(2)
	// Using integers avoids multiplications/divisions, ensuring speed and cache friendliness
	// 1.4142135623730951 = sqrt(2)
	// 1.4140625 = 181/128 ≈ sqrt(2)
	constexpr int ORTH = 128; // orthogonal step, 2^7
	constexpr int DIAG = 181; // diagonal step ≈ ORTH * sqrt(2)

	// =============================
	// Core Chamfer 8SSEDT function
	// =============================
	// We use integer weights: orthogonal = 128, diagonal = 181 (common simple chamfer)
	// Two passes: forward (top-left -> bottom-right), backward (bottom-right -> top-left)
	// This implementation uses only add/min, no multiply/divide.
	// Unfortunately, 128 will lose half of the grayscale anti aliasing information.
	// ORTH=128 will lose some grayscale anti-aliasing resolution, but is sufficient for most 8-bit SDFs
	void chamfer_8ssetd(int w, int h, Array<int>& out_dist) {
		// Forward pass
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				int idx = y*w + x;
				int v = out_dist[idx];
				if (x > 0) v = min(v, out_dist[idx-1] + ORTH);              // left
				if (y > 0) v = min(v, out_dist[idx-w] + ORTH);              // up
				if (x > 0 && y > 0) v = min(v, out_dist[idx-w-1] + DIAG);   // up-left
				if (x+1 < w && y > 0) v = min(v, out_dist[idx-w+1] + DIAG); // up-right (from previous row)
				out_dist[idx] = v;
			}
		}
		// Backward pass
		for (int y = h-1; y >= 0; --y) {
			for (int x = w-1; x >= 0; --x) {
				int idx = y*w + x;
				int v = out_dist[idx];
				if (x+1 < w) v = min(v, out_dist[idx+1] + ORTH);              // right
				if (y+1 < h) v = min(v, out_dist[idx+w] + ORTH);              // down
				if (x+1 < w && y+1 < h) v = min(v, out_dist[idx+w+1] + DIAG); // down-right
				if (x > 0 && y+1 < h) v = min(v, out_dist[idx+w-1] + DIAG);   // down-left
				out_dist[idx] = v;
			}
		}
	}

	// =============================
	// Compute unsigned SDF (distance field)
	// =============================
	// Input: binary image
	// Output: integer distance array
	//
	// Algorithm:
	//   Background pixels (0) are initialized with INF
	//   Feature pixels (255) are initialized with 0
	//   Gray pixels are approximated using half distance
	Array<int> compute_unsigned_distance(const uint8_t *bin, int w, int h, int stride) {
		auto size = w * h;
		Array<int> d_fore(size);
		for (int i = 0; i < size; i++) {
			if (bin[0] == 0) {
				d_fore[i] = INF;
			} else if (bin[0] == 255) {
				d_fore[i] = 0;
			} else {
				d_fore[i] = (255 - bin[0]) >> 1; // approximate mid-gray
			}
			bin += stride;
		}
		chamfer_8ssetd(w, h, d_fore);
		Qk_ReturnLocal(d_fore);
	}

	// =============================
	// Compute signed SDF
	// =============================
	// Input: binary image
	// Output: integer distance array with sign
	//   Positive inside features, negative outside
	//
	// Algorithm:
	//   Uses two distance arrays: d_fore (foreground) and d_back (background)
	//   Initialization:
	//     bin=0 -> d_fore=INF, d_back=0
	//     bin=255 -> d_fore=0, d_back=INF
	//     Gray pixels -> approximate mid-distance
	//   Perform chamfer transform on both arrays
	//   Final internal distance = ORTH - background distance
	Array<int> compute_signed_distance(const uint8_t *bin, int w, int h, int stride) {
		auto size = w * h;
		Array<int> d_fore(size), d_back(size);
		const uint8_t *src = bin;

		// Initialize distance arrays
		for (int i = 0; i < size; ++i) {
			if (src[0] == 0) {
				d_fore[i] = INF;
				d_back[i] = 0;
			} else if (src[0] == 255) {
				d_fore[i] = 0;
				d_back[i] = INF;
			} else {
				d_back[i] = src[0] >> 1;
				d_fore[i] = 128 - d_back[i];
			}
			src += stride;
		}

		// Forward and backward Chamfer transform
		chamfer_8ssetd(w, h, d_fore);
		chamfer_8ssetd(w, h, d_back);

		// Correct internal distances
		for (int i = 0; i < size; i++) {
			if (bin[i]) {
				d_fore[i] = 128 - d_back[i];
			}
		}
		Qk_ReturnLocal(d_fore);
	}

	// =============================
	// General distance interface
	// =============================
	Array<int> compute_distance(const uint8_t *bin, int w, int h, int stride, bool is_signed) {
		return is_signed ? compute_signed_distance(bin, w, h, stride)
											: compute_unsigned_distance(bin, w, h, stride);
	}

	// =============================
	// Compute normalized float SDF
	// =============================
	// Outputs:
	//   is_signed=true  -> signed float SDF in [-1,1] approx
	//   is_signed=false -> unsigned float SDF in [0,1] approx
	//
	// Normalization:
	//   Multiply each integer distance by 1/128 to map Chamfer units to float
	Pixel compute_distance_f32(const uint8_t *bin, int w, int h, int stride, bool is_signed) {
		Array<float> f32(w * h);
		auto f32p = f32.val();
		for (auto i: compute_distance(bin, w, h, stride, is_signed)) {
			*(f32p++) = i * (1.0f / 128.0f);
		}
		return Pixel({
			w, h, is_signed ? kSDF_F32_ColorType : kSDF_Unsigned_F32_ColorType, kUnknown_AlphaType
		}, Buffer((char*)f32.collapse(), w*h*sizeof(float)));
	}

} // namespace qk
