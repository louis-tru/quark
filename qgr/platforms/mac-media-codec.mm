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

#import "../media-codec-1.h"
#import <VideoToolbox/VideoToolbox.h>
#import <AudioToolbox/AudioToolbox.h>

XX_NS(qgr)

#define OUTPUT_BUFFER_NUM 12

/**
 * @class AppleVideoCodec
 */
class AppleVideoCodec: public MediaCodec {
 public:
	
	struct OutputBufferInfo {
		CVPixelBufferRef        buffer = nullptr;
		CVPixelBufferLockFlags  lock;
		uint64                  time;
	};
	
	AppleVideoCodec(Extractor* ex)
		: MediaCodec(ex)
		, m_session(nullptr)
		, m_format_desc(nullptr)
		, m_sample_data(nullptr), m_sample_time(0)
		, m_output_buffer_count(0)
		, m_start_decoder(0)
		, m_video_width(0), m_video_height(0)
		, m_presentation_time(0) 
	{
		
	}
	
	virtual ~AppleVideoCodec() {
		close();
		if ( m_sample_data ) {
			CFRelease(m_sample_data);
			m_sample_data = nil;
		}
		if (m_format_desc) {
			CFRelease(m_format_desc);
			m_format_desc = nil;
		}
	}
	
	/**
	 * @func initializ
	 */
	bool initialize() {
		
		OSStatus status;
		CMVideoCodecType codec_type;
		const TrackInfo& track = m_extractor->track();
		
		switch( track.codec_id ) {
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
		
		m_video_width = track.width;
		m_video_height = track.height;
		m_color_format = VIDEO_COLOR_FORMAT_YUV420SP;
		
		// init CMFormatDescriptionRef
		Buffer psp, pps;
		
		if ( MediaCodec::parse_avc_psp_pps(track.extradata, psp, pps) ) {
			byte*  param[2] = { (byte*)(*psp + 4), (byte*)(*pps + 4) };
			size_t size[2]  = { psp.length() - 4, pps.length() - 4 };
			status = CMVideoFormatDescriptionCreateFromH264ParameterSets(NULL, 2,
																																	 param, size, 4, &m_format_desc);
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
					@"avcC": [NSData dataWithBytes:*track.extradata length:track.extradata.length()],
				},
			};
			status = CMVideoFormatDescriptionCreate(NULL, codec_type,
																							track.width, track.height,
																							(__bridge CFDictionaryRef)extensions, &m_format_desc);
		}
		
		// create decoder session
		
		if ( status == noErr ) {
			CFRetain(m_format_desc);
			return open();
		}
		return false;
	}
	
	/**
	 * @func decompress_frame_cb
	 */
	static void decompress_frame_cb(AppleVideoCodec* self,
																	void* source_sample,
																	OSStatus status,
																	VTDecodeInfoFlags flags,
																	CVImageBufferRef buffer, CMTime pts, CMTime duration) 
	{
		if (status == noErr) {
			ScopeLock scope(self->m_output_buffer_mutex); // lock scope
			
			if ( self->m_output_buffer_count < OUTPUT_BUFFER_NUM ) {
				self->m_output_buffer_count++;
				
				for ( int i = 0; i < OUTPUT_BUFFER_NUM; i++ ) {
					OutputBufferInfo& output = self->m_output_buffer[i];
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
	
	/**
	 * @overwrite
	 */
	virtual bool open() {
		
		if ( m_session )  {
			return true;
		}
		
		m_start_decoder = 0;
		OSStatus status;
		const TrackInfo& track = m_extractor->track();
		
		XX_ASSERT(track.width);
		XX_ASSERT(track.height);
		
		CFDictionaryRef attrs = (__bridge CFDictionaryRef)
		[NSDictionary dictionaryWithObjectsAndKeys:
		 [NSNumber numberWithInt:
			kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange], (id)kCVPixelBufferPixelFormatTypeKey,
			[NSNumber numberWithUnsignedInt:track.width], (id)kCVPixelBufferWidthKey,
			[NSNumber numberWithUnsignedInt:track.height], (id)kCVPixelBufferHeightKey,
		 #if XX_IOS
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
		status = VTDecompressionSessionCreate(NULL, m_format_desc, NULL, attrs, &cb, &m_session);
		
		XX_ASSERT(status >= 0);
		
		CFRetain(m_session);
		
		flush();
		
		return status == noErr;
	}
	
	/**
	 * @overwrite
	 */
	virtual bool close() {
		if ( m_session ) {
			flush();
			// Prevent deadlocks in VTDecompressionSessionInvalidate by waiting for the frames to complete manually.
			// Seems to have appeared in iOS11
			VTDecompressionSessionWaitForAsynchronousFrames(m_session);
			VTDecompressionSessionInvalidate(m_session);
			CFRelease(m_session);
			m_session = nullptr;
		}
		return true;
	}
	
	/**
	 * @overwrite
	 */
	virtual bool flush() {
		ScopeLock scope(m_output_buffer_mutex);
		
		for ( int i = 0; i < OUTPUT_BUFFER_NUM; i++ ) {
			OutputBufferInfo& output = m_output_buffer[i];
			if (output.buffer) {
				CVPixelBufferUnlockBaseAddress(output.buffer, output.lock);
				CVPixelBufferRelease(output.buffer);
				output.buffer = nil;
			}
		}
		m_output_buffer_count = 0;
		m_presentation_time = 0;
		m_start_decoder = 0;
		return true;
	}
	
	/**
	 * @func get_sample_data
	 */
	CMSampleBufferRef get_sample_data() {
		OSStatus status;
		CMBlockBufferRef block;
		
		uint64 sample_time = m_extractor->sample_time();
		
		if ( m_sample_data && m_sample_time == sample_time ) {
			return m_sample_data;
		}
		
		if ( m_sample_data ) {
			CFRelease(m_sample_data);
			m_sample_data = nullptr;
		}
		
		WeakBuffer data = m_extractor->sample_data();
		MediaCodec::convert_sample_data_to_mp4_style(data);
		
		status = CMBlockBufferCreateWithMemoryBlock(nullptr,
																								(byte*)*data,
																								data.length(),
																								kCFAllocatorNull,
																								nullptr, 0, data.length(), 0, &block);
		if ( status == noErr ) {
			CMSampleTimingInfo frameTimingInfo;
			frameTimingInfo.decodeTimeStamp = CMTimeMake(m_extractor->sample_d_time(), 1);
			frameTimingInfo.duration = CMTimeMake(0, 1);
			frameTimingInfo.presentationTimeStamp = CMTimeMake(sample_time, 1);
			
			size_t sample_size = data.length();
			
			status = CMSampleBufferCreate(kCFAllocatorDefault, block, true,
																		nullptr,
																		nullptr,
																		m_format_desc, 1, 1,
																		&frameTimingInfo, 1, &sample_size, &m_sample_data);
			CFRelease(block);
			
			if ( status == noErr ) {
				m_sample_time = sample_time;
				return m_sample_data;
			}
		}
		return nullptr;
	}
	
	/**
	 * @overwrite
	 * */
	virtual bool advance() {
		if ( m_session && m_extractor->advance() ) {
			{ //
				ScopeLock scope(m_output_buffer_mutex);
				if ( m_output_buffer_count >= uint(OUTPUT_BUFFER_NUM / 1.5) ) {
					return false;
				}
			}
			
			if ( !m_start_decoder ) { // no start decoder
				if ( m_extractor->sample_flags() ) { // i frame
					m_start_decoder = 1; // start
				} else { // Discard
					m_extractor->deplete_sample();
					return false;
				}
			}
			
			OSStatus status;
			VTDecodeFrameFlags flags = kVTDecodeFrame_EnableAsynchronousDecompression;
			CMSampleBufferRef sample_data = get_sample_data();
			VTDecodeInfoFlags flagOut;
			
			if ( sample_data ) {
				status = VTDecompressionSessionDecodeFrame(m_session, sample_data, flags, NULL, &flagOut);
				
				if ( status == noErr ) {
					if ( m_extractor->eof_flags() ) {
						XX_DEBUG("%s", "eos flags");
					}
					m_extractor->deplete_sample();
					return true;
				} else  {
					if ( status == kVTInvalidSessionErr ) {
						// reset
						close();
						open();
					}
				}
			}
		}
		return false;
	}
	
	/**
	 * @overwrite
	 * */
	virtual OutputBuffer output() {
		ScopeLock scope(m_output_buffer_mutex);
		
		if ( m_output_buffer_count ) {
			OutputBufferInfo* buf = nullptr;
			OutputBufferInfo* unknown_time_frame = nullptr;
			OutputBuffer out;
			
			// sort frame
			for (int i = 0; i < OUTPUT_BUFFER_NUM; i++) {
				OutputBufferInfo* b2 = m_output_buffer + i;
				if ( b2->buffer ) {
					if ( b2->time == Uint64::max ) { //  Unknown time frame
						if ( unknown_time_frame ) {
							m_output_buffer_count--;
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
				if ( m_presentation_time ) {
					int64 diff = buf->time - m_presentation_time - m_frame_interval;
					if ( diff > m_frame_interval / 2 && diff < m_frame_interval * 3 ) {
						if (unknown_time_frame) {
							buf = unknown_time_frame;
							buf->time = m_presentation_time + m_frame_interval;
						}
					}
				}
			} else {
				buf = unknown_time_frame;
				buf->time = m_presentation_time + m_frame_interval;
			}
			
			// yuv420sp
			out.linesize[0] =  m_video_width * m_video_height;
			out.linesize[1] = out.linesize[0] / 2;
			out.data[0] = (byte*)CVPixelBufferGetBaseAddressOfPlane(buf->buffer, 0);  // y
			out.data[1] = (byte*)CVPixelBufferGetBaseAddressOfPlane(buf->buffer, 1);  // uv
			out.time  = m_presentation_time = buf->time;
			out.total = out.linesize[0] + out.linesize[1];
			return out;
		}
		return OutputBuffer();
	}
	
	/**
	 * @overwrite
	 */
	virtual void release(OutputBuffer& buffer) {
		ScopeLock scope(m_output_buffer_mutex);
		
		if ( buffer.total ) {
			int index = buffer.index;
			
			memset(&buffer, 0, sizeof(OutputBuffer));
			if ( m_output_buffer[index].buffer ) {
				OutputBufferInfo* buf = m_output_buffer + index;
				m_output_buffer_count--;
				CVPixelBufferUnlockBaseAddress(buf->buffer, buf->lock);
				CVPixelBufferRelease(buf->buffer);
				buf->buffer = nullptr;
			}
		}
	}
	
	/**
	 * @overwrite
	 */
	virtual void set_frame_size(uint size) {}
	virtual void set_threads(uint value) {}
	virtual void set_background_run(bool value) {}
	
 private:
	VTDecompressionSessionRef m_session;
	CMFormatDescriptionRef    m_format_desc;
	CMSampleBufferRef         m_sample_data;
	uint64                    m_sample_time;
	Mutex                     m_output_buffer_mutex;
	OutputBufferInfo          m_output_buffer[OUTPUT_BUFFER_NUM];
	uint  m_output_buffer_count;
	uint  m_start_decoder;
	uint  m_video_width;
	uint  m_video_height;
	uint64  m_presentation_time;
};

/**
* @func hardware
*/
MediaCodec* MediaCodec::hardware(MediaType type, MultimediaSource* source) {
	Extractor* ex = source->extractor(type);
	if ( ex ) {
		if (type == MEDIA_TYPE_AUDIO) {
			return NULL;
		} else {
			Handle<AppleVideoCodec> codec = new AppleVideoCodec(ex);
			if (codec->initialize()) {
				return codec.collapse();
			}
		}
	}
	return NULL;
}

XX_END
