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

#include <string.h>
#include <math.h>
#include "mathe.h"

FX_NS(ftr)

float Vec2::distance(Vec2 point) const {
	return sqrtf( powf(x() - point.x(), 2) + powf(y() - point.y(), 2) );
}

float Vec2::diagonal() const {
	return sqrtf( powf(width(), 2) + powf(height(), 2) );
}

bool FloatColor::operator==(const FloatColor& color) const {
	return color.r() == r() && color.g() == g() &&  color.b() == b() &&  color.a() == a();
}

static inline byte getPartColor(int color, int offset) {
	return (color >> offset) & 0xff;
}

Color::Color(uint color)
: MTColor<byte>(getPartColor(color, 24),
								getPartColor(color, 16),
								getPartColor(color, 8),
								getPartColor(color, 0))
{
	
}

bool Color::operator==(Color color) const {
	return *reinterpret_cast<const int*>(&color) == *reinterpret_cast<const int*>(this);
}

Mat::Mat(float value) {
	_value[0] = value;
	_value[1] = 0;
	_value[2] = 0;
	_value[3] = 0;
	_value[4] = value;
	_value[5] = 0;
}

Mat::Mat(float m0, float m1, float m2, float m3, float m4, float m5) {
	_value[0] = m0;
	_value[1] = m1;
	_value[2] = m2;
	_value[3] = m3;
	_value[4] = m4;
	_value[5] = m5;
}

Mat::Mat(const float* values, int length) {
	memcpy(_value, values, sizeof(float) * length);
}

Mat::Mat(Vec2 translate, Vec2 scale, float rotate_z, Vec2 skew) {
	if (rotate_z) {
		rotate_z  *= T_PI_RATIO_180;
		float cz  = cosf(rotate_z);
		float sz  = sinf(rotate_z);
		_value[0] = cz * scale[0];
		_value[1] = sz * scale[1];
		_value[2] = translate[0];
		_value[3] = -sz * scale[0];
		_value[4] = cz * scale[1];
		_value[5] = translate[1];
	}
	else {
		_value[0] = scale[0];
		_value[1] = 0;
		_value[2] = translate[0];
		_value[3] = 0;
		_value[4] = scale[1];
		_value[5] = translate[1];
	}
	if (skew[0] != 0.0f || skew[1] != 0.0f) {
		Mat::skew(skew[0], skew[1]);
	}
}

void Mat::translate(float x, float y) {
	/*
	 [ a, b, c ]   [ 1, 0, x ]
	 [ d, e, f ] * [ 0, 1, y ]
	 [ 0, 0, 1 ]   [ 0, 0, 1 ]
	 */
	_value[2] += _value[0] * x + _value[1] * y;
	_value[5] += _value[3] * x + _value[4] * y;
}

void Mat::translate_x(float x) {
	/*
	 [ a, b, c ]   [ 1, 0, x ]
	 [ e, f, g ] * [ 0, 1, 0 ]
	 [ 0, 0, 1 ]   [ 0, 0, 1 ]
	 */
	_value[2] += _value[0] * x;
	_value[5] += _value[3] * x;
}

void Mat::translate_y(float y) {
	/*
	 [ a, b, c ]   [ 1, 0, 0 ]
	 [ e, f, g ] * [ 0, 1, y ]
	 [ 0, 0, 1 ]   [ 0, 0, 1 ]
	 */
	_value[2] += _value[1] * y;
	_value[5] += _value[4] * y;
}

void Mat::scale(float x, float y) {
	/*
	 [ a, b, c ]   [ x, 0, 0 ]
	 [ d, e, f ] * [ 0, y, 0 ]
	 [ 0, 0, 1 ]   [ 0, 0, 1 ]
	 */
	_value[0] *= x;
	_value[3] *= x;
	_value[1] *= y;
	_value[4] *= y;
}

void Mat::scale_x(float x) {
	/*
	 [ a, b, c ]   [ x, 0, 0 ]
	 [ d, e, f ] * [ 0, 1, 0 ]
	 [ 0, 0, 1 ]   [ 0, 0, 1 ]
	 */
	_value[0] *= x;
	_value[3] *= x;
}

void Mat::scale_y(float y) {
	/*
	 [ a, b, c ]   [ 1, 0, 0 ]
	 [ d, e, f ] * [ 0, y, 0 ]
	 [ 0, 0, 1 ]   [ 0, 0, 1 ]
	 */
	_value[1] *= y;
	_value[4] *= y;
}

void Mat::rotatea(float z) {
	/*
	 [ a, b, c ]   [ cos(z),  sin(z), 0 ]
	 [ d, e, f ]   [-sin(z),  cos(z), 0 ]
	 [ 0, 0, 1 ]   [ 0,       0,      1 ]
	 */
	float cz  = cosf(z);
	float sz  = sinf(z);
	float a   = _value[0] * cz - _value[1] * sz;
	float d   = _value[3] * cz - _value[4] * sz;
	_value[1] = _value[0] * sz + _value[1] * cz;
	_value[4] = _value[3] * sz + _value[4] * cz;
	_value[0] = a;
	_value[3] = d;
}

void Mat::skewa(float x, float y){
	/*
	 | a, b, c |   |  1,       tan(x),  0 |
	 | d, e, f | * |  tan(y),  1,       0 |
	 | 0, 0, 1 |   |  0,       0,       1 |
	 */
	float tx  = tanf(x);
	float ty  = tanf(y);
	float a   = _value[0] + _value[1] * ty;
	float d   = _value[3] + _value[4] * ty;
	_value[1] += _value[0] * tx;
	_value[4] += _value[3] * tx;
	_value[0] = a;
	_value[3] = d;
}

void Mat::skewa_x(float x){
	/*
	 | a, b, c |   |  1,  tan(x),  0 |
	 | d, e, f | * |  0,  1,       0 |
	 | 0, 0, 1 |   |  0,  0,       1 |
	 */
	float tx = tanf(x);
	_value[1] += _value[0] * tx;
	_value[4] += _value[3] * tx;
}

void Mat::skewa_y(float y){
	/*
	 | a, b, c |   |  1,       0,  0 |
	 | d, e, f | * |  tan(y),  1,  0 |
	 | 0, 0, 1 |   |  0,       0,  1 |
	 */
	float ty = tanf(y);
	_value[0] += _value[1] * ty;
	_value[3] += _value[4] * ty;
}

Mat Mat::operator*(const Mat& b) const {
	Mat output;
	multiplication(b, output);
	return output;
}

Mat& Mat::operator*=(const Mat& b) {
	memcpy(_value, operator*(b)._value, sizeof(float) * 6);
	return *this;
}

Vec2 Mat::operator*(const Vec2& b) const {
	/*
	 [ a, b, c ]   [ a ]
	 [ d, e, f ] * [ b ]
	 [ 0, 0, 1 ]   [ 1 ]
	 */
	Vec2 vec;
	float* _v = const_cast<float*>(vec.value());
	const float* _a = _value;
	const float* _b = b.value();
	_v[0] = _a[0] * _b[0] + _a[1] * _b[1] + _a[2];
	_v[1] = _a[3] * _b[0] + _a[4] * _b[1] + _a[5];
	return vec;
}

/**
 * @func multiplication # 矩阵乘法
 * @arg b {const Mat&}
 * @arg output {Mat&}
 */
void Mat::multiplication(const Mat& b, Mat& output) const {
	/*
	 [ a1, b1, c1 ]   [ a2, b2, c2 ]
	 [ d1, e1, f1 ] * [ d2, e2, f2 ]
	 [ 0,  0,  1  ]   [ 0,  0,  1  ]
	 */
	// 矩阵
	float* _v = output._value;
	const float* _a = _value;
	const float* _b = b._value;
	_v[0] = _a[0] * _b[0] + _a[1] * _b[3];
	_v[3] = _a[3] * _b[0] + _a[4] * _b[3];
	_v[1] = _a[0] * _b[1] + _a[1] * _b[4];
	_v[4] = _a[3] * _b[1] + _a[4] * _b[4];
	_v[2] = _a[0] * _b[2] + _a[1] * _b[5] + _a[2];
	_v[5] = _a[3] * _b[2] + _a[4] * _b[5] + _a[5];
}

Mat4::Mat4(float value) {
	memset(this->_value, 0, sizeof(float) * 16);
	_value[0] = value;
	_value[5] = value;
	_value[10] = value;
	_value[15] = value;
}

Mat4::Mat4(float m0, float m1, float m2, float m3,
					 float m4, float m5, float m6, float m7,
					 float m8, float m9, float m10, float m11,
					 float m12, float m13, float m14, float m15) {
	_value[0] = m0;
	_value[1] = m1;
	_value[2] = m2;
	_value[3] = m3;
	//
	_value[4] = m4;
	_value[5] = m5;
	_value[6] = m6;
	_value[7] = m7;
	//
	_value[8] = m8;
	_value[9] = m9;
	_value[10] = m10;
	_value[11] = m11;
	//
	_value[12] = m12;
	_value[13] = m13;
	_value[14] = m14;
	_value[15] = m15;
}

Mat4::Mat4(const float* values, int length) {
	memcpy(_value, values, sizeof(float) * length);
}

Mat4::Mat4(Mat mat) {
	memset(this->_value, 0, sizeof(float) * 16);
	_value[0] = mat[0];
	_value[1] = mat[1];
	_value[3] = mat[2];
	_value[4] = mat[3];
	_value[5] = mat[4];
	_value[7] = mat[5];
	_value[10] = 1;
	_value[15] = 1;
}

void Mat4::translate(float x, float y, float z) {
	/*
	[ a, b, c, d ]   [ 1, 0, 0, x ]
	[ e, f, g, h ]   [ 0, 1, 0, y ]
	[ i, j, k, l ] * [ 0, 0, 1, z ]
	[ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
	*/
	_value[3] += _value[0] * x + _value[1] * y + _value[2] * z;
	_value[7] += _value[4] * x + _value[5] * y + _value[6] * z;
	_value[11] += _value[8] * x + _value[9] * y + _value[10] * z;
}

void Mat4::translate_x(float x) {
	/*
	 [ a, b, c, d ]   [ 1, 0, 0, x ]
	 [ e, f, g, h ]   [ 0, 1, 0, 0 ]
	 [ i, j, k, l ] * [ 0, 0, 1, 0 ]
	 [ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
	 */
	_value[3] += _value[0] * x;
	_value[7] += _value[4] * x;
	_value[11] += _value[8] * x;
}

void Mat4::translate_y(float y) {
	/*
	 [ a, b, c, d ]   [ 1, 0, 0, 0 ]
	 [ e, f, g, h ]   [ 0, 1, 0, y ]
	 [ i, j, k, l ] * [ 0, 0, 1, 0 ]
	 [ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
	 */
	_value[3] += _value[1] * y;
	_value[7] += _value[5] * y;
	_value[11] += _value[9] * y;
}

void Mat4::translate_z(float z) {
	/*
	 [ a, b, c, d ]   [ 1, 0, 0, 0 ]
	 [ e, f, g, h ]   [ 0, 1, 0, 0 ]
	 [ i, j, k, l ] * [ 0, 0, 1, z ]
	 [ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
	 */
	_value[3] += _value[2] * z;
	_value[7] += _value[6] * z;
	_value[11] += _value[10] * z;
}

void Mat4::scale(float x, float y, float z) {
	/*
	 [ a, b, c, d ]   [ x, 0, 0, 0 ]
	 [ e, f, g, h ]   [ 0, y, 0, 0 ]
	 [ i, j, k, l ] * [ 0, 0, z, 0 ]
	 [ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
	 */
	_value[0] *= x;
	_value[4] *= x;
	_value[8] *= x;
	_value[1] *= y;
	_value[5] *= y;
	_value[9] *= y;
	_value[2] *= z;
	_value[6] *= z;
	_value[10] *= z;
}

void Mat4::scale_x(float x) {
	/*
	 [ a, b, c, d ]   [ x, 0, 0, 0 ]
	 [ e, f, g, h ]   [ 0, 1, 0, 0 ]
	 [ i, j, k, l ] * [ 0, 0, 1, 0 ]
	 [ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
	 */
	_value[0] *= x;
	_value[4] *= x;
	_value[8] *= x;
}

void Mat4::scale_y(float y) {
	/*
	 [ a, b, c, d ]   [ 1, 0, 0, 0 ]
	 [ e, f, g, h ]   [ 0, y, 0, 0 ]
	 [ i, j, k, l ] * [ 0, 0, 1, 0 ]
	 [ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
	 */
	_value[1] *= y;
	_value[5] *= y;
	_value[9] *= y;
}

void Mat4::scale_z(float z) {
	/*
	 [ a, b, c, d ]   [ 1, 0, 0, 0 ]
	 [ e, f, g, h ]   [ 0, 1, 0, 0 ]
	 [ i, j, k, m ] * [ 0, 0, z, 0 ]
	 [ 0, 0, 0, 1 ]   [ 0, 0, 0, 1 ]
	 */
	_value[2] *= z;
	_value[6] *= z;
	_value[10] *= z;
}

void Mat4::rotatea(float x, float y, float z) {
	
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

	float cx = cosf(x);
	float sx = sinf(x);
	float cy = cosf(-y);
	float sy = sinf(-y);
	float cz = cosf(z);
	float sz = sinf(z);

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
	
	float a = _value[0] * m0 + _value[1] * m4 + _value[2] * m8;
	float e = _value[4] * m0 + _value[5] * m4 + _value[6] * m8;
	float i = _value[8] * m0 + _value[9] * m4 + _value[10] * m8;

	float b = _value[0] * m1 + _value[1] * m5 + _value[2] * m9;
	float f = _value[4] * m1 + _value[5] * m5 + _value[6] * m9;
	float j = _value[8] * m1 + _value[9] * m5 + _value[10] * m9;

	float c = _value[0] * m2 + _value[1] * m6 + _value[2] * m10;
	float g = _value[4] * m2 + _value[5] * m6 + _value[6] * m10;
	float k = _value[8] * m2 + _value[9] * m6 + _value[10] * m10;
	
	_value[0] = a;
	_value[4] = e;
	_value[8] = i;
	_value[1] = b;
	_value[5] = f;
	_value[9] = j;
	_value[2] = c;
	_value[6] = g;
	_value[10] = k;
}

void Mat4::rotatea_x(float x) {
	/*
	 [ a, b, c, d ]   [ 1,  0,      0,      0 ]
	 [ e, f, g, h ]   [ 0,  cos(x), sin(x), 0 ]
	 [ i, j, k, l ] * [ 0, -sin(x)  cos(x), 0 ]
	 [ 0, 0, 0, 1 ]   [ 0,  0,      0,      1 ]
	 */
	
	float cx = cosf(x);
	float sx = sinf(x);
	// 
	float b = _value[1] * cx - _value[2] * sx;
	float f = _value[5] * cx - _value[6] * sx;
	float j = _value[9] * cx - _value[10] * sx;
	// 
	_value[2] = _value[1] * sx + _value[2] * cx;
	_value[6] = _value[5] * sx + _value[6] * cx;
	_value[10] = _value[9] * sx + _value[10] * cx;
	// 
	_value[1] = b;
	_value[5] = f;
	_value[9] = j;
}

void Mat4::rotatea_y(float y) {
	/*
	 [ a, b, c, d ]   [ cos(y), 0, -sin(y), 0 ]
	 [ e, f, g, h ]   [ 0,      1, 0,       0 ]
	 [ i, j, k, l ] * [ sin(y), 0  cos(y),  0 ]
	 [ 0, 0, 0, 1 ]   [ 0,      0, 0,       1 ]
	 */
	float cy = cosf(y);
	float sy = sinf(y);
	// 
	float a = _value[0] * cy + _value[2] * sy;
	float e = _value[4] * cy + _value[6] * sy;
	float i = _value[8] * cy + _value[10] * sy;
	// 
	_value[2] = -_value[0] * sy + _value[2] * cy;
	_value[6] = -_value[4] * sy + _value[6] * cy;
	_value[10] = -_value[8] * sy + _value[10] * cy;
	// 
	_value[0] = a;
	_value[4] = e;
	_value[8] = i;
}

void Mat4::rotatea_z(float z) {
	/*
	 [ a, b, c, d ]   [ cos(z),  sin(z), 0, 0 ]
	 [ e, f, g, h ]   [-sin(z),  cos(z), 0, 0 ]
	 [ i, j, k, l ] * [ 0,       0       1, 0 ]
	 [ 0, 0, 0, 1 ]   [ 0,       0,      0, 1 ]
	 */
	float cz = cosf(z);
	float sz = sinf(z);
	// 
	float a = _value[0] * cz - _value[1] * sz;
	float e = _value[4] * cz - _value[5] * sz;
	float i = _value[8] * cz - _value[9] * sz;
	// 
	_value[1] = _value[0] * sz + _value[1] * cz;
	_value[5] = _value[4] * sz + _value[5] * cz;
	_value[9] = _value[8] * sz + _value[9] * cz;
	// 
	_value[0] = a;
	_value[4] = e;
	_value[8] = i;
}

void Mat4::skewa(float x, float y, float z) {
	
	float tx = tanf(x);
	float ty = tanf(y);
	float tz = tanf(z);
	
	/*
	 | a, b, c, d |   |  1,       tan(x),   tan(x),  0 |
	 | e, f, g, h |   |  tan(y),  1,        tan(y),  0 |
	 | i, j, k, l | * |  tan(z),  tan(z),   1,       0 |
	 | 0, 0, 0, 1 |   |  0,       0,        0,       1 |
	 */
	
	float a = _value[0] + _value[1] * ty + _value[2] * tz;
	float e = _value[4] + _value[5] * ty + _value[6] * tz;
	float i = _value[8] + _value[9] * ty + _value[10] * tz;

	float b = _value[0] * tx + _value[1] + _value[2] * tz;
	float f = _value[4] * tx + _value[5] + _value[6] * tz;
	float j = _value[8] * tx + _value[9] + _value[10] * tz;

	float c = _value[0] * tx + _value[1] * ty + _value[2];
	float g = _value[4] * tx + _value[5] * ty + _value[6];
	float k = _value[8] * tx + _value[9] * ty + _value[10];
	
	_value[0] = a;
	_value[4] = e;
	_value[8] = i;
	_value[1] = b;
	_value[5] = f;
	_value[9] = j;
	_value[2] = c;
	_value[6] = g;
	_value[10] = k;
}

void Mat4::skewa_x(float x) {
	
	/*
	 | a, b, c, d |   |  1,  tan(x),   tan(x),  0 |
	 | e, f, g, h |   |  0,  1,        0,       0 |
	 | i, j, k, l | * |  0,  0,        1,       0 |
	 | 0, 0, 0, 1 |   |  0,  0,        0,       1 |
	 */
	
	float tx = tanf(x);
	
	_value[1] += _value[0] * tx;
	_value[5] += _value[4] * tx;
	_value[9] += _value[8] * tx;
	
	_value[2] += _value[0] * tx;
	_value[6] += _value[4] * tx;
	_value[10] += _value[8] * tx;
}

void Mat4::skewa_y(float y) {
	/*
	 | a, b, c, d |   |  1,       0,        0,            0 |
	 | e, f, g, h |   |  tan(y),  1,        tan(y),       0 |
	 | i, j, k, l | * |  0,       0,        1,            0 |
	 | 0, 0, 0, 1 |   |  0,       0,        0,            1 |
	 */
	
	float ty = tanf(y);
	
	_value[0] += _value[1] * ty;
	_value[4] += _value[5] * ty;
	_value[8] += _value[9] * ty;
	
	_value[2] += _value[1] * ty;
	_value[6] += _value[5] * ty;
	_value[10] += _value[9] * ty;
}

void Mat4::skewa_z(float z) {
	/*
	 | a, b, c, d |   |  1,       0,              0,       0 |
	 | e, f, g, h |   |  0,       1,              0,       0 |
	 | i, j, k, l | * |  tan(z),  tan(z),         1,       0 |
	 | 0, 0, 0, 1 |   |  0,       0,              0,       1 |
	 */
	
	float tz = tanf(z);
	
	_value[0] += _value[2] * tz;
	_value[4] += _value[6] * tz;
	_value[8] += _value[10] * tz;
	
	_value[1] += _value[2] * tz;
	_value[5] += _value[6] * tz;
	_value[9] += _value[10] * tz;
}

Mat4 Mat4::operator*(const Mat4& b) const {
	Mat4 output;
	multiplication(b, output);
	return output;
}

Mat4& Mat4::operator*=(const Mat4& b) {
	memcpy(_value, operator*(b)._value, sizeof(float) * 16);
	return *this;
}

Vec4 Mat4::operator*(const Vec4& b) const {
	
	Vec4 vec;
	
	float* _v = const_cast<float*>(vec.value());
	const float* _a = _value;
	const float* _b = b.value();
	/*
	 [ a, b, c, d ]   [ a ]
	 [ e, f, g, h ]   [ b ]
	 [ i, j, k, l ] * [ c ]
	 [ 0, 0, 0, 1 ]   [ 1 ]
	 */
	
	_v[0] = _a[0] * _b[0] + _a[1] * _b[1] + _a[2] * _b[2] + _a[3] * _b[3];
	_v[1] = _a[4] * _b[0] + _a[5] * _b[1] + _a[6] * _b[2] + _a[7] * _b[3];
	_v[2] = _a[8] * _b[0] + _a[9] * _b[1] + _a[10] * _b[2] + _a[11] * _b[3];
	_v[3] = _a[12] * _b[0] + _a[13] * _b[1] + _a[14] * _b[2] + _a[15] * _b[3];
	return vec;
}

/**
 * @func multiplication # 矩阵乘法
 * @arg b {const Mat4&} b
 * @arg output {Mat4&} output
 */
void Mat4::multiplication(const Mat4& b, Mat4& output) const {
	/*
	 [ a1, b1, c1, d1 ]   [ a2, b2, c2, d2, a2, b2 ]
	 [ e1, f1, g1, h1 ]   [ e2, f2, g2, h2, a2, b2 ]
	 [ i1, j1, k1, l1 ] * [ i2, j2, k2, l2, a2, b2 ]
	 [ m1, n1, o1, p1 ]   [ m2, n2, o2, p2, a2, b2 ]
	 [ m1, n1, o1, p1 ]
	 [ m1, n1, o1, p1 ]
	 */
	float* _v = output._value;
	const float* _a = _value;
	const float* _b = b._value;
	// 
	_v[0] = _a[0] * _b[0] + _a[1] * _b[4] + _a[2] * _b[8] + _a[3] * _b[12];
	_v[4] = _a[4] * _b[0] + _a[5] * _b[4] + _a[6] * _b[8] + _a[7] * _b[12];
	_v[8] = _a[8] * _b[0] + _a[9] * _b[4] + _a[10] * _b[8] + _a[11] * _b[12];
	_v[12] = _a[12] * _b[0] + _a[13] * _b[4] + _a[14] * _b[8] + _a[15] * _b[12];
	// 
	_v[1] = _a[0] * _b[1] + _a[1] * _b[5] + _a[2] * _b[9] + _a[3] * _b[13];
	_v[5] = _a[4] * _b[1] + _a[5] * _b[5] + _a[6] * _b[9] + _a[7] * _b[13];
	_v[9] = _a[8] * _b[1] + _a[9] * _b[5] + _a[10] * _b[9] + _a[11] * _b[13];
	_v[13] = _a[12] * _b[1] + _a[13] * _b[5] + _a[14] * _b[9] + _a[15] * _b[13];
	// 
	_v[2] = _a[0] * _b[2] + _a[1] * _b[6] + _a[2] * _b[10] + _a[3] * _b[14];
	_v[6] = _a[4] * _b[2] + _a[5] * _b[6] + _a[6] * _b[10] + _a[7] * _b[14];
	_v[10] = _a[8] * _b[2] + _a[9] * _b[6] + _a[10] * _b[10] + _a[11] * _b[14];
	_v[14] = _a[12] * _b[2] + _a[13] * _b[6] + _a[14] * _b[10] + _a[15] * _b[14];
	// 
	_v[3] = _a[0] * _b[3] + _a[1] * _b[7] + _a[2] * _b[11] + _a[3] * _b[15];
	_v[7] = _a[4] * _b[3] + _a[5] * _b[7] + _a[6] * _b[11] + _a[7] * _b[15];
	_v[11] = _a[8] * _b[3] + _a[9] * _b[7] + _a[10] * _b[11] + _a[11] * _b[15];
	_v[15] = _a[12] * _b[3] + _a[13] * _b[7] + _a[14] * _b[11] + _a[15] * _b[15];
}

void Mat4::transpose() {
	/*
	 [ a,b,c,d ]
	 [ e,f,g,h ]
	 [ i,j,k,l ]
	 [ m,n,o,p ]
	 */
	float
	tmp = _value[1];
	_value[1] = _value[4];
	_value[4] = tmp;
	// 
	tmp = _value[2];
	_value[2] = _value[8];
	_value[8] = tmp;
	// 
	tmp = _value[3];
	_value[3] = _value[12];
	_value[12] = tmp;
	// 
	tmp = _value[6];
	_value[6] = _value[9];
	_value[9] = tmp;
	// 
	tmp = _value[7];
	_value[7] = _value[13];
	_value[13] = tmp;
	// 
	tmp = _value[11];
	_value[11] = _value[14];
	_value[14] = tmp;
}

Mat4 Mat4::frustum(float left, float right, float top, float bottom, float near, float far) {
	Mat4 matrix;
	float* _matrix = matrix._value;
	
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
	float* _matrix = matrix._value;
	
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

FX_END
