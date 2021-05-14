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

#import "flare/pcm-player.h"
#import "flare/util/handle.h"
#import "flare/util/loop.h"
#import <AudioToolbox/AudioToolbox.h>

namespace flare {

	#define QUEUE_BUFFER_COUNT 3
	#define WAIT_WRITE_BUFFER_COUNT 3

	/**
	* @class ApplePCMPlayer
	*/
	class ApplePCMPlayer: public Object, public PCMPlayer {
		public:
			typedef ObjectTraits Traits;

			virtual Object* to_object() { return this; }
			
			struct WaitWriteBuffer {
				Buffer  data;
				uint32_t    size = 0;
			};
			
			ApplePCMPlayer()
			: _queue(NULL)
			, _wait_write_buffer_index(0)
			, _wait_write_buffer_count(0)
			, _channel_count(0), _sample_rate(0), _volume(1), _player(false), _flush(false) {
				memset(_buffer_all, 0, sizeof(_buffer_all));
				memset(_buffer_free, 0, sizeof(_buffer_free));
			}
			
			virtual ~ApplePCMPlayer() {
				
				for (int i = 0; i < QUEUE_BUFFER_COUNT; i++) {
					if ( _buffer_all[i] ) {
						AudioQueueFreeBuffer(_queue, _buffer_all[i]);
					}
				}
				if ( _queue ) {
					AudioQueueStop(_queue, false);
					AudioQueueDispose(_queue, false); _queue = NULL;
				}
			}
			
			bool initialize(uint32_t channel_count, uint32_t sample_rate) {
				OSStatus status;
				
				_channel_count = channel_count;
				_sample_rate = sample_rate;
				
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
				
				if ( AudioQueueNewOutput(&desc, cb, this, NULL, NULL, 0, &_queue) == noErr ) { // new
					uint32_t size = buffer_size() * 4;
					
					for (int i = 0; i < QUEUE_BUFFER_COUNT; i++) {
						if ( AudioQueueAllocateBuffer(_queue, size, &_buffer_all[i]) == noErr ) {
							_buffer_free[i] = _buffer_all[i];
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
				ScopeLock scope(_mutex);
				
				if ( _flush ) {
					_flush = false;
					AudioQueueReset(_queue);
					AudioQueueSetParameter(_queue, kAudioQueueParam_Volume, _volume);
				}
				
				if ( _player ) {
					if ( _wait_write_buffer_count ) { // Write waiting buffer data
						WaitWriteBuffer* buf = _wait_write_buffer + _wait_write_buffer_index;
						
						if (buf->size <= in->mAudioDataBytesCapacity) {
							in->mAudioDataByteSize = buf->size;            // size
							memcpy(in->mAudioData, *buf->data, buf->size);  // copy audio data
							
							// next buffer
							_wait_write_buffer_index = (_wait_write_buffer_index + 1) % WAIT_WRITE_BUFFER_COUNT;
							_wait_write_buffer_count--;
							
							if ( AudioQueueEnqueueBuffer(_queue, in, 0, NULL) == noErr ) { // input success
								return;
							}
						} else {
							FX_ERR("self->_buffer_size <= in->mAudioDataBytesCapacity, buffer Capacity Too small");
						}
					}
					
					if ( AudioQueuePause(_queue) == noErr ) {
						_player = false;
					}
				}
				
				for ( int i = 0; i < QUEUE_BUFFER_COUNT; i++ ) {
					if ( !_buffer_free[i] ) {
						_buffer_free[i] = in;
						break;
					}
				}
			}
			
			/**
			* @overwrite
			*/
			virtual bool write(cBuffer& buffer) {
				ScopeLock scope(_mutex);
				
				if ( !_player ) {
					for (int i = 0; i < QUEUE_BUFFER_COUNT; i++) { // First fill audio queue
						AudioQueueBufferRef in = _buffer_free[i];
						if (in) {
							if ( buffer.length() <= in->mAudioDataBytesCapacity ) {
								in->mAudioDataByteSize = buffer.length();          // set size
								memcpy(in->mAudioData, *buffer, buffer.length());  // copy data
								//
								if ( AudioQueueEnqueueBuffer(_queue, in, 0, NULL) == noErr ) { // input data
									_buffer_free[i] = NULL;
									return true;
								}
							}
							return false;
						}
					}
				}
				
				bool r = false;
				
				// Wait write buffer
				if ( _wait_write_buffer_count < WAIT_WRITE_BUFFER_COUNT ) {
					WaitWriteBuffer* buf =  _wait_write_buffer +
					(_wait_write_buffer_index + _wait_write_buffer_count) % WAIT_WRITE_BUFFER_COUNT;
					_wait_write_buffer_count++;
					
					buf->data.write(buffer, 0);
					buf->size = buffer.length();
					r = true;
				}
				
				if ( !_player && _wait_write_buffer_count == WAIT_WRITE_BUFFER_COUNT ) { // start play
					if ( AudioQueueStart(_queue, NULL) == noErr ) {
						_player = true;
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
				ScopeLock scope(_mutex);
				_wait_write_buffer_index = 0;
				_wait_write_buffer_count = 0;
				AudioQueueSetParameter(_queue, kAudioQueueParam_Volume, 0);
				_flush = true;
			}
			
			/**
			* @overwrite
			* */
			virtual bool set_mute(bool value) {
				AudioQueueParameterValue volume;
				OSStatus status;
				
				{ //
					ScopeLock scope(_mutex);
					status = AudioQueueGetParameter(_queue, kAudioQueueParam_Volume, &volume);
				}
				
				if ( status == noErr ) {
					if ( value ) { // mute ok
						if ( volume != 0 ) {
							_volume = volume;
							return set_volume(0);
						}
					} else { // no mute
						if ( volume == 0 ) {
							return set_volume(_volume * 100);
						}
					}
				}
				return false;
			}
			
			/**
			* @overwrite
			* */
			virtual bool set_volume(uint32_t value) {
				ScopeLock scope(_mutex);
				OSStatus status;
				AudioQueueParameterValue v;
				
				v = FX_MIN(value, 100) / 100.0;
				
				status = AudioQueueSetParameter(_queue, kAudioQueueParam_Volume, _flush ? 0 : v);
				
				if ( status == noErr ) {
					_volume = v;
					return true;
				}
				return false;
			}
			
			/**
			* @overwrite
			* */
			virtual uint32_t buffer_size() {
				return FX_MAX(4096, _channel_count * _sample_rate / 10);
			}
			
		private:
			AudioQueueRef             _queue;
			AudioQueueBufferRef       _buffer_all[QUEUE_BUFFER_COUNT];
			AudioQueueBufferRef       _buffer_free[QUEUE_BUFFER_COUNT];
			WaitWriteBuffer           _wait_write_buffer[WAIT_WRITE_BUFFER_COUNT];
			uint32_t                      _wait_write_buffer_index;
			uint32_t                      _wait_write_buffer_count;
			uint32_t                      _channel_count;
			uint32_t                      _sample_rate;
			Mutex                     _mutex;
			AudioQueueParameterValue  _volume;
			bool                      _player;
			bool                      _flush;
	};

	PCMPlayer* PCMPlayer::create(uint32_t channel_count, uint32_t sample_rate) {
		Handle<ApplePCMPlayer> player = new ApplePCMPlayer();
		if ( player->initialize(channel_count, sample_rate) ) {
			return player.collapse();
		}
		return NULL;
	}

}
