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

#include "./buffer.h"
#include "./array.cc.inl"

XX_NS(ngui)
 
XX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(char, BufferContainer);
XX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(byte, BufferContainer);
XX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(int16, BufferContainer);
XX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint16, BufferContainer);
XX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(int, BufferContainer);
XX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint, BufferContainer);
XX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(int64, BufferContainer);
XX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint64, BufferContainer);
XX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(float, BufferContainer);
XX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(double, BufferContainer);
XX_DEF_ARRAY_SPECIAL_IMPLEMENTATION(bool, BufferContainer);

XX_END
