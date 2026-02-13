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

#ifndef __quark__ui__clipboard__
#define __quark__ui__clipboard__

#include "../util/util.h"

namespace qk {
	/**
	 * @class Clipboard
	 * @brief System clipboard access (text-only).
	 *
	 * Provides minimal cross-platform access to the OS clipboard for plain text.
	 * All write operations are marshalled onto the system main thread internally.
	 *
	 * Notes:
	 * - Only UTF-8 text is supported for now.
	 * - Binary/RTF/image formats are intentionally not handled at this stage.
	 * - On iOS, reading the clipboard may trigger the system privacy notice
	 *   when the content originates from another app.
	 */
	Qk_EXPORT class Clipboard: public Object {
	public:
		/**
		 * @brief Read current text from the system clipboard.
		 * @return Clipboard text as UTF-8 string. Returns empty string if no text exists.
		 *
		 * This is a direct read from the system clipboard.
		 */
		String get_text();

		/**
		 * @brief Write text into the system clipboard.
		 * @param text UTF-8 text to store.
		 *
		 * This call is internally forwarded to the system main thread
		 * to ensure platform safety.
		 */
		void set_text(cString& text);

		/**
		 * @brief Check whether the clipboard currently contains text.
		 * @return true if text content is available.
		 */
		bool has_text();

		/**
		 * @brief Clear clipboard contents.
		 *
		 * Internally dispatched to the system main thread.
		 */
		void clear();
	};
}

#endif