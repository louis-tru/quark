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

#ifndef __langou__pcm_player__
#define __langou__pcm_player__

#include "lutils/util.h"
#include "lutils/buffer.h"

/**
 * @ns langou
 */

XX_NS(langou)

/**
 * @class PCMPlayer
 */
class PCMPlayer: public Protocol {
 public:

	/**
	 * @func write
	 * */
	virtual bool write(cBuffer& buffer) = 0;

	/**
	 * @func flush
	 * */
	virtual void flush() = 0;

	/**
	 * @func set_mute
	 * */
	virtual bool set_mute(bool value) = 0;

	/**
	 * @func set_volume 0-100
	 * */
	virtual bool set_volume(uint value) = 0;

	/**
	 * @func buffer_size
	 * */
	virtual uint buffer_size() = 0;

	/**
	 * @func compensate
	 */
	virtual float compensate() = 0;

	/**
	 * @func create
	 */
	static PCMPlayer* create(uint channel_count, uint sample_rate);

};

XX_END
#endif
