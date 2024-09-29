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
#import <AudioToolbox/AudioToolbox.h>

namespace qk {
	#define OUTPUT_BUFFER_NUM 12

	class MacVideoCodec: public MediaCodec {
	public:

		struct OutputBufferInfo {
			CVPixelBufferRef        buffer = nullptr;
			CVPixelBufferLockFlags  lock;
			uint64_t                time;
		};

		MacVideoCodec(const Stream &stream)
			: MediaCodec(stream)
			, _session(nullptr)
			, _format_desc(nullptr)
			, _sample_data(nullptr), _sample_time(0)
			, _output_buffer_count(0)
			, _video_width(0), _video_height(0)
			, _presentation_time(0)
			, _packet(nullptr)
		{
		}
		
		~MacVideoCodec() {
			close();
			if ( _sample_data ) {
				CFRelease(_sample_data);
				_sample_data = nil;
			}
			if (_format_desc) {
				CFRelease(_format_desc);
				_format_desc = nil;
			}
		}

		bool initialize() {
			CMVideoCodecType codec_type;

			switch( _stream.codec_id ) {
				case AV_CODEC_ID_H263 : // h.263
					codec_type = kCMVideoCodecType_H263;
					break;
				case AV_CODEC_ID_H264 : // h.264
					codec_type = kCMVideoCodecType_H264;
					break;
				case AV_CODEC_ID_HEVC : // h.265
					codec_type = kCMVideoCodecType_HEVC;
					break;
				case AV_CODEC_ID_MPEG1VIDEO :
					codec_type = kCMVideoCodecType_MPEG1Video;
					break;
				case AV_CODEC_ID_MPEG2VIDEO :
					codec_type = kCMVideoCodecType_MPEG2Video;
					break;
				case AV_CODEC_ID_MPEG4 :
					codec_type = kCMVideoCodecType_MPEG4Video;
					break;
				default:
					return false;
			}

			_video_width = _stream.width;
			_video_height = _stream.height;

			// init CMFormatDescriptionRef
			Buffer psp, pps;
			OSStatus status;

			if ( MediaCodec::parse_avc_psp_pps(WeakBuffer(_stream.extra.extradata).buffer(), psp, pps) ) {
				uint8_t*  param[2] = { (uint8_t*)(*psp + 4), (uint8_t*)(*pps + 4) };
				size_t size[2]  = { psp.length() - 4, pps.length() - 4 };
				status = CMVideoFormatDescriptionCreateFromH264ParameterSets(NULL, 2,
																																		param, size, 4, &_format_desc);
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
						@"avcC": [NSData dataWithBytes:*_stream.extra.extradata length:_stream.extra.extradata.length()],
					},
				};
				status = CMVideoFormatDescriptionCreate(NULL, codec_type,
																								_stream.width, _stream.height,
																								(__bridge CFDictionaryRef)extensions, &_format_desc);
			}

			// create decoder session
			if ( status == noErr ) {
				CFRetain(_format_desc);
				return open();
			}
			return false;
		}

		static void decompress_frame_cb(MacVideoCodec* self,
																		void* source_sample,
																		OSStatus status,
																		VTDecodeInfoFlags flags,
																		CVImageBufferRef buffer, CMTime pts, CMTime duration
		) {
			if (status == noErr) {
				ScopeLock scope(self->_mutex); // lock scope
				
				if ( self->_output_buffer_count < OUTPUT_BUFFER_NUM ) {
					self->_output_buffer_count++;
					
					for ( int i = 0; i < OUTPUT_BUFFER_NUM; i++ ) {
						OutputBufferInfo& output = self->_output_buffer[i];
						if ( !output.buffer ) {
							output.buffer = CVPixelBufferRetain(buffer);
							output.time = pts.value;
							CVPixelBufferLockBaseAddress(output.buffer, output.lock);
							return;
						}
					}
				} else {
					CVPixelBufferRelease(buffer);
				}
			}
		}

		bool is_open() const override {
			return _session;
		}

		bool open(const Stream *stream = nullptr) override {
			if ( _session ) return true;
			if (!stream) stream = &_stream;

			Qk_Assert(stream->width);
			Qk_Assert(stream->height);

			CFDictionaryRef attrs = (__bridge CFDictionaryRef)
			[NSDictionary dictionaryWithObjectsAndKeys:
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
				(VTDecompressionOutputCallback)&MacVideoCodec::decompress_frame_cb,
				this,
			};
			OSStatus status;
			status = VTDecompressionSessionCreate(NULL, _format_desc, NULL, attrs, &cb, &_session);
			
			if (status == noErr) {
				CFRetain(_session);
				flush();
				return true;
			} else {
				CFRelease(_session); _session = nullptr;
				return false;
			}
		}

		void close() override {
			if ( is_open() ) {
				flush();
				// Prevent deadlocks in VTDecompressionSessionInvalidate by waiting for the frames to complete manually.
				// Seems to have appeared in iOS11
				VTDecompressionSessionWaitForAsynchronousFrames(_session);
				VTDecompressionSessionInvalidate(_session);
				CFRelease(_session); _session = nullptr;
			}
		}
		
		void flush() override {
			if ( is_open() ) {
				ScopeLock scope(_mutex);
				for ( int i = 0; i < OUTPUT_BUFFER_NUM; i++ ) {
					OutputBufferInfo& output = _output_buffer[i];
					if (output.buffer) {
						CVPixelBufferUnlockBaseAddress(output.buffer, output.lock);
						CVPixelBufferRelease(output.buffer);
						output.buffer = nil;
					}
				}
				_output_buffer_count = 0;
				_presentation_time = 0;
				_need_keyframe = true;
				delete _packet; _packet = nullptr;
			}
		}

		CMSampleBufferRef get_sample_data(const Packet *pkt) {
			OSStatus status;
			CMBlockBufferRef block;

			uint64_t sample_time = pkt->pts;

			if ( _sample_data && _sample_time == sample_time ) {
				return _sample_data;
			}

			if ( _sample_data ) {
				CFRelease(_sample_data);
				_sample_data = nullptr;
			}

			auto size = pkt->size;
			auto buf = pkt->data;
			MediaCodec::convert_sample_data_to_mp4_style(buf, size);

			status = CMBlockBufferCreateWithMemoryBlock(nullptr,
																									buf,
																									size,
																									kCFAllocatorNull,
																									nullptr, 0, size, 0, &block);
			if ( status == noErr ) {
				CMSampleTimingInfo frameTimingInfo;
				frameTimingInfo.decodeTimeStamp = CMTimeMake(pkt->dts, 1);
				frameTimingInfo.duration = CMTimeMake(0, 1);
				frameTimingInfo.presentationTimeStamp = CMTimeMake(sample_time, 1);

				size_t sample_size = size;

				status = CMSampleBufferCreate(kCFAllocatorDefault, block, true,
																			nullptr,
																			nullptr,
																			_format_desc, 1, 1,
																			&frameTimingInfo, 1, &sample_size, &_sample_data);
				CFRelease(block);

				if ( status == noErr ) {
					_sample_time = sample_time;
					return _sample_data;
				}
			}
			return nullptr;
		}

		int send_packet(const Packet *pkt) override {
			if ( _session ) {
				{
					ScopeLock scope(_mutex);
					if ( _output_buffer_count >= uint32_t(OUTPUT_BUFFER_NUM / 1.5) ) {
						return AVERROR(EAGAIN);
					}
				}
				if ( _need_keyframe ) { // need key frame
					if ( pkt->flags & AV_PKT_FLAG_KEY ) { // i frame
						_need_keyframe = false; // start
					} else { // Discard
						return 0;
					}
				}

				OSStatus status;
				VTDecodeFrameFlags flags = kVTDecodeFrame_EnableAsynchronousDecompression;
				CMSampleBufferRef sample_data = get_sample_data(pkt);
				VTDecodeInfoFlags flagOut;

				if ( sample_data ) {
					status = VTDecompressionSessionDecodeFrame(_session, sample_data, flags, NULL, &flagOut);

					if (status == noErr) {
						return 0;
					} else if (status == kVTInvalidSessionErr) {
						close();
						open(); // reopen
					}
				}
			}
			return AVERROR(ENOMEM);
		}

		int send_packet_for(Extractor *extractor) override {
			Qk_Assert_Eq(type(), extractor->type());
			if (!_session) {
				return AVERROR(EINVAL);
			}
			auto packet = _packet ? _packet: extractor->advance();
			if (!packet) {
				return AVERROR(EAGAIN);
			}
			int rc = send_packet(_packet);
			if (rc) {
				ScopeLock lock(_mutex);
				_packet = packet;
			} else if (_packet) {
				ScopeLock lock(_mutex);
				delete _packet; _packet = nullptr;
			}
			return rc;
		}

		int receive_frame(Frame **frame) override {
			ScopeLock scope(_mutex);
			/*
			if ( _output_buffer_count ) {
				OutputBufferInfo* buf = nullptr;
				OutputBufferInfo* unknown_time_frame = nullptr;
				Frame out;

				// sort frame
				for (int i = 0; i < OUTPUT_BUFFER_NUM; i++) {
					OutputBufferInfo* b2 = _output_buffer + i;
					if ( b2->buffer ) {
						if ( b2->time == Uint64::limit_max ) { //  Unknown time frame
							if ( unknown_time_frame ) {
								_output_buffer_count--;
								CVPixelBufferUnlockBaseAddress(unknown_time_frame->buffer, unknown_time_frame->lock);
								CVPixelBufferRelease(unknown_time_frame->buffer);
								unknown_time_frame->buffer = nullptr;
							}
							unknown_time_frame = b2;
						} else {
							if ( buf ) {
								if ( b2->time < buf->time ) {
									buf = b2; out.index = i;
								}
							} else {
								buf = b2; out.index = i;
							}
						}
					}
				}

				// correct time
				if ( buf ) {
					if ( _presentation_time ) {
						int64_t diff = buf->time - _presentation_time - _stream.average_framerate;
						if ( diff > _stream.average_framerate / 2 && diff < _stream.average_framerate * 3 ) {
							if (unknown_time_frame) {
								buf = unknown_time_frame;
								buf->time = _presentation_time + _stream.average_framerate;
							}
						}
					}
				} else {
					buf = unknown_time_frame;
					buf->time = _presentation_time + _stream.average_framerate;
				}

				// yuv420sp
				out.linesize[0] =  _video_width * _video_height;
				out.linesize[1] = out.linesize[0] / 2;
				out.data[0] = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(buf->buffer, 0);  // y
				out.data[1] = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(buf->buffer, 1);  // uv
				out.time  = _presentation_time = buf->time;
				out.total = out.linesize[0] + out.linesize[1];
				return out;
			}
			return Frame();*/
		}

		void release(Frame& frame) {
			/*ScopeLock scope(_mutex);

			if ( frame.total ) {
				int index = frame.index;

				memset(&frame, 0, sizeof(Frame));
				if ( _output_buffer[index].buffer ) {
					OutputBufferInfo* buf = _output_buffer + index;
					_output_buffer_count--;
					CVPixelBufferUnlockBaseAddress(buf->buffer, buf->lock);
					CVPixelBufferRelease(buf->buffer);
					buf->buffer = nullptr;
				}
			}*/
		}

		void set_threads(uint32_t value) override {}

	private:
		VTDecompressionSessionRef _session;
		CMFormatDescriptionRef _format_desc;
		CMSampleBufferRef  _sample_data;
		uint64_t         _sample_time;
		Mutex            _mutex;
		OutputBufferInfo _output_buffer[OUTPUT_BUFFER_NUM];
		uint32_t  _output_buffer_count;
		uint32_t  _video_width, _video_height;
		uint64_t  _presentation_time;
		Packet*   _packet;
		bool      _need_keyframe;
	};

	MediaCodec* MediaCodec_hardware(MediaType type, MediaSource* source) {
		Extractor* ex = source->extractor(type);
		if ( ex ) {
			if (type == kAudio_MediaType) {
				return nullptr;
			}
			//Sp<MacVideoCodec> codec = new MacVideoCodec(ex->stream());
			//if (codec->initialize()) {
			//	return codec.collapse();
			//}
		}
		return nullptr;
	}

}
