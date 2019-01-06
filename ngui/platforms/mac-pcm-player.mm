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

#include "../pcm-player.h"
#include "ngui/utils/handle.h"
#include "ngui/utils/loop.h"
#include <Audiotoolbox/AudioToolbox.h>

XX_NS(ngui)

#define QUEUE_BUFFER_COUNT 3
#define WAIT_WRITE_BUFFER_COUNT 3

/**
 * @class ApplePCMPlayer
 */
class ApplePCMPlayer: public Object, public PCMPlayer {
 public:
	typedef DefaultTraits Traits;
	
	struct WaitWriteBuffer {
		Buffer  data;
		uint    size = 0;
	};
	
	ApplePCMPlayer()
	: m_queue(NULL)
	, m_wait_write_buffer_index(0)
	, m_wait_write_buffer_count(0)
	, m_channel_count(0), m_sample_rate(0), m_volume(1), m_player(false), m_flush(false) {
		memset(m_buffer_all, 0, sizeof(m_buffer_all));
		memset(m_buffer_free, 0, sizeof(m_buffer_free));
	}
	
	virtual ~ApplePCMPlayer() {
		
		for (int i = 0; i < QUEUE_BUFFER_COUNT; i++) {
			if ( m_buffer_all[i] ) {
				AudioQueueFreeBuffer(m_queue, m_buffer_all[i]);
			}
		}
		if ( m_queue ) {
			AudioQueueStop(m_queue, false);
			AudioQueueDispose(m_queue, false); m_queue = NULL;
		}
	}
	
	bool initialize(uint channel_count, uint sample_rate) {
		OSStatus status;
		
		m_channel_count = channel_count;
		m_sample_rate = sample_rate;
		
		AudioStreamBasicDescription desc;
		
		desc.mSampleRate  = sample_rate;
		desc.mFormatID    = kAudioFormatLinearPCM;
		desc.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
		desc.mChannelsPerFrame  = channel_count;
		desc.mFramesPerPacket   = 1;
		desc.mBitsPerChannel    = 16;
		desc.mBytesPerFrame     = 2 * channel_count;
		desc.mBytesPerPacket    = 2 * channel_count;
		
		AudioQueueOutputCallback cb = (AudioQueueOutputCallback)&ApplePCMPlayer::buffer_callback;
		
		if ( AudioQueueNewOutput(&desc, cb, this, NULL, NULL, 0, &m_queue) == noErr ) { // new
			uint size = buffer_size() * 4;
			
			for (int i = 0; i < QUEUE_BUFFER_COUNT; i++) {
				if ( AudioQueueAllocateBuffer(m_queue, size, &m_buffer_all[i]) == noErr ) {
					m_buffer_free[i] = m_buffer_all[i];
				} else {
					return false;
				}
			}
			return true;
		}
		return false;
	}
	
	/**
	 * @func buffer_callback
	 */
	static void buffer_callback(ApplePCMPlayer* self,
															AudioQueueRef queue, AudioQueueBufferRef in) {
		self->buffer_callback2(in);
	}
	
	/**
	 * @func buffer_callback2
	 */
	void buffer_callback2(AudioQueueBufferRef in) {
		ScopeLock scope(m_mutex);
		
		if ( m_flush ) {
			m_flush = false;
			AudioQueueReset(m_queue);
			AudioQueueSetParameter(m_queue, kAudioQueueParam_Volume, m_volume);
		}
		
		if ( m_player ) {
			if ( m_wait_write_buffer_count ) { // Write waiting buffer data
				WaitWriteBuffer* buf = m_wait_write_buffer + m_wait_write_buffer_index;
				
				if (buf->size <= in->mAudioDataBytesCapacity) {
					in->mAudioDataByteSize = buf->size;            // size
					memcpy(in->mAudioData, *buf->data, buf->size);  // copy audio data
					
					// next buffer
					m_wait_write_buffer_index = (m_wait_write_buffer_index + 1) % WAIT_WRITE_BUFFER_COUNT;
					m_wait_write_buffer_count--;
					
					if ( AudioQueueEnqueueBuffer(m_queue, in, 0, NULL) == noErr ) { // input success
						return;
					}
				} else {
					XX_ERR("self->m_buffer_size <= in->mAudioDataBytesCapacity, buffer Capacity Too small");
				}
			}
			
			if ( AudioQueuePause(m_queue) == noErr ) {
				m_player = false;
			}
		}
		
		for ( int i = 0; i < QUEUE_BUFFER_COUNT; i++ ) {
			if ( !m_buffer_free[i] ) {
				m_buffer_free[i] = in;
				break;
			}
		}
	}
	
	/**
	 * @overwrite
	 */
	virtual bool write(cBuffer& buffer) {
		ScopeLock scope(m_mutex);
		
		if ( !m_player ) {
			for (int i = 0; i < QUEUE_BUFFER_COUNT; i++) { // First fill audio queue
				AudioQueueBufferRef in = m_buffer_free[i];
				if (in) {
					if ( buffer.length() <= in->mAudioDataBytesCapacity ) {
						in->mAudioDataByteSize = buffer.length();          // set size
						memcpy(in->mAudioData, *buffer, buffer.length());  // copy data
						//
						if ( AudioQueueEnqueueBuffer(m_queue, in, 0, NULL) == noErr ) { // input data
							m_buffer_free[i] = NULL;
							return true;
						}
					}
					return false;
				}
			}
		}
		
		bool r = false;
		
		// Wait write buffer
		if ( m_wait_write_buffer_count < WAIT_WRITE_BUFFER_COUNT ) {
			WaitWriteBuffer* buf =  m_wait_write_buffer +
			(m_wait_write_buffer_index + m_wait_write_buffer_count) % WAIT_WRITE_BUFFER_COUNT;
			m_wait_write_buffer_count++;
			
			buf->data.write(buffer, 0);
			buf->size = buffer.length();
			r = true;
		}
		
		if ( !m_player && m_wait_write_buffer_count == WAIT_WRITE_BUFFER_COUNT ) { // start play
			if ( AudioQueueStart(m_queue, NULL) == noErr ) {
				m_player = true;
			}
		}
		return r;
	}
	
	/**
	 * @overwrite
	 */
	virtual float compensate() {
		return -1.0;
	}

	/**
	 * @overwrite
	 */
	virtual void flush() {
		ScopeLock scope(m_mutex);
		m_wait_write_buffer_index = 0;
		m_wait_write_buffer_count = 0;
		AudioQueueSetParameter(m_queue, kAudioQueueParam_Volume, 0);
		m_flush = true;
	}
	
	/**
	 * @overwrite
	 * */
	virtual bool set_mute(bool value) {
		AudioQueueParameterValue volume;
		OSStatus status;
		
		{ //
			ScopeLock scope(m_mutex);
			status = AudioQueueGetParameter(m_queue, kAudioQueueParam_Volume, &volume);
		}
		
		if ( status == noErr ) {
			if ( value ) { // mute ok
				if ( volume != 0 ) {
					m_volume = volume;
					return set_volume(0);
				}
			} else { // no mute
				if ( volume == 0 ) {
					return set_volume(m_volume * 100);
				}
			}
		}
		return false;
	}
	
	/**
	 * @overwrite
	 * */
	virtual bool set_volume(uint value) {
		ScopeLock scope(m_mutex);
		OSStatus status;
		AudioQueueParameterValue v;
		
		v = XX_MIN(value, 100) / 100.0;
		
		status = AudioQueueSetParameter(m_queue, kAudioQueueParam_Volume, m_flush ? 0 : v);
		
		if ( status == noErr ) {
			m_volume = v;
			return true;
		}
		return false;
	}
	
	/**
	 * @overwrite
	 * */
	virtual uint buffer_size() {
		return XX_MAX(4096, m_channel_count * m_sample_rate / 10);
	}
	
 private:
	AudioQueueRef             m_queue;
	AudioQueueBufferRef       m_buffer_all[QUEUE_BUFFER_COUNT];
	AudioQueueBufferRef       m_buffer_free[QUEUE_BUFFER_COUNT];
	WaitWriteBuffer           m_wait_write_buffer[WAIT_WRITE_BUFFER_COUNT];
	uint                      m_wait_write_buffer_index;
	uint                      m_wait_write_buffer_count;
	uint                      m_channel_count;
	uint                      m_sample_rate;
	Mutex                     m_mutex;
	AudioQueueParameterValue  m_volume;
	bool                      m_player;
	bool                      m_flush;
};

PCMPlayer* PCMPlayer::create(uint channel_count, uint sample_rate) {
	Handle<ApplePCMPlayer> player = new ApplePCMPlayer();
	if ( player->initialize(channel_count, sample_rate) ) {
		return player.collapse();
	}
	return NULL;
}

XX_END
