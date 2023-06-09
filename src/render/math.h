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

#ifndef __quark_render_math__
#define __quark_render_math__

#include "../util/util.h"
#include "../util/array.h"

#define Qk_PI 3.1415926535898f                   // PI
#define Qk_PI2 1.5707963267949f                  // PI / 2
#define Qk_PI_RATIO_180  0.017453292519943f      // PI / 180

namespace qk {

	template <typename T, int LEN> struct Qk_EXPORT Vec {
		inline Vec(): Vec(0) {}
		inline Vec(T f) {
			for (int i = 0; i < LEN; i++) val[i] = f;
		}
		inline Vec(T a, T b) {
			val[0] = a; val[1] = b;
		}
		inline Vec(T a, T b, T c) {
			val[0] = a; val[1] = b; val[2] = c;
		}
		inline Vec(T a, T b, T c, T d) {
			val[0] = a; val[1] = b; val[2] = c; val[3] = d;
		}
		inline Vec(Vec<T, 2> vec2, T f) {
			val[0] = vec2[0]; val[1] = vec2[1]; val[2] = f;
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
			for (int i = 0; i < LEN; i++) c.val[i] += b.val[i]; return c;
		}
		inline Vec operator-(const Vec& b) const {
			Vec c(*this);
			for (int i = 0; i < LEN; i++) c.val[i] -= b.val[i]; return c;
		}
		inline Vec operator*(const Vec& b) const {
			Vec c(*this);
			for (int i = 0; i < LEN; i++) c.val[i] *= b.val[i]; return c;
		}
		inline Vec operator/(const Vec& b) const {
			Vec c(*this);
			for (int i = 0; i < LEN; i++) c.val[i] /= b.val[i]; return c;
		}
		// ------------------------------------------
		inline Vec& operator+=(const Vec& b) {
			for (int i = 0; i < LEN; i++) val[i] += b.val[i]; return *this;
		}
		inline Vec& operator-=(const Vec& b) {
			for (int i = 0; i < LEN; i++) val[i] -= b.val[i]; return *this;
		}
		inline Vec& operator*=(const Vec& b) {
			for (int i = 0; i < LEN; i++) val[i] *= b.val[i]; return *this;
		}
		inline Vec& operator/=(const Vec& b) {
			for (int i = 0; i < LEN; i++) val[i] /= b.val[i]; return *this;
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

		inline void set_x(T v) { val[0] = v; }
		inline void set_y(T v) { val[1] = v; }
		inline void set_z(T v) { val[2] = v; }
		inline void set_w(T v) { val[3] = v; }

		inline void set_r(T v) { val[0] = v; }
		inline void set_g(T v) { val[1] = v; }
		inline void set_b(T v) { val[2] = v; }
		inline void set_a(T v) { val[3] = v; }

		// ------------------------------------------
		inline bool is_zero() const {
			for (int i = 0; i < LEN; i++) if (val[i] != 0) return false;
			return true;
		}
		inline bool is_zero_or() const {
			for (int i = 0; i < LEN; i++) if (val[i] == 0) return true;
			return false;
		}
		/**
		 * @method length() returns vector length
		 */
		float length() const;
		float dot(const Vec& b) const;
		Vec   normalized() const;
		/**
		 * @method rotate90z() Default to use Cartesian coordinate system
		 */
		Vec   rotate90z(bool ccw/*counter clock wise*/) const;
		/**
		 * Default to use Cartesian coordinate system
		 * @method normal() Default clockwise direction inward, screen coordinates outward
		 * @arg ccw {bool} if ccw=true then clockwise direction outward
		 */
		Vec   normalline(const Vec *prev, const Vec *next, bool ccw) const;
		float angle(const Vec& b) const;

		// ------------------------------------------
		T val[LEN];
	};

	template<>               Vec<float,2>::Vec(float f);
	template<>               Vec<int,2>::Vec(int f);
	template<> bool          Vec<float,2>::is_zero() const;
	template<> bool          Vec<float,2>::is_zero_or() const;
	template<> float         Vec<float,2>::length() const;
	template<> float         Vec<float,2>::dot(const Vec& b) const;
	template<> Vec<float,2>  Vec<float,2>::normalized() const;
	template<> Vec<float,2>  Vec<float,2>::rotate90z(bool ccw) const;
	template<> Vec<float,2>  Vec<float,2>::normalline(const Vec *prev, const Vec *next, bool ccw) const;
	template<> float         Vec<float,2>::angle(const Vec& b) const;
	template<> bool          Vec<float,2>::operator==(const Vec& b) const;
	template<> bool          Vec<int,2>::operator==(const Vec& b) const;

	template<typename T> 
	struct                   MRect { T origin,size; }; // rect
	template<typename T> 
	struct                   MRegion { T origin,end;}; // region

	// typedef vec
	typedef Vec<float,2>     Vec2;
	typedef Vec<float,3>     Vec3;
	typedef Vec<float,4>     Vec4;
	typedef Vec<int,2>       iVec2;
	typedef Vec<int,3>       iVec3;
	typedef Vec<int,4>       iVec4;
	// typedef rect
	typedef MRect<Vec2>      Rect;
	typedef MRegion<Vec2>    Region;
	typedef MRect<iVec2>     iRect;
	typedef MRegion<iVec2>   iRegion;

	struct Qk_EXPORT Color4f: Vec<float, 4> {
		Color4f(): Vec<float, 4>(0, 0, 0, 1) {}
		Color4f(float r, float g, float b)
			: Vec<float, 4>(r, g, b, 1) {}
		Color4f(float r, float g, float b, float a)
			: Vec<float, 4>(r, g, b, a) {}
		bool operator==(const Color4f& color) const;
		bool operator!=(const Color4f& color) const;
	};

	struct Qk_EXPORT Color: Vec<uint8_t, 4> {
		static Color from(uint32_t color); //! ignore endianness, small end data as a,b,g,r
		static Color from_abgr(uint32_t abgr); //! high => low as a,b,g,r
		static Color from_rgba(uint32_t rgba); //! high => low as r,g,b,a
		Color(): Vec<uint8_t, 4>(0, 0, 0, 255) {}
		Color(uint8_t r, uint8_t g, uint8_t b)
			: Vec<uint8_t, 4>(r, g, b, 255) {}
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
			: Vec<uint8_t, 4>(r, g, b, a) {}
		bool operator==(Color color) const;
		bool operator!=(Color color) const;
		Color4f  to_color4f() const;
		uint32_t to_uint32() const; //! ignore endianness, small end data as a,b,g,r
		uint32_t to_uint32_abgr() const; //! high => low as a,b,g,r
		uint32_t to_uint32_rgba() const; //! high => low as r,g,b,a
	};

	struct Qk_EXPORT Mat: Vec<float, 6> {
		inline Mat(): Mat(1) {}
		Mat(float value);
		Mat(float m0, float m1, float m2, float m3, float m4, float m5);
		Mat(const float* values, int length = 6);
		Mat(Vec2 translate, Vec2 scale, float rotate, Vec2 skew);
		void translate(float x, float y);
		void translate_x(float x);
		void translate_y(float y);
		void scale(float x, float y);
		void scale_x(float x);
		void scale_y(float y);
		void rotate(float rotate);
		void skew(float x, float y);
		void skew_x(float x);
		void skew_y(float y);
		Mat  operator*(const Mat& b) const;
		Mat& operator*=(const Mat& b);
		Vec2 operator*(const Vec2& b) const;
		void mul(const Mat& b, Mat& output) const;
	};

	struct Qk_EXPORT Mat4: Vec<float, 16> {
		inline Mat4(): Mat4(1) {}
		Mat4(float value);
		Mat4(float m0, float m1, float m2, float m3,
				float m4, float m5, float m6, float m7,
				float m8, float m9, float m10, float m11,
				float m12, float m13, float m14, float m15);
		Mat4(const float* values, int length = 16);
		Mat4(Mat mat);

		void translate(float x, float y, float z);
		void translate_x(float x);
		void translate_y(float y);
		void translate_z(float z);
		
		void scale(float x, float y, float z);
		void scale_x(float x);
		void scale_y(float y);
		void scale_z(float z);

		void rotate(float x, float y, float z);
		void rotate_x(float x);
		void rotate_y(float y);
		void rotate_z(float z);

		void skew(float x, float y, float z);
		void skew_x(float x);
		void skew_y(float y);
		void skew_z(float z);

		Mat4 operator*(const Mat4& b) const;
		Mat4& operator*=(const Mat4& b);
		Vec4 operator*(const Vec4& vec) const;
		void mul(const Mat4& b, Mat4& output) const;
		void transpose();
		
		/**
		 * @method frustum Create a perspective frustum matrix
		 */
		static Mat4 frustum(float left, float right, float top, float bottom, float near, float far);
		
		/**
		 * @method ortho Create an orthogonal projection matrix
		 */
		static Mat4 ortho(float left, float right, float top, float bottom, float near, float far);
	};

	Qk_DEF_ARRAY_SPECIAL(Vec2);

	Qk_EXPORT float math_invSqrt(float x); // 1/sqrt(x)
	Qk_EXPORT float math_sqrt(float x);
}
#endif