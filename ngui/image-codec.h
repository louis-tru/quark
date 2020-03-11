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

#ifndef __ngui__image_codec__
#define __ngui__image_codec__

#include "nxkit/handle.h"
#include "nxkit/array.h"
#include "nxkit/buffer.h"

/**
 * @ns ngui
 */

NX_NS(ngui)

class PixelData;
typedef const PixelData cPixelData;

/**
 * @class PixelData
 */
class NX_EXPORT PixelData: public Object {
 public:

	enum Format: uint64 {
		PVRTCI_2BPP_RGB = 0,
		PVRTCI_2BPP_RGBA,
		PVRTCI_4BPP_RGB,
		PVRTCI_4BPP_RGBA,
		PVRTCII_2BPP,
		PVRTCII_4BPP,
		ETC1,
		DXT1,
		DXT2,
		DXT3,
		DXT4,
		DXT5,
		
		//These formats are identical to some DXT formats.
		BC1 = DXT1,
		BC2 = DXT3,
		BC3 = DXT5,
		
		//These are currently unsupported:
		BC4,
		BC5,
		BC6,
		BC7,
		
		//These are supported
		UYVY,
		YUY2,
		YUV420P,
		YUV420SP,
		YUV411P,
		YUV411SP,
		BW1BPP,
		SharedExponentR9G9B9E5,
		RGBG8888,
		GRGB8888,
		ETC2_RGB,
		ETC2_RGBA,
		ETC2_RGB_A1,
		EAC_R11,
		EAC_RG11,
		
		//Invalid value
		NumCompressedPFs,
		
		//
		RGBA8888            = 100001,
		RGBX8888            = 100002,
		RGB888              = 100003,
		RGB565              = 100004,
		RGBA5551            = 100005,
		RGBA4444            = 100006,
		RGBX4444            = 100007,
		ALPHA8              = 100008,
		LUMINANCE8          = 100009,
		LUMINANCE_ALPHA88   = 100010,
		INVALID             = 200000
	};
	
	PixelData();
	PixelData(cPixelData& data);
	PixelData(PixelData&& data);
	PixelData(Format format);
	PixelData(Buffer body, int width, int height,
						Format format, bool is_premultiplied_alpha = false);
	PixelData(WeakBuffer body, int width, int height,
						Format format, bool is_premultiplied_alpha = false);
	PixelData(const Array<WeakBuffer>& body, int width, int height,
						Format format, bool is_premultiplied_alpha = false);
	
	/**
	 * @func body 图像数据主体
	 */
	inline cWeakBuffer& body(uint index = 0) const { return m_body[index]; }
	
	/**
	 * @func body_count
	 * */
	inline uint body_count() const { return m_body.length(); }
	
	/**
	 * @func width 图像宽度
	 */
	inline int width() const { return m_width; };
	
	/**
	 * @func height 图像高度
	 */
	inline int height() const { return m_height; };
	
	/**
	 * @func format 图像像素的排列格式
	 */
	inline Format format() const { return m_format; };
	
	/**
	 * @func is_premultiplied_alpha 图像数据是否对通道信息进行了预先处理,存在alpha通道才有效.
	 */
	inline bool is_premultiplied_alpha() const { return m_is_premultiplied_alpha; };
	
	/**
	 * @func is_compressd_format
	 */
	static bool is_compressd_format(Format format);
	
	/**
	 * @func get_pixel_data_size
	 */
	static uint get_pixel_data_size(Format format);
	
 private:
	
	Buffer              m_data;
	int                 m_width;
	int                 m_height;
	Array<WeakBuffer>   m_body;
	Format              m_format;
	bool                m_is_premultiplied_alpha;
};

/**
 * @class ImageCodec
 */
class NX_EXPORT ImageCodec: public Object {
 public:
	
	enum ImageFormat {
		Unknown = 0,
		TGA,
		JPEG,
		GIF,
		PNG,
		WEBP,
		PVRTC,
	};
	
	/**
	 * 解码图像为GPU可读取的格式如:RGBA8888/RGBA4444/ETC1/ETC2_RGB/ETC2_RGBA...,并返回mipmap列表
	 * @func decode
	 * @arg data {cBuffer&}
	 * @ret {Array<PixelData>}
	 */
	virtual Array<PixelData> decode(cBuffer& data) = 0;
	
	/**
	 * @func decode_header
	 * 只解码头信息,返回除主体数据以外的描述数据 width、height、format、
	 * 如果当前只需要知道图像的附加信息可调用该函数,
	 * 因为解码像 jpg、png 这种复杂压缩图像格式是很耗时间的.
	 */
	virtual PixelData decode_header(cBuffer& data) = 0;
	
	/**
	 * @func encode 编码图像数据
	 */
	virtual Buffer encode(cPixelData& data) = 0;
	
	/**
	 * @func get_image_format 通过路径获取图片类型
	 */
	static ImageFormat get_image_format(cString& path);
	
	/**
	 * @func create # 通过格式创建图像解析器
	 */
	static ImageCodec* shared(ImageFormat format);
	
};

/**
 * @class TGAImageCodec
 */
class NX_EXPORT TGAImageCodec: public ImageCodec {
 public:
	virtual Array<PixelData> decode(cBuffer& data);
	virtual PixelData decode_header(cBuffer& data);
	virtual Buffer encode(cPixelData& data);
	friend class _Inl; class _Inl;
};

/**
 * @class JPEGImageCodec
 */
class NX_EXPORT JPEGImageCodec: public ImageCodec {
 public:
	virtual Array<PixelData> decode(cBuffer& data);
	virtual PixelData decode_header(cBuffer& data);
	virtual Buffer encode(cPixelData& data);
};

/**
 * @class GIFImageCodec
 */
class NX_EXPORT GIFImageCodec: public ImageCodec {
 public:
	virtual Array<PixelData> decode(cBuffer& data);
	virtual PixelData decode_header(cBuffer& data);
	virtual Buffer encode(cPixelData& data);
};

/**
 * @class PNGImageParser
 */
class NX_EXPORT PNGImageCodec: public ImageCodec {
 public:
	virtual Array<PixelData> decode(cBuffer& data);
	virtual PixelData decode_header(cBuffer& data);
	virtual Buffer encode(cPixelData& data);
};

/**
 * @class WEBPImageCodec
 */
class NX_EXPORT WEBPImageCodec: public ImageCodec {
 public:
	virtual Array<PixelData> decode(cBuffer& data);
	virtual PixelData decode_header(cBuffer& data);
	virtual Buffer encode(cPixelData& data);
};

/**
 * 原生GPU压缩图像容器格式,无需解码GPU可直接读取
 * 格式可包含 :
 * PVRTCI_4BPP_RGB/PVRTCI_4BPP_RGBA/PVRTCI_2BPP_RGB/PVRTCI_2BPP_RGBA/PVRTCII_4BPP/PVRTCII_2BPP
 * DXT1/DXT2/DXT3/DXT4/DXT5/ETC1/ETC2_RGB/ETC2_RGBA/ETC2_RGB_A1/ETC2/EAC_R11/EAC_RG11
 * BC4/BC5/UYVY/YUY2/RGBG8888/GRGB8888/BW1BPP...
 * @class PVRTImageParser
 */
class NX_EXPORT PVRTCImageCodec: public ImageCodec {
 public:
	virtual Array<PixelData> decode(cBuffer& data);
	virtual PixelData decode_header(cBuffer& data);
	virtual Buffer encode(cPixelData& data);
	NX_DEFINE_INLINE_CLASS(_Inl);
};

NX_END
#endif
