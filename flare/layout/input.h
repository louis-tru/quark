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

#ifndef __flare__layout__input__
#define __flare__layout__input__

#include "./box.h"

namespace flare {

	class FX_EXPORT Input: public Box {
		FX_Define_View(Input);
		public:

		/**
		 * @enum KeyboardType
		 */
		enum KeyboardType: uint8_t {
			NORMAL = value::NORMAL,
			ASCII = value::ASCII,
			NUMBER = value::NUMBER,
			URL = value::URL,
			NUMBER_PAD = value::NUMBER_PAD,
			PHONE = value::PHONE,
			NAME_PHONE = value::NAME_PHONE,
			EMAIL = value::EMAIL,
			DECIMAL = value::DECIMAL,
			TWITTER = value::TWITTER,
			SEARCH = value::SEARCH,
			ASCII_NUMBER = value::ASCII_NUMBER,
		};

		/**
		* @enum KeyboardReturnType
		*/		
		enum KeyboardReturnType: uint8_t {
			NORMAL = value::NORMAL,
			GO = value::GO,
			JOIN = value::JOIN,
			NEXT = value::NEXT,
			ROUTE = value::ROUTE,
			SEARCH = value::SEARCH,
			SEND = value::SEND,
			DONE = value::DONE,
			EMERGENCY = value::EMERGENCY,
			CONTINUE = value::CONTINUE,
		};

		// TODO ...
		private:
		// TODO ...
	};

}

#endif