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

#import "quark/media/media_inl.h"
#import <VideoToolbox/VideoToolbox.h>

namespace qk {

	void deleteCMFormatDescription(CMFormatDescriptionRef ref) {
		if (ref)
			CFRelease(ref);
	}
	void retainCMFormatDescription(CMFormatDescriptionRef ref) {
		if (ref)
			CFRetain(ref);
	}

	typedef std::remove_pointer_t<CMFormatDescriptionRef> CMFormatDescription;
	typedef Sp<
		CMFormatDescription,
		object_traits_from<CMFormatDescription, deleteCMFormatDescription, retainCMFormatDescription>
	> CMFormatDescriptionAuto;

	class AppleVideoCodec: public MediaCodec {
	public:
		AppleVideoCodec(const Stream &stream): MediaCodec(stream)
			, _session(nil)
			, _packet(nil)
		{}

		~AppleVideoCodec() {
			close();
		}

		bool is_open() const override {
			return _session;
		}

		bool open(const Stream *stream = nullptr) override {
			ScopeLock scope(_mutex);

			if (_session) return true;
			if (!stream) stream = &_stream;
			if (stream->type != kVideo_MediaType) return false;

			Qk_ASSERT_NE(0, stream->width);
			Qk_ASSERT_NE(0, stream->height);

			if (!_desc || *stream != _stream) {
				CMVideoCodecType codec;
				switch( stream->codec_id ) {
					case AV_CODEC_ID_H263: // h.263
						codec = kCMVideoCodecType_H263; break;
					case AV_CODEC_ID_H264: // h.264
						codec = kCMVideoCodecType_H264; break;
					case AV_CODEC_ID_HEVC: // h.265
						codec = kCMVideoCodecType_HEVC; break;
					case AV_CODEC_ID_MPEG1VIDEO:
						codec = kCMVideoCodecType_MPEG1Video; break;
					case AV_CODEC_ID_MPEG2VIDEO:
						codec = kCMVideoCodecType_MPEG2Video; break;
					case AV_CODEC_ID_MPEG4:
						codec = kCMVideoCodecType_MPEG4Video; break;
					default:
						return false;
				}

				Buffer psp, pps;
				OSStatus status;
				auto &extra = stream->extra.extradata;

				CMFormatDescriptionRef desc;

				if ( MediaCodec::parse_avc_psp_pps(extra, psp, pps) ) {
					uint8_t *param[2] = { (uint8_t*)(*psp + 4), (uint8_t*)(*pps + 4) };
					size_t   size[2]  = { psp.length() - 4, pps.length() - 4 };
					status = CMVideoFormatDescriptionCreateFromH264ParameterSets(nil, 2, param, size, 4, &desc);
				} else {
					NSDictionary* extensions =
					@{
						@"CVImageBufferChromaLocationBottomField": @"left",
						@"CVImageBufferChromaLocationTopField": @"left",
						@"FullRangeVideo": @NO,
						@"CVPixelAspectRatio": @{
							@"HorizontalSpacing": @0,
							@"VerticalSpacing": @0,
						},
						@"SampleDescriptionExtensionAtoms": @{
							@"avcC": [NSData dataWithBytes:*extra length:extra.length()],
						},
					};
					status = CMVideoFormatDescriptionCreate(nil, codec,
																									stream->width, stream->height,
																									(__bridge CFDictionaryRef)extensions, &desc);
				}

				if ( status != noErr ) {
					return false;
				}
				_desc = desc;
			}

			CFDictionaryRef attrs = (__bridge CFDictionaryRef)[NSDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithInt: kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange],
					(id)kCVPixelBufferPixelFormatTypeKey,
				[NSNumber numberWithUnsignedInt:stream->width], (id)kCVPixelBufferWidthKey,
				[NSNumber numberWithUnsignedInt:stream->height], (id)kCVPixelBufferHeightKey,
#if Qk_iOS
				[NSNumber numberWithBool:NO], (id)kCVPixelBufferOpenGLESCompatibilityKey,
#else
				[NSNumber numberWithBool:NO], (id)kCVPixelBufferOpenGLCompatibilityKey,
#endif
				nil
			];

			VTDecompressionOutputCallbackRecord cb = {
				(VTDecompressionOutputCallback)&AppleVideoCodec::decompress_frame_cb,
				this,
			};
			if (VTDecompressionSessionCreate(nil, _desc.get(), nil, attrs, &cb, &_session) != noErr) {
				return false;
			}

			if (stream != &_stream) {
				_stream = *stream;
			}
			CFRetain(_session);
			clearData();
			return true;
		}

		void clearData() {
			if ( _session ) {
				for (auto f: _frames)
					delete f;
				_frames.clear();
				_need_keyframe = true;
				_pending = 0;
				Releasep(_packet);
			}
		}

		void close() override {
			if (_session) {
				VTDecompressionSessionWaitForAsynchronousFrames(_session);
				ScopeLock scope(_mutex);
				clearData();
				// Prevent deadlocks in VTDecompressionSessionInvalidate 
				// by waiting for the frames to complete manually.
				// Seems to have appeared in iOS11
				VTDecompressionSessionInvalidate(_session);
				CFRelease(_session); _session = nil;
			}
		}

		void flush() override {
			if (_session) {
				VTDecompressionSessionWaitForAsynchronousFrames(_session);
				ScopeLock scope(_mutex);
				clearData();
			}
		}

		static void decompress_frame_cb(AppleVideoCodec* self, void* source, OSStatus status,
																		VTDecodeInfoFlags flags,
																		CVImageBufferRef buffer, CMTime pts, CMTime duration
		) {
			if (status != noErr)
				return;
			ScopeLock scope(self->_mutex); // lock scope
			auto w = self->_stream.width, h = self->_stream.height;
			auto f = Frame::Make();
			auto avf = f->avframe;
			auto buf = av_buffer_alloc(av_image_get_buffer_size(AV_PIX_FMT_NV12, w, h, 1));
			Qk_ASSERT_EQ(buf->size,
				av_image_fill_arrays(avf->data, avf->linesize, buf->data, AV_PIX_FMT_NV12, w, h, 1)
			);
			CVPixelBufferLockFlags lock;
			CVPixelBufferLockBaseAddress(buffer, lock);

			memcpy(avf->data[0], CVPixelBufferGetBaseAddressOfPlane(buffer, 0), avf->linesize[0] * h); // y
			memcpy(avf->data[1], CVPixelBufferGetBaseAddressOfPlane(buffer, 1), avf->linesize[1] * h >> 1); // uv

			CVPixelBufferUnlockBaseAddress(buffer, lock);

			avf->buf[0] = buf;
			avf->extended_data = nil;
			avf->format = AV_PIX_FMT_NV12;
			avf->flags = 0; // flags
			avf->key_frame = 0;
			avf->width = w;
			avf->height = h;
			f->data = avf->data;
			f->linesize = reinterpret_cast<uint32_t*>(avf->linesize);
			f->dataitems = 2;
			f->pts = Qk_Max(0, pts.value);
			f->pkt_duration = duration.value;
			f->nb_samples = 0;
			f->width = w;
			f->height = h;
			f->format = kYUV420SP_ColorType; // yuv420sp

			self->_frames.pushBack(f);
		}

		CMSampleBufferRef new_sample_data(const Packet *pkt) {
			if (!MediaCodec::convert_sample_data_to_mp4_style(pkt->data, pkt->size))
				return nil;

			OSStatus status;
			CMBlockBufferRef block;
			CMSampleBufferRef buf = nil;

			status = CMBlockBufferCreateWithMemoryBlock(nil,
																									pkt->data, pkt->size,
																									kCFAllocatorNull,
																									nil, 0, pkt->size, 0, &block);
			if ( status != noErr )
				return nil;

			CMSampleTimingInfo info;
			info.duration = CMTimeMake(pkt->duration, 1e6);
			info.presentationTimeStamp = CMTimeMake(pkt->pts, 1e6);
			info.decodeTimeStamp = CMTimeMake(pkt->dts, 1e6);

			size_t ssize = pkt->size;

			status = CMSampleBufferCreate(kCFAllocatorDefault,
																		block, true, nil, nil,
																		_desc.get(), 1, 1, &info, 1, &ssize, &buf);
			CFRelease(block);

			return buf;
		}

		/**
		 * Non atomic safe method
		 */
		int send_packet(const Packet *pkt) override {
			if (_session && pkt) {
				if (_frames.length() >= 6)
					return AVERROR(EAGAIN);
				if ( _need_keyframe ) { // need key frame
					if ( pkt->flags & AV_PKT_FLAG_KEY ) { // i frame
						_need_keyframe = false; // start
					} else { // Discard
						return 0;
					}
				}
				OSStatus status;
				VTDecodeFrameFlags flags = kVTDecodeFrame_EnableAsynchronousDecompression;
				VTDecodeInfoFlags flagOut;
				CMSampleBufferRef data = new_sample_data(pkt);

				if ( data ) {
					status = VTDecompressionSessionDecodeFrame(_session, data, flags, nil, &flagOut);
					Qk_ASSERT_NE(status, kVTInvalidSessionErr); // need reopen ?

					if (status == noErr)
						_pending++;
					CFRelease(data);
					return status;
				}
			}
			return AVERROR(ENOMEM);
		}

		int send_packet(Extractor *extractor) override {
			if (!extractor || !_session) {
				return AVERROR(EINVAL);
			}
			Qk_ASSERT_EQ(type(), extractor->type());

			Lock lock(_mutex);
			if (!_packet) {
				_packet = extractor->advance();
			}
			if (!_packet) {
				return AVERROR(EAGAIN);
			}
			lock.unlock();
			int rc = send_packet(_packet);
			if (rc == 0) {
				lock.lock();
				Releasep(_packet);
			}
			return rc;
		}

		Frame* receive_frame() override {
			if (_frames.length()) {
				ScopeLock scope(_mutex);
				auto cu = _frames.begin(), it = cu.next();
				if ((*cu)->pts) {
					auto len = Uint32::min(_frames.length() - 1, 5);
					for (int i = 0; i < len; i++) {
						if ((*cu)->pts > (*it)->pts)
							cu = it; // Sort frames
						it++;
					}
				}
				auto f = *cu;
				_frames.erase(cu);
				_pending--;
				return f;
			}
			return nullptr;
		}

		void set_threads(uint32_t value) override {}

		bool finished() override {
			return _pending == 0;
		}

	private:
		CMFormatDescriptionAuto _desc;
		VTDecompressionSessionRef _session;
		Mutex        _mutex;
		Packet      *_packet;
		List<Frame*> _frames;
		bool         _need_keyframe;
		int          _pending;
	};

	MediaCodec* MediaCodec_hardware(MediaType type, Extractor* ex) {
		if ( ex ) {
			if (type == kVideo_MediaType) {
				Sp<AppleVideoCodec> codec = new AppleVideoCodec(ex->stream());
				if (codec->open()) {
					return codec.collapse();
				}
			}
		}
		return nil;
	}

}
