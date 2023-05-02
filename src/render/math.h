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

	template <typename T, int LEN> struct MVec {
		T val[LEN];
		inline T operator[](int index) const { return val[index]; }
		inline T& operator[](int index) { return val[index]; }
	};

	template <typename T> struct MVec2: public MVec<T, 2> {
		inline MVec2(): MVec2(0) {}
		inline MVec2(T f) {
			this->val[0] = f; this->val[1] = f;
		}
		inline MVec2(T a, T b) {
			this->val[0] = a; this->val[1] = b;
		}
		inline bool operator==(const MVec2& b) const {
			return this->val[0] == b.val[0] && this->val[1] == b.val[1];
		}
		inline bool operator!=(const MVec2& b) const { return !operator==(b); }
		inline MVec2<T> operator-(const MVec2& b) const {
			return MVec2<T>(this->val[0] - b[0], this->val[1] - b[1]);
		}
		inline MVec2<T> operator+(const MVec2& b) const {
			return MVec2<T>(this->val[0] + b[0], this->val[1] + b[1]);
		}
		inline MVec2<T> operator*(const MVec2& b) const {
			return MVec2<T>(this->val[0] * b[0], this->val[1] * b[1]);
		}
		inline MVec2<T> operator/(const MVec2& b) const {
			return MVec2<T>(this->val[0] / b[0], this->val[1] / b[1]);
		}
		inline MVec2<T>& operator-=(const MVec2& b) {
			this->val[0] -= b[0]; this->val[1] -= b[1]; return *this;
		}
		inline MVec2<T>& operator+=(const MVec2& b) {
			this->val[0] += b[0]; this->val[1] += b[1]; return *this;
		}
		inline MVec2<T>& operator*=(const MVec2& b) {
			this->val[0] *= b[0]; this->val[1] *= b[1]; return *this;
		}
		inline MVec2<T>& operator/=(const MVec2& b) {
			this->val[0] /= b[0]; this->val[1] /= b[1]; return *this;
		}
		inline T x() const { return this->val[0]; }
		inline T y() const { return this->val[1]; }
		inline void set_x(T v) { this->val[0] = v; }
		inline void set_y(T v) { this->val[1] = v; }
		inline bool is_zero() const {
			return this->val[0] == 0 && this->val[1] == 0;
		}
		inline bool is_zero_or() const {
			return this->val[0] == 0 || this->val[1] == 0;
		}
		/**
		 * @method length() returns vector length
		 */
		float length() const;
		float dot(const MVec2& b) const;
		MVec2 normalized() const;
		/**
		 * @method rotate90() Default to use Cartesian coordinate system
		 */
		MVec2 rotate90(bool ccw/*counter clock wise*/) const;
		/**
		 * Default to use Cartesian coordinate system
		 * @method normal() Default clockwise direction inward, screen coordinates outward
		 * @arg ccw {bool} if ccw=true then clockwise direction outward
		 */
		MVec2 normalline(const MVec2 *prev, const MVec2 *next, bool ccw) const;
		float angle(const MVec2& b) const;
	};

	template<> float        MVec2<float>::length() const;
	template<> float        MVec2<float>::dot(const MVec2& b) const;
	template<> MVec2<float> MVec2<float>::normalized() const;
	template<> MVec2<float> MVec2<float>::rotate90(bool ccw) const;
	template<> MVec2<float> MVec2<float>::normalline(const MVec2 *prev, const MVec2 *next, bool ccw) const;
	template<> float        MVec2<float>::angle(const MVec2& b) const;

	template <typename T> struct MVec3: public MVec<T, 3> {
		inline MVec3(): MVec3(0) {}
		inline MVec3(T f) {
			this->val[0] = f; this->val[1] = f; this->val[2] = f;
		}
		inline MVec3(MVec2<T> vec2, T f) {
			this->val[0] = vec2[0]; this->val[1] = vec2[1]; this->val[2] = f;
		}
		inline MVec3(T a, T b, T c) {
			this->val[0] = a; this->val[1] = b; this->val[2] = c;
		}
		inline bool operator==(const MVec3& b) const {
			return this->val[0] == b.val[0] &&
				this->val[1] == b.val[1] && this->val[2] == b.val[2];
		}
		inline bool operator!=(const MVec3& b) const { return !operator==(b); }

		inline T x() const { return this->val[0]; }
		inline T y() const { return this->val[1]; }
		inline T z() const { return this->val[2]; }
		inline const MVec2<T>& xy() const { return *(const MVec2<T>*)(this); }
		inline const MVec2<T>& yz() const { return *(const MVec2<T>*)(this->val[1]); }
		inline void set_x(T v) { this->val[0] = v; }
		inline void set_y(T v) { this->val[1] = v; }
		inline void set_z(T v) { this->val[2] = v; }
	};

	template <typename T> struct MVec4: public MVec<T, 4> {
		inline MVec4(): MVec4(0) {}
		inline MVec4(T f) {
			this->val[0] = f; this->val[1] = f; this->val[2] = f; this->val[3] = f;
		}
		inline MVec4(T a, T b, T c, T d) {
			this->val[0] = a; this->val[1] = b; this->val[2] = c; this->val[3] = d;
		}
		inline bool operator==(const MVec4& b) const {
			return  this->val[0] == b.val[0] && this->val[1] == b.val[1] &&
							this->val[2] == b.val[2] && this->val[3] == b.val[3];
		}
		inline bool operator!=(const MVec4& b) const { return !operator==(b); }

		inline T x() const { return this->val[0]; }
		inline T y() const { return this->val[1]; }
		inline T z() const { return this->val[2]; }
		inline T w() const { return this->val[3]; }
		inline void set_x(T v) { this->val[0] = v; }
		inline void set_y(T v) { this->val[1] = v; }
		inline void set_z(T v) { this->val[2] = v; }
		inline void set_w(T v) { this->val[4] = v; }
	};

	typedef MVec2<float> Vec2;
	typedef MVec3<float> Vec3;
	typedef MVec4<float> Vec4;
	typedef MVec2<int>   Vec2i;
	typedef MVec3<int>   Vec3i;
	typedef MVec4<int>   Vec4i;

	// rect
	template<typename T> struct MRect {
		T origin,size;
	};

	template<typename T> struct MRegion {
		T origin,end;
	};

	// rect int
	typedef MRect<Vec2>    Rect;
	typedef MRegion<Vec2>  Region;
	typedef MRect<Vec2i>   Recti;
	typedef MRegion<Vec2i> Regioni;

	template <typename T> struct MColor: public MVec<T, 4> {
		inline MColor(){}
		inline MColor(T r, T g, T b, T a) {
			this->val[0] = r; this->val[1] = g; this->val[2] = b; this->val[3] = a;
		}
		inline T r() const { return this->val[0]; }
		inline T g() const { return this->val[1]; }
		inline T b() const { return this->val[2]; }
		inline T a() const { return this->val[3]; }
		inline void r(T value) { this->val[0] = value; }
		inline void g(T value) { this->val[1] = value; }
		inline void b(T value) { this->val[2] = value; }
		inline void a(T value) { this->val[3] = value; }
	};

	struct Qk_EXPORT Color4f: public MColor<float> {
		Color4f(): MColor<float>(0, 0, 0, 1) {}
		Color4f(float r, float g, float b)
			: MColor<float>(r, g, b, 1) {}
		Color4f(float r, float g, float b, float a)
			: MColor<float>(r, g, b, a) {}
		bool operator==(const Color4f& color) const;
		bool operator!=(const Color4f& color) const;
	};

	struct Qk_EXPORT Color: public MColor<uint8_t> {
		static Color from(uint32_t color); //! ignore endianness, small end data as a,b,g,r
		static Color from_abgr(uint32_t abgr); //! high => low as a,b,g,r
		static Color from_rgba(uint32_t rgba); //! high => low as r,g,b,a
		Color(): MColor<uint8_t>(0, 0, 0, 255) {}
		Color(uint8_t r, uint8_t g, uint8_t b)
			: MColor<uint8_t>(r, g, b, 255) {}
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
			: MColor<uint8_t>(r, g, b, a) {}
		bool operator==(Color color) const;
		bool operator!=(Color color) const;
		Color4f  to_color4f() const;
		uint32_t to_uint32() const; //! ignore endianness, small end data as a,b,g,r
		uint32_t to_uint32_abgr() const; //! high => low as a,b,g,r
		uint32_t to_uint32_rgba() const; //! high => low as r,g,b,a
	};

	struct Qk_EXPORT Mat: public MVec<float, 6> {
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

	struct Qk_EXPORT Mat4: public MVec<float, 16> {
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
