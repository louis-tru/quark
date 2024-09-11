/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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
 * ***** END LICENSE BLOCK *****/

#include "./media_inl.h"

namespace qk {

	Extractor::Extractor(MediaType type, MediaSource* host, Array<TrackInfo>&& tracks)
		: _host(host)
		, _type(type)
		, _track_index(0)
		, _tracks(std::move(tracks))
		, _sample_data_cache()
		, _sample_index_cache(0)
		, _sample_count_cache(0)
		, _sample_data({ Buffer(), NULL, 0, 0, 0 })
		, _eof_flags(0)
		, _disable(1)
	{
	}

	/**
	* @method select_track
	*/
	bool Extractor::select_track(uint32_t index) {
		ScopeLock lock(_host->_inl->mutex());
		if ( _track_index != index && index < _tracks.length() ) {
			_host->_inl->extractor_flush(this);
			_track_index = index;
			return true;
		}
		return false;
	}

	/**
	* @method deplete_sample
	*/
	uint32_t Extractor::deplete_sample(Char* out, uint32_t size) {
		if ( _sample_data.size ) {
			size = Qk_MIN(_sample_data.size, size);
			memcpy(out, _sample_data.data, size);
			_sample_data.data += size;
			_sample_data.size -= size;
			return size;
		}
		return 0;
	}


	/**
	* @method deplete_sample
	*/
	uint32_t Extractor::deplete_sample(Buffer& out) {
		uint32_t size = out.write(_sample_data.data, 0, _sample_data.size);
		_sample_data.size = 0;
		return size;
	}

	/**
	* @method deplete_sample
	*/
	uint32_t Extractor::deplete_sample(uint32_t size) {
		size = Qk_MIN(size, _sample_data.size);
		_sample_data.size -= size;
		_sample_data.data += size;
		return size;
	}

	/**
	* @method advance
	*/
	bool Extractor::advance() {
		return _host->_inl->extractor_advance(this);
	}

}