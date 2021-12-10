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

#ifndef __flare__image_source__
#define __flare__image_source__

#include "./util/util.h"
#include "./util/string.h"

namespace flare {

	class F_EXPORT ImageSource: public Reference {
		F_HIDDEN_ALL_COPY(ImageSource);
	 public:
		bool markAsTexture(); // mark as gpu texture
		virtual String id() const;
		virtual void load();
		// ImageSourcePool.shared().get('http://flare.cool/res/test.jpeg')
		// <Image src={new ImageSource('http://flare.cool/res/test.jpeg')} />
		// <Image
		//   src='http://flare.cool/res/test.jpeg'
		//   width={100} 
		//   height={100} margin={100} padding={100} 
		//   fill="#f00,rgba(0,0,0,1)" class="img1" style={{width:100, height: 100}}
		// />
	 private:
	};

	class F_EXPORT ImageSourcePool: public Object {
		F_HIDDEN_ALL_COPY(ImageSourcePool);
	 public:
	 private:
	};

}

#endif