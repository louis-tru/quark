/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __ftr__mathe__
#define __ftr__mathe__

#include "ftr/util/util.h"

/**
 * @ns ftr
 */

#define T_PI 3.1415926535898f                   // PI
#define T_PI_RATIO_180  0.017453292519943f      // PI / 180
#define T_180_RATIO_PI 57.29577951308232f       // 180 / PI

FX_NS(ftr)

template <typename T, int LEN>
class MTVec {
 public:
	inline MTVec() { }
	inline MTVec(T value) {
		for (int i = 0; i < LEN; i++) {
			_value[i] = value;
		}
	}
	inline T operator[](int index) const { return _value[index]; }
	inline T& operator[](int index) { return _value[index]; }
	inline const T* value() const { return _value; }
	inline T* value() { return _value; }
 protected:
	T _value[LEN];
};

template <typename T>
class MTColor: public MTVec<T, 4> {
 public:
	inline MTColor(T r, T g, T b, T a){
		this->_value[0] = r;
		this->_value[1] = g;
		this->_value[2] = b;
		this->_value[3] = a;
	}
	inline T r() const { return this->_value[0]; }
	inline T g() const { return this->_value[1]; }
	inline T b() const { return this->_value[2]; }
	inline T a() const { return this->_value[3]; }
	inline void r(T value) { this->_value[0] = value; }
	inline void g(T value) { this->_value[1] = value; }
	inline void b(T value) { this->_value[2] = value; }
	inline void a(T value) { this->_value[3] = value; }
};

template <int LEN>
class Vec: public MTVec<float, LEN> {
 public:
	inline Vec() { }
	inline Vec(float f): MTVec<float, LEN>(f) { }
};

class FX_EXPORT Vec2: public Vec<2> {
 public:
	inline Vec2(): Vec2(0) { }
	inline Vec2(float a, float b) {
		_value[0] = a;
		_value[1] = b;
	}
	inline Vec2(float f) {
		_value[0] = f;
		_value[1] = f;
	}
	inline float x() const { return _value[0]; }
	inline float y() const { return _value[1]; }
	inline void x(float value) { _value[0] = value; }
	inline void y(float value) { _value[1] = value; }
	inline float width() const { return _value[0]; }
	inline float height() const { return _value[1]; }
	inline void width(float value) { _value[0] = value; }
	inline void height(float value) { _value[1] = value; }
	inline bool operator==(const Vec2& b) const {
		return _value[0] == b._value[0] && _value[1] == b._value[1];
	}
	inline Vec2 operator-(const Vec2& b) const {
		return Vec2(_value[0] - b[0], _value[1] - b[1]);
	}
	inline Vec2 operator+(const Vec2& b) const {
		return Vec2(_value[0] + b[0], _value[1] + b[1]);
	}
	inline Vec2& operator-=(const Vec2& b) {
		_value[0] -= b[0]; _value[1] -= b[1]; return *this;
	}
	inline Vec2& operator+=(const Vec2& b) {
		_value[0] += b[0]; _value[1] += b[1]; return *this;
	}
	inline bool is_zero() const { return _value[0] == 0 || _value[1] == 0; }
	inline bool operator!=(const Vec2& b) const { return !operator==(b); }
	float distance(Vec2 point) const;
	float diagonal() const;
};

class FX_EXPORT Vec3: public Vec<3> {
 public:
	inline Vec3(): Vec3(0) { }
	inline Vec3(float a, float b, float c) {
		_value[0] = a;
		_value[1] = b;
		_value[2] = c;
	}
	inline Vec3(float f) {
		_value[0] = f;
		_value[1] = f;
		_value[2] = f;
	}
	inline float x() const { return _value[0]; }
	inline float y() const { return _value[1]; }
	inline float z() const { return _value[2]; }
	inline void x(float value) { _value[0] = value; }
	inline void y(float value) { _value[1] = value; }
	inline void z(float value) { _value[2] = value; }
	inline float width() const { return _value[0]; }
	inline float height() const { return _value[1]; }
	inline float depth() const { return _value[2]; }
	inline void width(float value) { _value[0] = value; }
	inline void height(float value) { _value[1] = value; }
	inline void depth(float value) { _value[2] = value; }
	inline bool operator==(const Vec3& b) const {
		return _value[0] == b._value[0] &&
					 _value[1] == b._value[1] && _value[2] == b._value[2];
	}
	inline bool operator!=(const Vec3& b) const { return !operator==(b); }
};

class FX_EXPORT Vec4: public Vec<4> {
 public:
	inline Vec4(): Vec4(0) { }
	inline Vec4(float a, float b, float c, float d) {
		_value[0] = a;
		_value[1] = b;
		_value[2] = c;
		_value[3] = d;
	}
	inline Vec4(float f) {
		_value[0] = f;
		_value[1] = f;
		_value[2] = f;
		_value[3] = f;
	}
	inline float x() const { return _value[0]; }
	inline float y() const { return _value[1]; }
	inline float z() const { return _value[2]; }
	inline float w() const { return _value[3]; }
	inline void x(float value) { _value[0] = value; }
	inline void y(float value) { _value[1] = value; }
	inline void z(float value) { _value[2] = value; }
	inline void w(float value) { _value[3] = value; }
	inline float width() const { return _value[0]; }
	inline float height() const { return _value[1]; }
	inline float depth() const { return _value[2]; }
	inline void width(float value) { _value[0] = value; }
	inline void height(float value) { _value[1] = value; }
	inline void depth(float value) { _value[2] = value; }
	inline bool operator==(const Vec4& b) const {
		return  _value[0] == b._value[0] && _value[1] == b._value[1] &&
						_value[2] == b._value[2] && _value[3] == b._value[3];
	}
	inline bool operator!=(const Vec4& b) const { return !operator==(b); }
};

/**
 * @class FloatColor
 */
class FX_EXPORT FloatColor: public MTColor<float> {
 public:
	inline FloatColor(): MTColor<float>(0, 0, 0, 1) { }
	inline FloatColor(float r, float g, float b): MTColor<float>(r, g, b, 1) { }
	inline FloatColor(float r, float g, float b, float a): MTColor<float>(r, g, b, a) { }
	bool operator==(const FloatColor& color) const;
	inline bool operator!=(const FloatColor& color) const { return ! operator==(color); }
};

/**
 * @class Color
 */
class FX_EXPORT Color: public MTColor<byte> {
 public:
	Color(uint color);
	inline Color(): MTColor<byte>(0, 0, 0, 255) { }
	inline Color(byte r, byte g, byte b): MTColor<byte>(r, g, b, 255) { }
	inline Color(byte r, byte g, byte b, byte a): MTColor<byte>(r, g, b, a) { }
	bool operator==(Color color) const;
	inline bool operator!=(Color color) const { return ! operator==(color); }
	inline FloatColor to_float_color() const {
		return FloatColor(r() / 255.0f, g() / 255.0f, b() / 255.0f, a() / 255.0f);
	}
};

/**
 * @class Mat
 */
class FX_EXPORT Mat: public Vec<6> {
 public:
	inline Mat(): Mat(1) { }
	Mat(float value);
	Mat(float m0, float m1, float m2, float m3, float m4, float m5);
	Mat(const float* values, int length = 6);
	Mat(Vec2 translate, Vec2 scale, float rotate_z, Vec2 skew);
	inline float m0() const { return _value[0]; }
	inline float m1() const { return _value[1]; }
	inline float m2() const { return _value[2]; }
	inline float m3() const { return _value[3]; }
	inline float m4() const { return _value[4]; }
	inline float m5() const { return _value[5]; }
	inline void m0(float value) { _value[0] = value; }
	inline void m1(float value) { _value[1] = value; }
	inline void m2(float value) { _value[2] = value; }
	inline void m3(float value) { _value[3] = value; }
	inline void m4(float value) { _value[4] = value; }
	inline void m5(float value) { _value[5] = value; }
	
	/**
	 * @func translate 平移
	 */
	void translate(float x, float y);
	void translate_x(float x);
	void translate_y(float y);
	
	/**
	 * @func scale 缩放
	 */
	void scale(float x, float y);
	void scale_x(float x);
	void scale_y(float y);
	
	/**
	 * @func rotate 通过角度旋转
	 */
	inline void rotate(float z) {
		rotatea(z * T_PI_RATIO_180);
	}
	
	/**
	 * @func rotatea 通过弧度旋转
	 */
	void rotatea(float rotate);
	
	/**
	 * @func skew 通过角度歪斜变形
	 */
	void skew(float x, float y) {
		skewa(x * T_PI_RATIO_180, y * T_PI_RATIO_180);
	}
	void skew_x(float x) { skewa_x(x * T_PI_RATIO_180); }
	void skew_y(float y) { skewa_y(y * T_PI_RATIO_180); }
	
	/**
	 * @func skewa 通过弧度歪斜变形
	 */
	void skewa(float x, float y);
	void skewa_x(float x);
	void skewa_y(float y);
	
	/**
	 * @func operator* 矩阵乘法,返回新的4*4矩阵
	 */
	Mat operator*(const Mat& b) const;
	
	/**
	 * @func operator*= 矩阵乘法,相乘后赋值给自己,并返回自引用
	 */
	Mat& operator*=(const Mat& b);
	
	/**
	 * @func operator* 与向量乘法,向量做为列向量使用
	 */
	Vec2 operator*(const Vec2& b) const;
	
	/**
	 * @func multiplication 矩阵乘法
	 */
	void multiplication(const Mat& b, Mat& output) const;
	
};

/**
 * @class Mat4
 */
class FX_EXPORT Mat4: public Vec<16> {
 public:
	inline Mat4(): Mat4(1) { }
	Mat4(float value);
	Mat4(float m0, float m1, float m2, float m3,
			 float m4, float m5, float m6, float m7,
			 float m8, float m9, float m10, float m11,
			 float m12, float m13, float m14, float m15);
	Mat4(const float* values, int length = 16);
	Mat4(Mat mat);
	inline float m0() const { return _value[0]; }
	inline float m1() const { return _value[1]; }
	inline float m2() const { return _value[2]; }
	inline float m3() const { return _value[3]; }
	inline float m4() const { return _value[4]; }
	inline float m5() const { return _value[5]; }
	inline float m6() const { return _value[6]; }
	inline float m7() const { return _value[7]; }
	inline float m8() const { return _value[8]; }
	inline float m9() const { return _value[9]; }
	inline float m10() const { return _value[10]; }
	inline float m11() const { return _value[11]; }
	inline float m12() const { return _value[12]; }
	inline float m13() const { return _value[13]; }
	inline float m14() const { return _value[14]; }
	inline float m15() const { return _value[15]; }
	inline void m0(float value) { _value[0] = value; }
	inline void m1(float value) { _value[1] = value; }
	inline void m2(float value) { _value[2] = value; }
	inline void m3(float value) { _value[3] = value; }
	inline void m4(float value) { _value[4] = value; }
	inline void m5(float value) { _value[5] = value; }
	inline void m6(float value) { _value[6] = value; }
	inline void m7(float value) { _value[7] = value; }
	inline void m8(float value) { _value[8] = value; }
	inline void m9(float value) { _value[9] = value; }
	inline void m10(float value) { _value[10] = value; }
	inline void m11(float value) { _value[11] = value; }
	inline void m12(float value) { _value[12] = value; }
	inline void m13(float value) { _value[13] = value; }
	inline void m14(float value) { _value[14] = value; }
	inline void m15(float value) { _value[15] = value; }
	
	/**
	 * @func translate 平移
	 */
	void translate(float x, float y, float z);
	void translate_x(float x);
	void translate_y(float y);
	void translate_z(float z);
	
	/**
	 * @func scale 缩放
	 */
	void scale(float x, float y, float z);
	void scale_x(float x);
	void scale_y(float y);
	void scale_z(float z);
	
	/**
	 * @func rotate 通过角度旋转
	 */
	inline void rotate(float x, float y, float z) {
		rotatea(x * T_PI_RATIO_180, y * T_PI_RATIO_180, z * T_PI_RATIO_180);
	}
	inline void rotate_x(float x) { rotatea_x(x * T_PI_RATIO_180); }
	inline void rotate_y(float y) { rotatea_y(y * T_PI_RATIO_180); }
	inline void rotate_z(float z) { rotatea_z(z * T_PI_RATIO_180); }
	
	/**
	 * @func rotatea 通过弧度旋转
	 */
	void rotatea(float x, float y, float z);
	void rotatea_x(float x);
	void rotatea_y(float y);
	void rotatea_z(float z);
	
	/**
	 * @func skew 通过角度歪斜变形
	 */
	void skew(float x, float y, float z) {
		skewa(x * T_PI_RATIO_180, y * T_PI_RATIO_180, z * T_PI_RATIO_180);
	}
	void skew_x(float x) { skewa_x(x * T_PI_RATIO_180); }
	void skew_y(float y) { skewa_y(y * T_PI_RATIO_180); }
	void skew_z(float z) { skewa_z(z * T_PI_RATIO_180); }
	
	/**
	 * @func skewa 通过弧度歪斜变形
	 */
	void skewa(float x, float y, float z);
	void skewa_x(float x);
	void skewa_y(float y);
	void skewa_z(float z);
	
	/**
	 * @func operator* 矩阵乘法,返回新的4*4矩阵
	 */
	Mat4 operator*(const Mat4& b) const;
	
	/**
	 * @func operator*= # 矩阵乘法,相乘后赋值给自己,并返回自引用
	 */
	Mat4& operator*=(const Mat4& b);
	
	/**
	 * @func operator* 与向量乘法,向量做为列向量使用
	 */
	Vec4 operator*(const Vec4& vec) const;
	
	/**
	 * 矩阵乘法
	 */
	void multiplication(const Mat4& b, Mat4& output) const;
	
	/**
	 * @func transpose 转置矩阵
	 */
	void transpose();
	
	/**
	 * @func frustum 创建一个透视平截头体矩阵
	 */
	static Mat4 frustum(float left, float right, float top, float bottom, float near, float far);
	
	/**
	 * @func ortho 创建一个正交投影矩阵
	 */
	static Mat4 ortho(float left, float right, float top, float bottom, float near, float far);
};

FX_END
#endif
