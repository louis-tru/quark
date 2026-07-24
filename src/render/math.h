/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark_render_math__
#define __quark_render_math__

#include "../util/util.h"
#include "../util/array.h"

#define Qk_PI 3.1415926535898f                   // PI
#define Qk_PI_2 6.2831853071796f                 // PI * 2
#define Qk_PI_2_1 1.5707963267949f               // PI / 2
#define Qk_PI_RATIO_180  0.017453292519943f      // PI / 180
#define Qk_SQRT_2        1.4142135623730951f     // sqrt(2)

namespace qk {

	template <typename T, int LEN> struct Qk_EXPORT Vec {
		T val[LEN];
		inline Vec(): Vec(0) {}
		inline Vec(T f) {
			if (LEN == 2) {
				val[0] = f; val[1] = f;
			} else if (LEN == 3) {
				val[0] = f; val[1] = f; val[2] = f;
			} else if (LEN == 4) {
				val[0] = f; val[1] = f; val[2] = f; val[3] = f;
			} else {
				for (int i = 0; i < LEN; i++)
					val[i] = f;
			}
		}
		//vec2
		inline Vec(T a, T b) {
			val[0] = a; val[1] = b;
		}
		//vec3
		inline Vec(T a, T b, T c) {
			val[0] = a; val[1] = b; val[2] = c;
		}
		inline Vec(T a, const Vec<T,2>& b) {
			val[0] = a; val[1] = b[0]; val[2] = b[1];
		}
		inline Vec(const Vec<T,2>& a, T b) {
			val[0] = a[0]; val[1] = a[1]; val[2] = b;
		}
		//vec4
		inline Vec(T a, T b, T c, T d) {
			val[0] = a; val[1] = b; val[2] = c; val[3] = d;
		}
		inline Vec(T a, const Vec<T,3>& b) {
			val[0] = a; val[1] = b[0]; val[2] = b[1]; val[3] = b[2];
		}
		inline Vec(const Vec<T,3>& a, T b) {
			val[0] = a[0]; val[1] = a[1]; val[2] = a[2]; val[3] = b;
		}
		inline Vec(const Vec<T,2>& a, const Vec<T,2>& b) {
			val[0] = a[0]; val[1] = a[1]; val[2] = b[0]; val[3] = b[1];
		}
		// ------------------------------------------
		inline T operator[](int index) const {
			return val[index];
		}
		inline T& operator[](int index) {
			return val[index];
		}
		inline bool operator==(const Vec& b) const {
			for (int i = 0; i < LEN; i++)
				if (val[i] != b.val[i]) return false;
			return true;
		}
		inline bool operator!=(const Vec& b) const {
			return !operator==(b);
		}

		// ------------------------------------------
		inline Vec operator+(const Vec& b) const {
			Vec c(*this);
			for (int i = 0; i < LEN; i++) c.val[i] += b.val[i];
			return c;
		}
		inline Vec operator-(const Vec& b) const {
			Vec c(*this);
			for (int i = 0; i < LEN; i++) c.val[i] -= b.val[i];
			return c;
		}
		inline Vec operator*(const Vec& b) const {
			Vec c(*this);
			for (int i = 0; i < LEN; i++) c.val[i] *= b.val[i];
			return c;
		}
		inline Vec operator/(const Vec& b) const {
			Vec c(*this);
			for (int i = 0; i < LEN; i++) c.val[i] /= b.val[i];
			return c;
		}
		// ------------------------------------------
		inline Vec& operator+=(const Vec& b) {
			for (int i = 0; i < LEN; i++) val[i] += b.val[i];
			return *this;
		}
		inline Vec& operator-=(const Vec& b) {
			for (int i = 0; i < LEN; i++) val[i] -= b.val[i];
			return *this;
		}
		inline Vec& operator*=(const Vec& b) {
			for (int i = 0; i < LEN; i++) val[i] *= b.val[i];
			return *this;
		}
		inline Vec& operator/=(const Vec& b) {
			for (int i = 0; i < LEN; i++) val[i] /= b.val[i];
			return *this;
		}

		// ------------------------------------------
		inline T x() const { return val[0]; }
		inline T y() const { return val[1]; }
		inline T z() const { return val[2]; }
		inline T w() const { return val[3]; }

		inline T r() const { return val[0]; }
		inline T g() const { return val[1]; }
		inline T b() const { return val[2]; }
		inline T a() const { return val[3]; }

		inline T width() const { return val[0]; }
		inline T height() const { return val[1]; }
		inline T depth() const { return val[2]; }

		inline void set_x(T v) { val[0] = v; }
		inline void set_y(T v) { val[1] = v; }
		inline void set_z(T v) { val[2] = v; }
		inline void set_w(T v) { val[3] = v; }

		inline void set_r(T v) { val[0] = v; }
		inline void set_g(T v) { val[1] = v; }
		inline void set_b(T v) { val[2] = v; }
		inline void set_a(T v) { val[3] = v; }

		inline void set_width(T v) { val[0] = v; }
		inline void set_height(T v) { val[1] = v; }
		inline void set_depth(T v) { val[2] = v; }

		inline bool is_zero() const {
			for (int i = 0; i < LEN; i++) if (val[i] != 0) return false;
			return true;
		}
		inline bool is_zero_axis() const {
			for (int i = 0; i < LEN; i++) if (val[i] == 0) return true;
			return false;
		}
		template<typename M>
		inline const M& as() const {
			return *static_cast<const M*>(this);
		}
	};

	#define Qk_Vec_Operator(Name,T,Len) \
		Name(): Vec() {}; \
		Name(T f): Vec(f) {} \
		Name  operator+(const Vec<T,Len>& b) const; \
		Name  operator-(const Vec<T,Len>& b) const; \
		Name  operator*(const Vec<T,Len>& b) const; \
		Name  operator/(const Vec<T,Len>& b) const; \
		Name  operator-() const; \
		Name  operator+(T b) const; \
		Name  operator-(T b) const; \
		Name  operator*(T b) const; \
		Name  operator/(T b) const; \
		Name& operator+=(const Vec<T,Len>& b); \
		Name& operator-=(const Vec<T,Len>& b); \
		Name& operator*=(const Vec<T,Len>& b); \
		Name& operator*=(T b); \
		Name& operator/=(const Vec<T,Len>& b);\
		bool  operator==(const Vec<T,Len>& b) const; \
		bool  operator!=(const Vec<T,Len>& b) const

	// ------------------------------------------
	// Qk Coordinate System
	// 2D/UI:
	// x right+
	// y down+
	// clockwise positive rotation
	// 3D:
	// left-handed
	// positive z forward
	// clockwise positive rotation
	// Euler order: ZXY
	struct Qk_EXPORT Vec2: Vec<float,2> {
		Qk_Vec_Operator(Vec2,float,2);

		Vec2(float a, float b): Vec(a,b) {}

		/**
		 * @method length() returns vector length
		 */
		float length() const;

		/**
		 * @method lengthSq() returns vector length squared
		*/
		float lengthSq() const;

		/**
		 * @method dot() returns vector inner product
		*/
		float dot(const Vec<float,2> b) const;

		/**
		 * @method det() returns scalar outer product
		*/
		float det(const Vec<float,2> b) const;

		/**
		 * @method dot() returns normalized vector
		*/
		Vec2  normalized() const;

		/**
		 * @method rotate90z() Default to use Cartesian coordinate system
		 */
		Vec2  rotate90z() const;

		/**
		 * @method rotate270z() ccw rotate 90
		 */
		Vec2  rotate270z() const;

		/**
		 * @method rotate(radians) Default to use Cartesian coordinate system
		 */
		Vec2  rotate(float radians) const;

		/**
		 * Default to use Cartesian coordinate system
		 * 
		 * Returns zero when the previous is on the same side and on the same line as the next
		 * 
		 * @method normal() Default clockwise direction inward, screen coordinates outward
		 */
		Vec2  normalline(const Vec2 *prev, const Vec2 *next) const;

		/**
		 * @method angle() return vector angle
		*/
		float angle() const;
		
		/**
		* @method round
		*/
		Vec2 round() const;

		/**
		 * @method floor return floor of vector
		 */
		Vec2 floor() const;

		/**
		 * @method ceil return ceil of vector
		 */
		Vec2 ceil() const;

		/**
		 * @method min return min of two vector
		 */
		Vec2 min(const Vec2 &b) const;

		/**
		 * @method max return max of two vector
		 */
		Vec2 max(const Vec2 &b) const;

		/**
		 * @method angleTo(to) return vector angle
		 */
		float angleTo(const Vec2 to) const;
	};

	struct Qk_EXPORT Vec3: Vec<float,3> {
		Qk_Vec_Operator(Vec3,float,3);
		Vec3(float x, float y, float z = 0.0): Vec(x,y,z) {};
		Vec3(float x, const Vec<float, 2> &yz): Vec(x, yz) {};
		Vec3(const Vec<float, 2> &xy, float z = 0.0): Vec(xy, z) {};
		float length() const;
		float dot(const Vec<float,3>& b) const;
		Vec3  det(const Vec<float,3>& b) const;
		Vec2  xy() const { return Vec2(x(), y()); }
	};

	#undef Qk_Vec_Operator

	// ------------------------------------------

	template<typename T> struct MRect { T begin,size; }; // rect
	template<typename T> struct MRange { // range
		T begin,end;
		inline T size() const { return end - begin; }
		inline bool operator==(const MRange &b) const { return begin == b.begin && end == b.end; }
		// expand begin/end to integer values, useful for pixel coverage calculation
		inline MRange expandToInteger() const;
		// clip to another range, useful for pixel coverage calculation with a clip rect
		inline MRange clip(const MRange &clip) const;
		// join two ranges (union)
		inline MRange join(const MRange &b) const;
		inline bool isEmpty() const { return begin.x() >= end.x() || begin.y() >= end.y(); }
		inline MRange offset(T offset) const { return {begin + offset, end + offset}; }
	};
	template<typename T> struct MRegion {
		// range = (origin+begin, origin+end), size = end - begin
		T begin,end,origin;
	};
	template<typename T> struct MLimitRange { T min,max; }; // limit range

	typedef Vec<float,4>      Vec4; // typedef vec
	typedef MRect<Vec2>       Rect; // typedef rect
	typedef MRange<Vec2>      Range; // typedef range
	typedef MRegion<Vec2>     Region; // typedef region
	typedef MLimitRange<Vec2> LimitRange; // typedef limit range
	typedef Vec<int,2>        IVec2;
	typedef Vec<int,3>        IVec3;
	typedef Vec<int,4>        IVec4;
	typedef MRect<IVec2>      IRect;
	typedef MRange<IVec2>     IRange;

	struct Vec3Padding { Vec3 value; float padding; };
	struct IVec3Padding { IVec3 value; int32_t padding; };

	template<>
	inline Range Range::expandToInteger() const {
		return {begin.floor(), end.ceil()};
	}
	template<>
	inline Range Range::clip(const Range &clip) const {
		return {begin.max(clip.begin), end.min(clip.end)};
	}
	template<>
	inline Range Range::join(const Range &b) const {
		return {begin.min(b.begin), end.max(b.end)};
	}
	template<>
	Vec<float,4> Vec<float,4>::operator*(const Vec<float,4> &v) const;
	template<>
	Vec<float,4>& Vec<float,4>::operator*=(const Vec<float,4> &v);

	// ------------------------------------------

	struct Color;
	struct Qk_EXPORT Color4f: Vec<float, 4> {
		Color4f(): Vec<float, 4>(0, 0, 0, 1) {}
		Color4f(float r, float g, float b)
			: Vec<float, 4>(r, g, b, 1) {}
		Color4f(float r, float g, float b, float a)
			: Vec<float, 4>(r, g, b, a) {}
		Color4f(const Vec4 &v): Vec<float, 4>(v) {};
		bool operator==(const Color4f& color) const;
		bool operator!=(const Color4f& color) const;
		Color4f mul_alpha_only(float alpha) const;
		Color4f mul_rgb_only(const Color4f& color) const;
		Color4f mul(const Color4f& color) const;
		Color4f premul_alpha() const;
		Color4f recover_unpremul_alpha() const;
		Color to_color() const;
	};

	// Memory sort: low => high as r,g,b,a
	struct Qk_EXPORT Color: Vec<uint8_t, 4> {
		static Color from(uint32_t color); //!< ignore endianness, small end data as a,b,g,r
		static Color from_abgr(uint32_t abgr); //!< high => low as a,b,g,r
		static Color from_rgba(uint32_t rgba); //!< high => low as r,g,b,a
		Color(): Vec<uint8_t, 4>(0, 0, 0, 255) {}
		Color(uint8_t r, uint8_t g, uint8_t b)
			: Vec<uint8_t, 4>(r, g, b, 255) {}
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
			: Vec<uint8_t, 4>(r, g, b, a) {}
		bool operator==(Color color) const;
		bool operator!=(Color color) const;
		float    to_float_alpha() const;
		Color4f  mul_alpha_only(float alpha) const;
		Color4f  mul_rgb_only(const Color4f& color) const;
		Color4f  mul_color4f(const Color4f& color) const;
		Color4f  premul_alpha() const;
		Color4f  to_color4f() const;
		uint32_t to_uint32_abgr() const; //!< high => low as a,b,g,r
		uint32_t to_uint32_rgba() const; //!< high => low as r,g,b,a
		Color blendSrcOver(const Color &src) const;
	};

	struct Qk_EXPORT Mat: Vec<float, 6> {
		Mat(): Mat(1) {}
		Mat(float value);
		Mat(float m0, float m1, float m2, float m3, float m4, float m5);
		Mat(const float* values, int length = 6);
		Mat(Vec2 translate, Vec2 scale, float rotate, Vec2 skew);
		Mat& translate(Vec2 v);
		Mat& translate_x(float x);
		Mat& translate_y(float y);
		Mat& scale(Vec2 v);
		Mat& scale_x(float x);
		Mat& scale_y(float y);
		Mat& rotate(float rotate);
		Mat& skew(Vec2 v);
		Mat& skew_x(float x);
		Mat& skew_y(float y);
		Mat& set_translate(Vec2 v);
		bool operator==(const Mat& b) const;
		bool operator!=(const Mat& b) const {return !operator==(b);}
		Mat  operator*(const Mat& b) const;
		Mat& operator*=(const Mat& b);
		Vec2 operator*(const Vec2& b) const;
		Vec2 mul_vec2_no_translate(const Vec2& b) const;
		void mul_vec2_batch(Vec2* batch, int count) const;
		void mul_vec2_no_translate_batch(Vec2* batch, int count) const;
		void mul(const Mat& b, Mat& output) const;
		bool is_identity() const; // no translate, no scale, no rotate, no skew
		bool is_translate_only() const; // no scale, no rotate, no skew
		bool is_scale_only() const; // no translate, no rotate, no skew
		bool has_translation() const; // has translate, may have scale/rotate/skew
		bool has_scaling() const; // has scale, may have translate/rotate/skew
		bool has_skew() const; // has skew, may have translate/rotate/scale
		Mat inverse() const; // return inverse matrix
		String toString() const;
		Vec2 getTranslate() const { return Vec2(val[2], val[5]); }
	};

	struct Qk_EXPORT Mat4: Vec<float, 16> {
		Mat4(): Mat4(1) {}
		Mat4(float value);
		Mat4(float m0, float m1, float m2, float m3,
				float m4, float m5, float m6, float m7,
				float m8, float m9, float m10, float m11,
				float m12, float m13, float m14, float m15);
		Mat4(const float* values, int length = 16);
		Mat4(Mat mat);

		Mat4& translate(Vec3 v);
		Mat4& translate_x(float x);
		Mat4& translate_y(float y);
		Mat4& translate_z(float z);

		Mat4& scale(Vec3 v);
		Mat4& scale_x(float x);
		Mat4& scale_y(float y);
		Mat4& scale_z(float z);

		Mat4& rotate(Vec3 v);
		Mat4& rotate_x(float x);
		Mat4& rotate_y(float y);
		Mat4& rotate_z(float z);

		Mat4& skew(Vec3 v);
		Mat4& skew_x(float x);
		Mat4& skew_y(float y);
		Mat4& skew_z(float z);

		Mat4 operator*(const Mat4& b) const;
		Mat4& operator*=(const Mat4& b);
		Vec4 operator*(const Vec4& vec) const;
		void mul(const Mat4& b, Mat4& output) const;
		Mat4 transpose() const;

		/**
		 * Create a perspective projection matrix.
		 * Uses Qk screen-space / left-handed convention:
		 *   x right+
		 *   y down+
		 *   z forward+
		 * Maps camera-space z after perspective divide:
		 *   near -> -1
		 *   far  ->  1
		 * Requires:
		 *   right != left
		 *   bottom != top
		 *   far != near
		 *   near > 0
		 *   far  > near
		 */
		static Mat4 frustum(float left, float right, float top, float bottom, float near, float far);

		/**
		 * Create an orthogonal projection matrix.
		 * Uses Qk screen-space convention:
		 *   x right+
		 *   y down+
		 *   z forward+
		 * Z is mapped linearly without sign inversion:
		 *   near -> -1
		 *   far  ->  1
		 */
		static Mat4 ortho(float left, float right, float top, float bottom, float near, float far);
	};

	Qk_EXPORT float math_invSqrt(float x); // 1/sqrt(x)
	Qk_EXPORT float math_sqrt(float x);

	#define Qk_Vec_Types(Fn) Fn(Vec2) Fn(Vec3) Fn(Vec4) Fn(Color) Fn(Color4f) \
		Fn(Rect) Fn(Range) Fn(Region) Fn(LimitRange) Fn(IVec2) Fn(IVec3) Fn(IVec4) Fn(IRect)
	#define Qk_Ordinary_Type(Vec) \
	template<> struct ObjectTraits<Vec>: ObjectTraitsBase<Vec> { \
		static constexpr bool isOrdinary = true; };
	Qk_Vec_Types(Qk_Ordinary_Type)
	#undef Qk_Ordinary_Type
	#undef Qk_Vec_Types
}
#endif
