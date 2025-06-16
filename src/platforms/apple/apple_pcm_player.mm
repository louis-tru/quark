/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
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
 * ***** END LICENSE BLOCK ***** */

#import "src/media/pcm_player.h"
#import "src/util/handle.h"
#import "src/util/loop.h"
#import <AudioToolbox/AudioToolbox.h>

namespace qk {

	class ApplePCMPlayer: public Object, public PCMPlayer {
	public:
		Object* asObject() override { return this; }

		ApplePCMPlayer()
			: _audio(nil)
			, _queue_num_half(0)
			, _volume(1), _play(false), _mute(false)
		{
		}

		~ApplePCMPlayer() {
			{
				ScopeLock scope(_mutex);
				for (auto buf: _buffers)
					AudioQueueFreeBuffer(_audio, buf);
				_queue.clear();
			}
			if ( _audio ) {
				AudioQueueStop(_audio, false);
				AudioQueueDispose(_audio, false); _audio = nullptr;
			}
		}

		bool init(const Stream& stream) {
			OSStatus status;
			AudioStreamBasicDescription desc;
			desc.mSampleRate = stream.sample_rate;
			desc.mFormatID = kAudioFormatLinearPCM;
			desc.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
			desc.mFramesPerPacket = 1;
			desc.mChannelsPerFrame = stream.channels;
			desc.mBitsPerChannel = 16;
			desc.mBytesPerPacket =
			desc.mBytesPerFrame = (desc.mBitsPerChannel / 8) * stream.channels;
			desc.mReserved = 0;

			AudioChannelLayout layout;
			layout.mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelBitmap;
			layout.mChannelBitmap = 0xffffffff & stream.channel_layout;
			layout.mNumberChannelDescriptions = 0;

			auto cb = [](void *ctx, AudioQueueRef audio, AudioQueueBufferRef in) {
				static_cast<ApplePCMPlayer*>(ctx)->callback_proc(in);
			};
			if (AudioQueueNewOutput(&desc, cb, this, nil, nil, 0, &_audio) != noErr) {
				return false;
			}
			if (AudioQueueSetProperty(_audio, kAudioQueueProperty_ChannelLayout, &layout, sizeof(layout)) != noErr) {
				return false;
			}

			_sample_rate = stream.sample_rate;
			_channels = stream.channels;
			_channel_layout = stream.channel_layout;

			return true;
		}

		void callback_proc(AudioQueueBufferRef in) {
			ScopeLock scope(_mutex);
			_queue.pushBack(in);
			Qk_ASSERT_LE(_queue.length(), _buffers.length()); // <=
			//Qk_DLog("callback_proc, idle:%d %p", len, in);
			if (_queue.length() == _buffers.length()) {
				_play = false;
				Qk_ASSERT_EQ(AudioQueueStop(_audio, true), noErr);
			}
		}

		bool write(const Frame *frame) override {
			ScopeLock scope(_mutex);

			if (!_queue_num_half) {
				_queue_num_half = 1;// ceilf(_sample_rate / (frame->nb_samples * 75.0f));
				//auto size = frame->nb_samples * _channels * 2;
				int size = _channels * _sample_rate * 2 * (32.0f / 1000.0f); // 32ms
				Qk_ASSERT_NE(0, size);
				auto num = _queue_num_half * 2 + 1;
				for (int i = 0; i < num; i++) {
					AudioQueueBufferRef buf;
					Qk_ASSERT_EQ(noErr,
					AudioQueueAllocateBuffer(_audio, size, &buf));
					_buffers.push(buf);
					_queue.pushBack(buf);
				}
			}

			if (_queue.length()) {
				AudioQueueBufferRef &buf = _queue.front();
				if (buf->mAudioDataBytesCapacity < frame->linesize[0]) {
					AudioQueueFreeBuffer(_audio, buf);
					Qk_ASSERT_EQ(noErr,
					AudioQueueAllocateBuffer(_audio, frame->linesize[0], &buf));
				}
				buf->mAudioDataByteSize = frame->linesize[0];
				memcpy(buf->mAudioData, frame->data[0], frame->linesize[0]);

				if (AudioQueueEnqueueBuffer(_audio, buf, 0, nullptr) == noErr) {
					_queue.popFront();
					//Qk_DLog("PCM_write,  ok idle:%d %p", _queue.length(), buf);
					if (!_play) {
						_play = true;
						Qk_ASSERT_EQ(noErr, AudioQueueStart(_audio, nullptr));
						Qk_DLog("AudioQueueStart");
					}
					return true;
				}
			}
			// Qk_DLog("PCM_write fail %d %d", _queue.length(), frame->pts);
			return false;
		}

		float delayed() override {
			return Qk_Max(1, _queue_num_half);
		}

		void flush() override {
			Qk_ASSERT_EQ(noErr, AudioQueueReset(_audio));
		}

		void set_mute(bool value) override {
			_mute = value;
			set_volume(_volume);
		}

		void set_volume(float value) override {
			ScopeLock scope(_mutex);
			_volume = Float32::clamp(value, 0, 1);
			Qk_ASSERT_EQ(noErr,
				AudioQueueSetParameter(_audio, kAudioQueueParam_Volume, _mute ? 0: _volume)
			);
		}

	private:
		AudioQueueRef              _audio;
		Array<AudioQueueBufferRef> _buffers;
		List<AudioQueueBufferRef>  _queue;
		uint32_t                   _queue_num_half;
		uint32_t                   _sample_rate, _channels;
		uint64_t                   _channel_layout;
		float                      _volume;
		Mutex                      _mutex;
		bool                       _play, _mute;
	};

	PCMPlayer* PCMPlayer::create(const Stream &stream) {
		Sp<ApplePCMPlayer> pcm = new ApplePCMPlayer();
		if ( pcm->init(stream) ) {
			return pcm.collapse();
		}
		return nullptr;
	}
}
