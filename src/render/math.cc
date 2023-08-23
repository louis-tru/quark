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

#include <string.h>
#include <math.h>
#include "./math.h"

#define Qk_ARRAY_NO_IMPL 1

#include "../util/array.cc"

namespace qk {
	Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(Vec2,0);
	Qk_DEF_ARRAY_SPECIAL_IMPLEMENTATION(Vec3,0);
	
	float math_invSqrt(float x) {
#if Qk_Soft_Sqrt
		float xhalf = 0.5 * x;
		int i = *(int*)&x; // get bits for floating value
		i = 0x5f3759df - (i >> 1); // gives initial guess
		//i = 0x5f375a86 - (i >> 1); // gives initial guess
		x = *(float*)&i; // convert bits back to float
		x = x * (1.5 - xhalf * x * x); // Newton step
		return x;
#else
		return 1.0/sqrtf(x);
#endif
	}
	
	float math_sqrt(float x) {
#if Qk_Soft_Sqrt
#if Qk_Fast_Sqrt_High
		float xhalf = 0.5 * x;
		int i = *(int*)&x; // get bits for floating value
		i = 0x5f3759df - (i >> 1); // gives initial guess
		float y = *(float*)&i;
		y = y * (1.5 - xhalf * y * y); // Newton step
		y = y * (1.5 - xhalf * y * y);
		return x * y;
#else
		return 1.0 / math_invSqrt(x);
#endif
#else
		return sqrtf(x);
#endif
	}

	template<>      Vec<float,6>::Vec() {}
	template<>      Vec<float,16>::Vec() {}
	template<> Vec<int,2>::Vec(int f): Vec(f,f) {
	}
	template<> bool Vec<int,2>::operator==(const Vec& b) const {
		return val[0] == b.val[0] && val[1] == b.val[1];
	}
	template<> bool Vec<float,2>::is_zero() const {
		return val[0] == 0 && val[1] == 0;
	}
	template<> bool Vec<float,2>::is_zero_axis() const {
		return val[0] == 0 || val[1] == 0;
	}

	// ------------------------------------------

	Vec2::Vec2(): Vec(0,0) {
	}
	Vec2::Vec2(float f): Vec(f,f) {
	}
	Vec2::Vec2(const Vec<float,2>& v): Vec(v) {
	}
	Vec2::Vec2(float a, float b): Vec(a,b) {
	}
	Vec2 Vec2::operator+(const Vec<float,2>& b) const {
		return Vec2(val[0] + b.val[0], val[1] + b.val[1]);
	}
	Vec2 Vec2::operator-(const Vec<float,2>& b) const {
		return Vec2(val[0] - b.val[0], val[1] - b.val[1]);
	}
	Vec2 Vec2::operator*(const Vec<float,2>& b) const {
		return Vec2(val[0] * b.val[0], val[1] * b.val[1]);
	}
	Vec2 Vec2::operator/(const Vec<float,2>& b) const {
		return Vec2(val[0] / b.val[0], val[1] / b.val[1]);
	}

	Vec2 Vec2::operator+(float b) const {
		return Vec2(val[0] + b, val[1] + b);
	}
	Vec2 Vec2::operator-(float b) const {
		return Vec2(val[0] - b, val[1] - b);
	}
	Vec2 Vec2::operator*(float b) const {
		return Vec2(val[0] * b, val[1] * b);
	}
	Vec2 Vec2::operator/(float b) const {
		return Vec2(val[0] / b, val[1] / b);
	}

	Vec2 Vec2::operator-() const {
		return Vec2(-val[0], -val[1]);
	}

	Vec2& Vec2::operator+=(const Vec<float,2>& b) {
		val[0] += b.val[0]; val[1] += b.val[1]; return *this;
	}
	Vec2& Vec2::operator-=(const Vec<float,2>& b) {
		val[0] -= b.val[0]; val[1] -= b.val[1]; return *this;
	}
	Vec2& Vec2::operator*=(const Vec<float,2>& b) {
		val[0] *= b.val[0]; val[1] *= b.val[1]; return *this;
	}
	Vec2& Vec2::operator*=(float b) {
		val[0] *= b; val[1] *= b; return *this;
	}
	Vec2& Vec2::operator/=(const Vec<float,2>& b) {
		val[0] /= b.val[0]; val[1] /= b.val[1]; return *this;
	}

	bool  Vec2::operator==(const Vec<float,2>& b) const {
		return *reinterpret_cast<const double*>(val) == *reinterpret_cast<const double*>(b.val);
	}
	bool  Vec2::operator!=(const Vec<float,2>& b) const {
		return *reinterpret_cast<const double*>(val) != *reinterpret_cast<const double*>(b.val);
	}

	float Vec2::dot(const Vec<float,2> b) const {
		return val[0] * b.val[0] + val[1] * b.val[1];
	}

	Vec3 Vec2::det(const Vec<float,2> b,const Vec<float,2> c) const {
		return Vec3(val[0], b.val[0], c.val[0]).det(Vec3(val[1], b.val[1], c.val[1]));
	}

	float Vec2::length() const {
		if (val[0] == 0)
			return abs(val[1]);
		else if (val[1] == 0)
			return abs(val[0]);
		else
			return sqrtf( val[0] * val[0] + val[1] * val[1] );
	}

	Vec2 Vec2::rotate90z() const {
		return Vec2{val[1], -val[0]};
	}

	Vec2 Vec2::rotate270z() const { // ccw rotate 90
		return Vec2{-val[1], val[0]};
	}

	Vec2 Vec2::normalized() const {
		if (val[0] == 0.0f) {
			if (val[1] == 0.0f) {
				return {0.0f,0.0f}; // invalid value
			}
			return {
				0.0f, val[1] < 0.0f ? -1.0f: 1.0f
			};
		} else if (val[1] == 0.0f) {
			return {
				val[0] < 0.0f ? -1.0f: 1.0f, 0.0f
			};
		}
		float len = sqrtf(val[0] * val[0] + val[1] * val[1]);
		return { val[0] / len, val[1] / len };
	}

	Vec2 Vec2::normalline(const Vec2 *prev, const Vec2 *next) const {
		if (!prev) {
			return Vec2(next->x() - x(), next->y() - y()).normalized().rotate90z();
		}
		if (!next) {
			return Vec2(x() - prev->x(), y() - prev->y()).normalized().rotate90z();
		}
		Vec2 toNext   = Vec2(next->x() - x(), next->y() - y()).normalized().rotate90z();
		Vec2 fromPrev = Vec2(x() - prev->x(), y() - prev->y()).normalized().rotate90z();

		// Returns zero when the previous is on the same side and on the same line as the next
		return (toNext + fromPrev).normalized();
	}

	float Vec2::angle() const {
		if (val[0] > 0) { // x > 0
			float a = atanf(val[1]/val[0]);
			if (a < 0)
				a += Qk_PI_2;
			return a;
		}
		else if (val[0] < 0) { // x < 0
			return atanf(val[1]/val[0]) + Qk_PI;
		}
		// x == 0
		if (val[1] > 0) { // y > 0
			return Qk_PI_2_1;
		} if (val[1] < 0) { // y < 0
			return Qk_PI_2_1 + Qk_PI;
		} else { // y equal 0 and x equal 0
			return 0;
		}
	}

	float Vec2::angleTo(Vec2 to) const {
		float a = angle() - to.angle();
		// if (a < 0)
			// a += Qk_PI_2;
		return a;
		// return acosf(dot(to) / (length() * to.length()));
	}

	// ------------------------------------------

	Vec3::Vec3(): Vec(0,0,0) {
	}
	Vec3::Vec3(float f): Vec(f,f,f) {
	}
	Vec3::Vec3(const Vec<float,3>& v): Vec(v) {
	}
	Vec3::Vec3(float a, float b, float c): Vec(a,b,c) {
	}
	Vec3::Vec3(const Vec<float, 2> &vec2, float f): Vec(vec2.val[0],vec2.val[1],f) {
	}
	Vec3::Vec3(float f, const Vec<float, 2> &vec2): Vec(f,vec2.val[0],vec2.val[1]) {
	}
	Vec3 Vec3::operator+(const Vec<float,3>& b) const {
		return Vec3(val[0] + b.val[0], val[1] + b.val[1], val[2] + b.val[2]);
	}
	Vec3 Vec3::operator-(const Vec<float,3>& b) const {
		return Vec3(val[0] - b.val[0], val[1] - b.val[1], val[2] - b.val[2]);
	}
	Vec3 Vec3::operator*(const Vec<float,3>& b) const {
		return Vec3(val[0] * b.val[0], val[1] * b.val[1], val[2] * b.val[2]);
	}
	Vec3 Vec3::operator/(const Vec<float,3>& b) const {
		return Vec3(val[0] / b.val[0], val[1] / b.val[1], val[2] / b.val[2]);
	}

	Vec3 Vec3::operator+(float b) const {
		return Vec3(val[0] + b, val[1] + b, val[2] + b);
	}
	Vec3 Vec3::operator-(float b) const {
		return Vec3(val[0] - b, val[1] - b, val[2] - b);
	}
	Vec3 Vec3::operator*(float b) const {
		return Vec3(val[0] * b, val[1] * b, val[2] * b);
	}
	Vec3 Vec3::operator/(float b) const {
		return Vec3(val[0] / b, val[1] / b, val[2] / b);
	}

	Vec3 Vec3::operator-() const {
		return Vec3(-val[0], -val[1], -val[2]);
	}

	Vec3& Vec3::operator+=(const Vec<float,3>& b) {
		val[0] += b.val[0]; val[1] += b.val[1]; val[2] += b.val[2]; return *this;
	}
	Vec3& Vec3::operator-=(const Vec<float,3>& b) {
		val[0] -= b.val[0]; val[1] -= b.val[1]; val[2] -= b.val[2]; return *this;
	}
	Vec3& Vec3::operator*=(const Vec<float,3>& b) {
		val[0] *= b.val[0]; val[1] *= b.val[1]; val[2] *= b.val[2]; return *this;
	}
	Vec3& Vec3::operator*=(float b) {
		val[0] *= b; val[1] *= b; val[2] *= b; return *this;
	}
	Vec3& Vec3::operator/=(const Vec<float,3>& b) {
		val[0] /= b.val[0]; val[1] /= b.val[1]; val[2] /= b.val[2]; return *this;
	}

	bool  Vec3::operator==(const Vec<float,3>& b) const {
		return *reinterpret_cast<const double*>(val) == *reinterpret_cast<const double*>(b.val) && val[2] == b.val[2];
	}
	bool  Vec3::operator!=(const Vec<float,3>& b) const {
		return *reinterpret_cast<const double*>(val) != *reinterpret_cast<const double*>(b.val) || val[2] != b.val[2];
	}

	float Vec3::length() const {
		return sqrtf( val[0] * val[0] + val[1] * val[1] + val[2] * val[2] );
	}
	float Vec3::dot(const Vec<float,3>& b) const {
		// a->.b->=|a->||b->|cos(ab)
		return val[0] * b.val[0] + val[1] * b.val[1] + val[2] * b.val[2];
	}
	Vec3  Vec3::det(const Vec<float,3>& b) const {
		// a->xb->=|a->||b->|sin(ab)=|c->|
		return Vec3(
			y()*b.z()-b.y()*z(), // x
			z()*b.x()-b.z()*x(), // y
			x()*b.y()-b.x()*y()  // z
		);
	}

	// ------------------------------------------

	bool Color4f::operator==(const Color4f& color) const {
		return color.r() == r() && color.g() == g() &&  color.b() == b() &&  color.a() == a();
	}

	bool Color4f::operator!=(const Color4f& color) const {
		return ! operator==(color);
	}

	bool Color::operator==(Color color) const {
		return *reinterpret_cast<const int*>(&color) == *reinterpret_cast<const int*>(this);
	}

	bool Color::operator!=(Color color) const {
		return *reinterpret_cast<const int*>(&color) != *reinterpret_cast<const int*>(this);
	}

	Color4f Color::to_color4f() const {
		// create indexed table
		constexpr float scale = 1 / 255.0f;
		return Color4f(r() * scale,
									 g() * scale,
									 b() * scale,
									 a() * scale
								);
	}

	Color4f Color::to_color4f_alpha(float alpha) const {
		constexpr float scale = 1 / 255.0f;
		return Color4f(r() * scale,
									 g() * scale,
									 b() * scale,
									 a() * scale * alpha
								);
	}

	union {
		struct { char is_small_end,_[3]; }; int b;
	} constexpr test_small { .b=1 };

	Color Color::from(uint32_t color) { // ignore endianness
		return *reinterpret_cast<Color*>(&color);
	}

	uint32_t swap_bit_form_uint32_t(uint32_t i) {
		return
			((i >> 24)) |
			((i >> 8)  & 0x0000ff00) |
			((i << 8)  & 0x00ff0000) |
			((i << 24));
	}

	Color Color::from_rgba(uint32_t rgba) { // high => low as r,g,b,a
		if (test_small.is_small_end) {
			rgba = swap_bit_form_uint32_t(rgba);
		}
		return *reinterpret_cast<Color*>(&rgba);
	}

	Color Color::from_abgr(uint32_t abgr) { // high => low as a,b,g,r
		if (!test_small.is_small_end) {
			abgr = swap_bit_form_uint32_t(abgr);
		}
		return *reinterpret_cast<Color*>(&abgr);
	}

	uint32_t Color::to_uint32() const {// small end data as a,b,g,r
		return *reinterpret_cast<const uint32_t*>(this);
	}

	uint32_t Color::to_uint32_rgba() const {
		uint32_t rbga = *reinterpret_cast<const uint32_t*>(this);
		if (test_small.is_small_end) {
			rbga = swap_bit_form_uint32_t(rbga);
		}
		return rbga;
	}

	uint32_t Color::to_uint32_abgr() const {
		uint32_t abgr = *reinterpret_cast<const uint32_t*>(this);
		if (!test_small.is_small_end) {
			abgr = swap_bit_form_uint32_t(abgr);
		}
		return abgr;
	}

	Mat::Mat(float value) {
		val[0] = value;
		val[1] = 0;
		val[2] = 0;
		val[3] = 0;
		val[4] = value;
		val[5] = 0;
	}

	Mat::Mat(float m0, float m1, float m2, float m3, float m4, float m5) {
		val[0] = m0;
		val[1] = m1;
		val[2] = m2;
		val[3] = m3;
		val[4] = m4;
		val[5] = m5;
	}

	Mat::Mat(const float* values, int length) {
		memcpy(val, values, sizeof(float) * length);
	}

	Mat::Mat(Vec2 translate, Vec2 scale, float rotate, Vec2 skew) {
		if (rotate) {
			float cz  = cosf(rotate);
			float sz  = sinf(rotate);
			val[0] = cz * scale[0];
			val[1] = sz * scale[1];
			val[3] = -sz * scale[0];
			val[4] = cz * scale[1];
		}
		else {
			val[0] = scale[0];
			val[1] = 0;
			val[3] = 0;
			val[4] = scale[1];
		}

		val[2] = translate[0];
		val[5] = translate[1];

		if (skew[0] != 0.0f || skew[1] != 0.0f) {
			Mat::skew(skew);
		}
	}

	Mat& Mat::translate(Vec2 v) {
		/*
		[ a, b, c ]   [ 1, 0, x ]
		[ d, e, f ] * [ 0, 1, y ]
		[ 0, 0, 1 ]   [ 0, 0, 1 ]
		*/
		val[2] += val[0] * v.val[0] + val[1] * v.val[1];
		val[5] += val[3] * v.val[0] + val[4] * v.val[1];
		return *this;
	}

	Mat& Mat::translate_x(float x) {
		/*
		[ a, b, c ]   [ 1, 0, x ]
		[ e, f, g ] * [ 0, 1, 0 ]
		[ 0, 0, 1 ]   [ 0, 0, 1 ]
		*/
		val[2] += val[0] * x;
		val[5] += val[3] * x;
		return *this;
	}

	Mat& Mat::translate_y(float y) {
		/*
		[ a, b, c ]   [ 1, 0, 0 ]
		[ e, f, g ] * [ 0, 1, y ]
		[ 0, 0, 1 ]   [ 0, 0, 1 ]
		*/
		val[2] += val[1] * y;
		val[5] += val[4] * y;
		return *this;
	}

	Mat& Mat::scale(Vec2 v) {
		/*
		[ a, b, c ]   [ x, 0, 0 ]
		[ d, e, f ] * [ 0, y, 0 ]
		[ 0, 0, 1 ]   [ 0, 0, 1 ]
		*/
		val[0] *= v.val[0];
		val[3] *= v.val[0];
		val[1] *= v.val[1];
		val[4] *= v.val[1];
		return *this;
	}

	Mat& Mat::scale_x(float x) {
		/*
		[ a, b, c ]   [ x, 0, 0 ]
		[ d, e, f ] * [ 0, 1, 0 ]
		[ 0, 0, 1 ]   [ 0, 0, 1 ]
		*/
		val[0] *= x;
		val[3] *= x;
		return *this;
	}

	Mat& Mat::scale_y(float y) {
		/*
		[ a, b, c ]   [ 1, 0, 0 ]
		[ d, e, f ] * [ 0, y, 0 ]
		[ 0, 0, 1 ]   [ 0, 0, 1 ]
		*/
		val[1] *= y;
		val[4] *= y;
		return *this;
	}

	Mat& Mat::rotate(float z) {
		/*
		[ a, b, c ]   [ cos(z),  sin(z), 0 ]
		[ d, e, f ]   [-sin(z),  cos(z), 0 ]
		[ 0, 0, 1 ]   [ 0,       0,      1 ]
		*/
		float cz  = cosf(z);
		float sz  = sinf(z);
		float a   = val[0] * cz - val[1] * sz;
		float d   = val[3] * cz - val[4] * sz;
		val[1] = val[0] * sz + val[1] * cz;
		val[4] = val[3] * sz + val[4] * cz;
		val[0] = a;
		val[3] = d;
		return *this;
	}

	Mat& Mat::skew(Vec2 v){
		/*
		| a, b, c |   |  1,       tan(x),  0 |
		| d, e, f | * |  tan(y),  1,       0 |
		| 0, 0, 1 |   |  0,       0,       1 |
		*/
		float tx  = tanf(v.val[0]);
		float ty  = tanf(v.val[1]);
		float a   = val[0] + val[1] * ty;
		float d   = val[3] + val[4] * ty;
		val[1] += val[0] * tx;
		val[4] += val[3] * tx;
		val[0] = a;
		val[3] = d;
		return *this;
	}

	Mat& Mat::skew_x(float x){
		/*
		| a, b, c |   |  1,  tan(x),  0 |
		| d, e, f | * |  0,  1,       0 |
		| 0, 0, 1 |   |  0,  0,       1 |
		*/
		float tx = tanf(x);
		val[1] += val[0] * tx;
		val[4] += val[3] * tx;
		return *this;
	}

	Mat& Mat::skew_y(float y){
		/*
		| a, b, c |   |  1,       0,  0 |
		| d, e, f | * |  tan(y),  1,  0 |
		| 0, 0, 1 |   |  0,       0,  1 |
		*/
		float ty = tanf(y);
		val[0] += val[1] * ty;
		val[3] += val[4] * ty;
		return *this;
	}

	Mat Mat::operator*(const Mat& b) const {
		Mat output;
		mul(b, output);
		return output;
	}

	Mat& Mat::operator*=(const Mat& b) {
		memcpy(val, operator*(b).val, sizeof(float) * 6);
		return *this;
	}

	Vec2 Mat::operator*(const Vec2& b) const {
		/*
		[ a, b, c ]   [ a ]
		[ d, e, f ] * [ b ]
		[ 0, 0, 1 ]   [ 1 ]
		*/
		const float* _a = val;
		const float* _b = b.val;
		return Vec2(
			_a[0] * _b[0] + _a[1] * _b[1] + _a[2],
			_a[3] * _b[0] + _a[4] * _b[1] + _a[5]
		);
	}

	/**
	* @method mul # 矩阵乘法
	* @arg b {const Mat&}
	* @arg output {Mat&}
	*/
	void Mat::mul(const Mat& b, Mat& output) const {
		/*
		[ a1, b1, c1 ]   [ a2, b2, c2 ]
		[ d1, e1, f1 ] * [ d2, e2, f2 ]
		[ 0,  0,  1  ]   [ 0,  0,  1  ]
		*/
		// 矩阵
		float* _v = output.val;
		const float* _a = val;
		const float* _b = b.val;
		// 1 row
		_v[0] = _a[0]*_b[0] + _a[1]*_b[3];
		_v[1] = _a[0]*_b[1] + _a[1]*_b[4];
		_v[2] = _a[0]*_b[2] + _a[1]*_b[5] + _a[2];
		// 2 row
		_v[3] = _a[3]*_b[0] + _a[4]*_b[3];
		_v[4] = _a[3]*_b[1] + _a[4]*_b[4];
		_v[5] = _a[3]*_b[2] + _a[4]*_b[5] + _a[5];
	}

	Mat4::Mat4(float value) {
		constexpr float src = 0;
		memset_pattern4(val, &src, 16);
		val[0] = value;
		val[5] = value;
		val[10] = value;
		val[15] = value;
	}

	Mat4::Mat4(float m0, float m1, float m2, float m3,
						float m4, float m5, float m6, float m7,
						float m8, float m9, float m10, float m11,
						float m12, float m13, float m14, float m15) {
		val[0] = m0;
		val[1] = m1;
		val[2] = m2;
		val[3] = m3;
		//
		val[4] = m4;
		val[5] = m5;
		val[6] = m6;
		val[7] = m7;
		//
		val[8] = m8;
		val[9] = m9;
		val[10] = m10;
		val[11] = m11;
		//
		val[12] = m12;
		val[13] = m13;
		val[14] = m14;
		val[15] = m15;
	}

	Mat4::Mat4(const float* values, int length) {
		memcpy(val, values, sizeof(float) * length);
	}

	Mat4::Mat4(Mat mat) {
		val[0] = mat[0];
		val[1] = mat[1];
		val[2] = 0;
		val[3] = mat[2];
		//
		val[4] = mat[3];
		val[5] = mat[4];
		val[6] = 0;
		val[7] = mat[5];
		//
		val[8] = 0;
		val[9] = 0;
		val[10] = 1;
		val[11] = 0;
		//
		val[12] = 0;
		val[13] = 0;
		val[14] = 0;
		val[15] = 1;
	}

	Mat4& Mat4::translate(Vec3 v) {
		/*
		[ a, b, c, d ]   [ 1, 0, 0, x ]
		[ e, f, g, h ]   [ 0, 1, 0, y ]
		[ i, j, k, l ] * [ 0, 0, 1, z ]
		[ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
		*/
		val[3] += val[0] * v.x() + val[1] * v.y() + val[2] * v.z();
		val[7] += val[4] * v.x() + val[5] * v.y() + val[6] * v.z();
		val[11] += val[8] * v.x() + val[9] * v.y() + val[10] * v.z();
		return *this;
	}

	Mat4& Mat4::translate_x(float x) {
		/*
		[ a, b, c, d ]   [ 1, 0, 0, x ]
		[ e, f, g, h ]   [ 0, 1, 0, 0 ]
		[ i, j, k, l ] * [ 0, 0, 1, 0 ]
		[ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
		*/
		val[3] += val[0] * x;
		val[7] += val[4] * x;
		val[11] += val[8] * x;
		return *this;
	}

	Mat4& Mat4::translate_y(float y) {
		/*
		[ a, b, c, d ]   [ 1, 0, 0, 0 ]
		[ e, f, g, h ]   [ 0, 1, 0, y ]
		[ i, j, k, l ] * [ 0, 0, 1, 0 ]
		[ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
		*/
		val[3] += val[1] * y;
		val[7] += val[5] * y;
		val[11] += val[9] * y;
		return *this;
	}

	Mat4& Mat4::translate_z(float z) {
		/*
		[ a, b, c, d ]   [ 1, 0, 0, 0 ]
		[ e, f, g, h ]   [ 0, 1, 0, 0 ]
		[ i, j, k, l ] * [ 0, 0, 1, z ]
		[ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
		*/
		val[3] += val[2] * z;
		val[7] += val[6] * z;
		val[11] += val[10] * z;
		return *this;
	}

	Mat4& Mat4::scale(Vec3 v) {
		/*
		[ a, b, c, d ]   [ x, 0, 0, 0 ]
		[ e, f, g, h ]   [ 0, y, 0, 0 ]
		[ i, j, k, l ] * [ 0, 0, z, 0 ]
		[ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
		*/
		val[0] *= v.x();
		val[4] *= v.x();
		val[8] *= v.x();
		val[1] *= v.y();
		val[5] *= v.y();
		val[9] *= v.y();
		val[2] *= v.z();
		val[6] *= v.z();
		val[10] *= v.z();
		return *this;
	}

	Mat4& Mat4::scale_x(float x) {
		/*
		[ a, b, c, d ]   [ x, 0, 0, 0 ]
		[ e, f, g, h ]   [ 0, 1, 0, 0 ]
		[ i, j, k, l ] * [ 0, 0, 1, 0 ]
		[ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
		*/
		val[0] *= x;
		val[4] *= x;
		val[8] *= x;
	}

	Mat4& Mat4::scale_y(float y) {
		/*
		[ a, b, c, d ]   [ 1, 0, 0, 0 ]
		[ e, f, g, h ]   [ 0, y, 0, 0 ]
		[ i, j, k, l ] * [ 0, 0, 1, 0 ]
		[ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
		*/
		val[1] *= y;
		val[5] *= y;
		val[9] *= y;
	}

	Mat4& Mat4::scale_z(float z) {
		/*
		[ a, b, c, d ]   [ 1, 0, 0, 0 ]
		[ e, f, g, h ]   [ 0, 1, 0, 0 ]
		[ i, j, k, m ] * [ 0, 0, z, 0 ]
		[ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
		*/
		val[2] *= z;
		val[6] *= z;
		val[10] *= z;
	}

	Mat4& Mat4::rotate(Vec3 v) {
		
		// ZXY
		/*
		[ a, b, c, d ]   [ cycz + sxsysz,   czsxsy - cysz,   cxsy,  0 ]
		[ e, f, g, h ]   [ cxsz,            cxcz,           -sx,    0 ]
		[ i, j, k, l ] * [ cysxsz - czsy,   cyczsx + sysz,   cxcy,  0 ]
		[ 0, 0, 0, 1 ]   [ 0,               0,               0,     1 ]
		*/

		// ZYX
		/*
		| a, b, c, d |   |  cycz,  -cxsz + sxsycz,   sxsz + cxsycz,  0 |
		| e, f, g, h |   |  cysz,   cxcz + sxsysz,  -sxcz + cxsysz,  0 |
		| i, j, k, l | * | -sy,     sxcy,            cxcy,           0 |
		| 0, 0, 0, 1 |   |  0,      0,               0,              1 |
		*/

		float cx = cosf(v.x());
		float sx = sinf(v.x());
		float cy = cosf(-v.y());
		float sy = sinf(-v.y());
		float cz = cosf(v.z());
		float sz = sinf(v.z());

		// use ZXY
		float m0 = cy * cz + sx * sy * sz;
		float m4 = cz * sx * sy - cy * sz;
		float m8 = cx * sy;
		
		float m1 = cx * sz;
		float m5 = cx * cz;
		float m9 = -sx;
		
		float m2 = cy * sx * sz - cz * sy;
		float m6 = cy * cz * sx + sy * sz;
		float m10 = cx * cy;
		
		float a = val[0] * m0 + val[1] * m4 + val[2] * m8;
		float e = val[4] * m0 + val[5] * m4 + val[6] * m8;
		float i = val[8] * m0 + val[9] * m4 + val[10] * m8;

		float b = val[0] * m1 + val[1] * m5 + val[2] * m9;
		float f = val[4] * m1 + val[5] * m5 + val[6] * m9;
		float j = val[8] * m1 + val[9] * m5 + val[10] * m9;

		float c = val[0] * m2 + val[1] * m6 + val[2] * m10;
		float g = val[4] * m2 + val[5] * m6 + val[6] * m10;
		float k = val[8] * m2 + val[9] * m6 + val[10] * m10;
		
		val[0] = a;
		val[4] = e;
		val[8] = i;
		val[1] = b;
		val[5] = f;
		val[9] = j;
		val[2] = c;
		val[6] = g;
		val[10] = k;
		return *this;
	}

	Mat4& Mat4::rotate_x(float x) {
		/*
		[ a, b, c, d ]   [ 1,  0,      0,      0 ]
		[ e, f, g, h ]   [ 0,  cos(x), sin(x), 0 ]
		[ i, j, k, l ] * [ 0, -sin(x)  cos(x), 0 ]
		[ 0, 0, 0, 1 ]   [ 0,  0,      0,      1 ]
		*/
		
		float cx = cosf(x);
		float sx = sinf(x);
		//
		float b = val[1] * cx - val[2] * sx;
		float f = val[5] * cx - val[6] * sx;
		float j = val[9] * cx - val[10] * sx;
		//
		val[2] = val[1] * sx + val[2] * cx;
		val[6] = val[5] * sx + val[6] * cx;
		val[10] = val[9] * sx + val[10] * cx;
		//
		val[1] = b;
		val[5] = f;
		val[9] = j;
		return *this;
	}

	Mat4& Mat4::rotate_y(float y) {
		/*
		[ a, b, c, d ]   [ cos(y), 0, -sin(y), 0 ]
		[ e, f, g, h ]   [ 0,      1, 0,       0 ]
		[ i, j, k, l ] * [ sin(y), 0  cos(y),  0 ]
		[ 0, 0, 0, 1 ]   [ 0,      0, 0,       1 ]
		*/
		float cy = cosf(y);
		float sy = sinf(y);
		//
		float a = val[0] * cy + val[2] * sy;
		float e = val[4] * cy + val[6] * sy;
		float i = val[8] * cy + val[10] * sy;
		//
		val[2] = -val[0] * sy + val[2] * cy;
		val[6] = -val[4] * sy + val[6] * cy;
		val[10] = -val[8] * sy + val[10] * cy;
		//
		val[0] = a;
		val[4] = e;
		val[8] = i;
		return *this;
	}

	Mat4& Mat4::rotate_z(float z) {
		/*
		[ a, b, c, d ]   [ cos(z),  sin(z), 0, 0 ]
		[ e, f, g, h ]   [-sin(z),  cos(z), 0, 0 ]
		[ i, j, k, l ] * [ 0,       0       1, 0 ]
		[ 0, 0, 0, 1 ]   [ 0,       0,      0, 1 ]
		*/
		float cz = cosf(z);
		float sz = sinf(z);
		//
		float a = val[0] * cz - val[1] * sz;
		float e = val[4] * cz - val[5] * sz;
		float i = val[8] * cz - val[9] * sz;
		//
		val[1] = val[0] * sz + val[1] * cz;
		val[5] = val[4] * sz + val[5] * cz;
		val[9] = val[8] * sz + val[9] * cz;
		//
		val[0] = a;
		val[4] = e;
		val[8] = i;
		return *this;
	}

	Mat4& Mat4::skew(Vec3 v) {
		
		float tx = tanf(v.x());
		float ty = tanf(v.y());
		float tz = tanf(v.z());
		
		/*
		| a, b, c, d |   |  1,       tan(x),   tan(x),  0 |
		| e, f, g, h |   |  tan(y),  1,        tan(y),  0 |
		| i, j, k, l | * |  tan(z),  tan(z),   1,       0 |
		| 0, 0, 0, 1 |   |  0,       0,        0,       1 |
		*/
		
		float a = val[0] + val[1] * ty + val[2] * tz;
		float e = val[4] + val[5] * ty + val[6] * tz;
		float i = val[8] + val[9] * ty + val[10] * tz;

		float b = val[0] * tx + val[1] + val[2] * tz;
		float f = val[4] * tx + val[5] + val[6] * tz;
		float j = val[8] * tx + val[9] + val[10] * tz;

		float c = val[0] * tx + val[1] * ty + val[2];
		float g = val[4] * tx + val[5] * ty + val[6];
		float k = val[8] * tx + val[9] * ty + val[10];
		
		val[0] = a;
		val[4] = e;
		val[8] = i;
		val[1] = b;
		val[5] = f;
		val[9] = j;
		val[2] = c;
		val[6] = g;
		val[10] = k;

		return *this;
	}

	Mat4& Mat4::skew_x(float x) {
		
		/*
		| a, b, c, d |   |  1,  tan(x),   tan(x),  0 |
		| e, f, g, h |   |  0,  1,        0,       0 |
		| i, j, k, l | * |  0,  0,        1,       0 |
		| 0, 0, 0, 1 |   |  0,  0,        0,       1 |
		*/
		
		float tx = tanf(x);
		
		val[1] += val[0] * tx;
		val[5] += val[4] * tx;
		val[9] += val[8] * tx;
		
		val[2] += val[0] * tx;
		val[6] += val[4] * tx;
		val[10] += val[8] * tx;

		return *this;
	}

	Mat4& Mat4::skew_y(float y) {
		/*
		| a, b, c, d |   |  1,       0,        0,            0 |
		| e, f, g, h |   |  tan(y),  1,        tan(y),       0 |
		| i, j, k, l | * |  0,       0,        1,            0 |
		| 0, 0, 0, 1 |   |  0,       0,        0,            1 |
		*/
		
		float ty = tanf(y);
		
		val[0] += val[1] * ty;
		val[4] += val[5] * ty;
		val[8] += val[9] * ty;
		
		val[2] += val[1] * ty;
		val[6] += val[5] * ty;
		val[10] += val[9] * ty;

		return *this;
	}

	Mat4& Mat4::skew_z(float z) {
		/*
		| a, b, c, d |   |  1,       0,              0,       0 |
		| e, f, g, h |   |  0,       1,              0,       0 |
		| i, j, k, l | * |  tan(z),  tan(z),         1,       0 |
		| 0, 0, 0, 1 |   |  0,       0,              0,       1 |
		*/
		
		float tz = tanf(z);
		
		val[0] += val[2] * tz;
		val[4] += val[6] * tz;
		val[8] += val[10] * tz;
		
		val[1] += val[2] * tz;
		val[5] += val[6] * tz;
		val[9] += val[10] * tz;

		return *this;
	}

	Mat4 Mat4::operator*(const Mat4& b) const {
		Mat4 output;
		mul(b, output);
		return output;
	}

	Mat4& Mat4::operator*=(const Mat4& b) {
		memcpy(val, operator*(b).val, sizeof(float) * 16);
		return *this;
	}

	Vec4 Mat4::operator*(const Vec4& b) const {
		/*
		[ a, b, c, d ]   [ a ]
		[ e, f, g, h ]   [ b ]
		[ i, j, k, l ] * [ c ]
		[ 0, 0, 0, 1 ]   [ 1 ]
		*/
		const float* _a = val;
		const float* _b = b.val;
		return Vec4(
			_a[0] * _b[0] + _a[1] * _b[1] + _a[2] * _b[2] + _a[3] * _b[3],
			_a[4] * _b[0] + _a[5] * _b[1] + _a[6] * _b[2] + _a[7] * _b[3],
			_a[8] * _b[0] + _a[9] * _b[1] + _a[10] * _b[2] + _a[11] * _b[3],
			_a[12] * _b[0] + _a[13] * _b[1] + _a[14] * _b[2] + _a[15] * _b[3]
		);
	}

	/**
	 * @method mul # 矩阵乘法
	 * @arg b {const Mat4&} b
	 * @arg output {Mat4&} output
	*/
	void Mat4::mul(const Mat4& b, Mat4& output) const {
		/*
		[ a1, b1, c1, d1 ]   [ a2, b2, c2, d2 ]
		[ e1, f1, g1, h1 ]   [ e2, f2, g2, h2 ]
		[ i1, j1, k1, l1 ] * [ i2, j2, k2, l2 ]
		[ m1, n1, o1, p1 ]   [ m2, n2, o2, p2 ]
		*/
		float* _v = output.val;
		const float* _a = val;
		const float* _b = b.val;

		// 1 row
		_v[0] = _a[0]*_b[0] + _a[1]*_b[4] + _a[2]*_b[8]  + _a[3]*_b[12];
		_v[1] = _a[0]*_b[1] + _a[1]*_b[5] + _a[2]*_b[9]  + _a[3]*_b[13];
		_v[2] = _a[0]*_b[2] + _a[1]*_b[6] + _a[2]*_b[10] + _a[3]*_b[14];
		_v[3] = _a[0]*_b[3] + _a[1]*_b[7] + _a[2]*_b[11] + _a[3]*_b[15];
		// 2 row
		_v[4] = _a[4]*_b[0] + _a[5]*_b[4] + _a[6]*_b[8]  + _a[7]*_b[12];
		_v[5] = _a[4]*_b[1] + _a[5]*_b[5] + _a[6]*_b[9]  + _a[7]*_b[13];
		_v[6] = _a[4]*_b[2] + _a[5]*_b[6] + _a[6]*_b[10] + _a[7]*_b[14];
		_v[7] = _a[4]*_b[3] + _a[5]*_b[7] + _a[6]*_b[11] + _a[7]*_b[15];
		// 3 row
		_v[8] = _a[8]*_b[0]  + _a[9]*_b[4] + _a[10]*_b[8]  + _a[11]*_b[12];
		_v[9] = _a[8]*_b[1]  + _a[9]*_b[5] + _a[10]*_b[9]  + _a[11]*_b[13];
		_v[10] = _a[8]*_b[2] + _a[9]*_b[6] + _a[10]*_b[10] + _a[11]*_b[14];
		_v[11] = _a[8]*_b[3] + _a[9]*_b[7] + _a[10]*_b[11] + _a[11]*_b[15];
		// 4 row
		_v[12] = _a[12]*_b[0] + _a[13]*_b[4] + _a[14]*_b[8]  + _a[15]*_b[12];
		_v[13] = _a[12]*_b[1] + _a[13]*_b[5] + _a[14]*_b[9]  + _a[15]*_b[13];
		_v[14] = _a[12]*_b[2] + _a[13]*_b[6] + _a[14]*_b[10] + _a[15]*_b[14];
		_v[15] = _a[12]*_b[3] + _a[13]*_b[7] + _a[14]*_b[11] + _a[15]*_b[15];
	}

	Mat4 Mat4::transpose() const {
		float mat4[16] = {
			val[0],val[4],val[8],val[12],
			val[1],val[5],val[9],val[13],
			val[2],val[6],val[10],val[14],
			val[3],val[7],val[11],val[15],
		};
		return *reinterpret_cast<Mat4*>(mat4);
	}

	Mat4 Mat4::frustum(float left, float right, float top, float bottom, float near, float far) {
		Mat4 matrix;
		float* _matrix = matrix.val;
		
		_matrix[0]  = (2.0f * near) / (right - left);
		_matrix[1]  = 0.0;
		_matrix[2]  = (right + left) / (right - left);
		_matrix[3]  = 0.0;
		
		_matrix[4]  = 0.0;
		_matrix[5]  = (2.0f * near) / (top - bottom);
		_matrix[6]  = (top + bottom) / (top - bottom);
		_matrix[7]  = 0.0;
		
		_matrix[8]  = 0.0;
		_matrix[9]  = 0.0;
		_matrix[10] = -(far + near) / (far - near);
		_matrix[11] = -(2.0f * far * near) / (far - near);
		
		_matrix[12] = 0.0;
		_matrix[13] = 0.0;
		_matrix[14] = -1.0f;
		_matrix[15] = 0.0;

		return matrix;
	};

	Mat4 Mat4::ortho(float left, float right, float top, float bottom, float near, float far) {
		Mat4 matrix;
		float* _matrix = matrix.val;
		
		_matrix[0]  = 2.0f / (right - left);
		_matrix[1]  = 0.0;
		_matrix[2]  = 0.0;
		_matrix[3]  = -(right + left) / (right - left);
		
		_matrix[4]  = 0.0;
		_matrix[5]  = 2.0f / (top - bottom);
		_matrix[6]  = 0.0;
		_matrix[7]  = -(top + bottom) / (top - bottom);
		
		_matrix[8]  = 0.0;
		_matrix[9]  = 0.0;
		_matrix[10] = 2.0f  / (far - near);
		_matrix[11] = -(far + near) / (far - near);
		
		_matrix[12] = 0.0;
		_matrix[13] = 0.0;
		_matrix[14] = 0.0;
		_matrix[15] = 1.0;

		return matrix;
	}

}
