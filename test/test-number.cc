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

#include <nxutils/util.h>
#include <nxutils/loop.h>
#include <nxutils/array.h>
#include <ngui/sys.h>

using namespace ngui;

template<class T>
void test_operation_add(const char* name) {
  
}

template<class T, int T2, class T3 = T>
void test_operation(const char* name) {
  
  T numbers[10];
  for ( int i = 0; i < 10; i++) {
    numbers[i] = i + 1.11101;
  }
  
  uint64 s1 = sys::time_monotonic();
  
  for ( int i = 0; i < 100000; i++ ) {
    T3 a = 0;
    if (T2 == 0) {
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
	a += numbers[0];
	a += numbers[1];
	a += numbers[2];
	a += numbers[3];
	a += numbers[4];
	a += numbers[5];
	a += numbers[6];
	a += numbers[7];
	a += numbers[8];
	a += numbers[9];
    } else if ( T2 == 1) {
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
	a *= numbers[0];
	a *= numbers[1];
	a *= numbers[2];
	a *= numbers[3];
	a *= numbers[4];
	a *= numbers[5];
	a *= numbers[6];
	a *= numbers[7];
	a *= numbers[8];
	a *= numbers[9];
    }
  }
  
  uint64 s2 = sys::time_monotonic();
  
  LOG("%s: %llu", name, s2 - s1);
}

void test_number(int argc, char **argv) {
  
  double d = -500;
  
  char c = d;
  byte b = d;
  
  LOG("%d", c);
  LOG("%d", *(char*)&b);
  
  test_operation<int16, 0>("int16+");
  test_operation<int, 0>("int+");
  test_operation<std::atomic_int, 0, int>("AtomicInt+");
  test_operation<int64, 0>("int64+");
  test_operation<float, 0>("float+");
  test_operation<double, 0>("double+");
  test_operation<int16, 1>("int16*");
  test_operation<int, 1>("int*");
  test_operation<int64, 1>("int64*");
  test_operation<float, 1>("float*");
  test_operation<double, 1>("double*");
}
