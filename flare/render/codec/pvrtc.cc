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

#include "./codec.h"

F_NAMESPACE_START

static const uint32_t PVR2TEXTURE_FLAG_TYPE_MASK = 0xff;
static const uint64_t PVR3TEXTURE_PFHIGH_MASK = 0xffffffff00000000ULL;

enum {
	PVR2TexturePixelFormat_PVRTC2BPP_RGBA = 24,
	PVR2TexturePixelFormat_PVRTC4BPP_RGBA,
};

enum {
	PVR3TextureFlag_PremultipliedAlpha	= (1 << 1)	// has premultiplied alpha
};

class PVRTCImageCodec::_Inl: public PVRTCImageCodec {
#define _inl_pvr(self) static_cast<PVRTCImageCodec::_Inl*>(self)
	public:
#pragma pack(push,4)
	typedef struct {
		uint32_t headerLength;
		uint32_t height;
		uint32_t width;
		uint32_t numMipmaps;
		uint32_t flags;
		uint32_t dataLength;
		uint32_t bpp;
		uint32_t bitmaskRed;
		uint32_t bitmaskGreen;
		uint32_t bitmaskBlue;
		uint32_t bitmaskAlpha;
		uint32_t pvrTag;
		uint32_t numSurfs;
	} PVRv2TexHeader;
	
	struct PVRv3TexHeader {
		uint32_t version;
		uint32_t flags;
		uint64_t pixelFormat;
		uint32_t colorSpace;
		uint32_t channelType;
		uint32_t height;
		uint32_t width;
		uint32_t depth;
		uint32_t numberOfSurfaces;
		uint32_t numberOfFaces;
		uint32_t numberOfMipmaps;
		uint32_t metadataLength;
	};
#pragma pack(pop)
	
	/*!***********************************************************************
	 @Function		_getFormatMinDims
	 @Input			pixelFormat	A PVR Pixel Format ID.
	 @Modified		minX			Returns the minimum width.
	 @Modified		minY			Returns the minimum height.
	 @Modified		minZ			Returns the minimum depth.
	 @Description	Gets the minimum dimensions (x,y,z) for a given pixel format.
	 *************************************************************************/
	void m_get_format_min_dims (uint64_t pixelFormat,
															uint& minX, uint& minY, uint& minZ) {
		switch (pixelFormat) {
			case PixelData::DXT1:
			case PixelData::DXT2:
			case PixelData::DXT3:
			case PixelData::DXT4:
			case PixelData::DXT5:
			case PixelData::BC4:
			case PixelData::BC5:
			case PixelData::ETC1:
			case PixelData::ETC2_RGB:
			case PixelData::ETC2_RGBA:
			case PixelData::ETC2_RGB_A1:
			case PixelData::EAC_R11:
			case PixelData::EAC_RG11:
				minX = 4;
				minY = 4;
				minZ = 1;
				break;
			case PixelData::PVRTCI_4BPP_RGB:
			case PixelData::PVRTCI_4BPP_RGBA:
				minX = 8;
				minY = 8;
				minZ = 1;
				break;
			case PixelData::PVRTCI_2BPP_RGB:
			case PixelData::PVRTCI_2BPP_RGBA:
				minX = 16;
				minY = 8;
				minZ = 1;
				break;
			case PixelData::PVRTCII_4BPP:
				minX = 4;
				minY = 4;
				minZ = 1;
				break;
			case PixelData::PVRTCII_2BPP:
				minX = 8;
				minY = 4;
				minZ = 1;
				break;
			case PixelData::UYVY:
			case PixelData::YUY2:
			case PixelData::RGBG8888:
			case PixelData::GRGB8888:
				minX = 2;
				minY = 1;
				minZ = 1;
				break;
			case PixelData::BW1BPP:
				minX = 8;
				minY = 1;
				minZ = 1;
				break;
			default: //Non-compressed formats all return 1.
				minX = 1;
				minY = 1;
				minZ = 1;
				break;
		}
	}
	
	/*!***********************************************************************
	 @Function		_getBitsPerPixel
	 @Input			pixelFormat			A PVR Pixel Format ID.
	 @Return		const uint	Number of bits per pixel.
	 @Description	Returns the number of bits per pixel in a PVR Pixel Format
	 identifier.
	 *************************************************************************/
	uint32_t m_get_bits_per_pixel(uint64_t pixel_format) {
		
		if((pixel_format & PVR3TEXTURE_PFHIGH_MASK) != 0){
			byte* PixelFormatChar = (byte*)&pixel_format;
			return PixelFormatChar[4] + PixelFormatChar[5] + PixelFormatChar[6] + PixelFormatChar[7];
		}
		else{
			switch (pixel_format){
				case PixelData::BW1BPP:
					return 1;
				case PixelData::PVRTCI_2BPP_RGB:
				case PixelData::PVRTCI_2BPP_RGBA:
				case PixelData::PVRTCII_2BPP:
					return 2;
				case PixelData::PVRTCI_4BPP_RGB:
				case PixelData::PVRTCI_4BPP_RGBA:
				case PixelData::PVRTCII_4BPP:
				case PixelData::ETC1:
				case PixelData::EAC_R11:
				case PixelData::ETC2_RGB:
				case PixelData::ETC2_RGB_A1:
				case PixelData::DXT1:
				case PixelData::BC4:
					return 4;
				case PixelData::DXT2:
				case PixelData::DXT3:
				case PixelData::DXT4:
				case PixelData::DXT5:
				case PixelData::BC5:
				case PixelData::EAC_RG11:
				case PixelData::ETC2_RGBA:
					return 8;
				case PixelData::YUY2:
				case PixelData::UYVY:
				case PixelData::RGBG8888:
				case PixelData::GRGB8888:
					return 16;
				case PixelData::SharedExponentR9G9B9E5:
					return 32;
				case PixelData::NumCompressedPFs:
				default:
					return 0;
			}
		}
		return 0;
	}
	
	/*!***********************************************************************
	 @Function		_getTextureDataSize
	 @param[in]		header	Specifies the texture header.
	 @Input			iMipLevel	Specifies a mip level to check, 'PVRTEX_ALLMIPLEVELS'
	 can be passed to get the size of all MIP levels.
	 @Input			bAllSurfs	Size of all surfaces is calculated if true,
	 only a single surface if false.
	 @Input			bAllFaces	Size of all faces is calculated if true,
	 only a single face if false.
	 @Return			uint		Size in BYTES of the specified texture area.
	 @Description	Gets the size in BYTES of the texture, given various input
	 parameters.	User can retrieve the size of either all
	 surfaces or a single surface, all faces or a single face and
	 all MIP-Maps or a single specified MIP level.
	 *************************************************************************/
	uint32_t m_get_texture_data_size (PVRv3TexHeader& header,
																int iMipLevel, bool bAllSurfaces = false,
																bool bAllFaces = false) {
		//The smallest divisible sizes for a pixel format
		uint32_t uiSmallestWidth  = 1;
		uint32_t uiSmallestHeight = 1;
		uint32_t uiSmallestDepth  = 1;
		
		uint64_t PixelFormatPartHigh = header.pixelFormat & PVR3TEXTURE_PFHIGH_MASK;
		
		//If the pixel format is compressed, get the pixel format's minimum dimensions.
		if (PixelFormatPartHigh == 0) {
			m_get_format_min_dims(header.pixelFormat, uiSmallestWidth, uiSmallestHeight, uiSmallestDepth);
		}
		
		//Needs to be 64-bit integer to support 16kx16k and higher sizes.
		uint64_t uiDataSize = 0;
		
		//Get the dimensions of the specified MIP Map level.
		uint32_t uiWidth = F_MAX(1, header.width >> iMipLevel);
		uint32_t uiHeight = F_MAX(1, header.height >> iMipLevel);
		uint32_t uiDepth = F_MAX(1, header.depth >> iMipLevel);
		
		//If pixel format is compressed, the dimensions need to be padded.
		if (PixelFormatPartHigh == 0) {
			uiWidth=uiWidth+( (-1*uiWidth)%uiSmallestWidth);
			uiHeight=uiHeight+( (-1*uiHeight)%uiSmallestHeight);
			uiDepth=uiDepth+( (-1*uiDepth)%uiSmallestDepth);
		}
		
		//Work out the specified MIP Map's data size
		uiDataSize = m_get_bits_per_pixel(header.pixelFormat) * uiWidth * uiHeight * uiDepth;
		
		//The number of faces/surfaces to register the size of.
		uint32_t numfaces = bAllFaces ? header.numberOfFaces : 1;
		uint32_t numsurfs = bAllSurfaces ? header.numberOfSurfaces : 1;
		
		//Multiply the data size by number of faces and surfaces specified, and return.
		return (uint)(uiDataSize / 8) * numsurfs * numfaces;
	}
	
	bool m_is_pvr_v2 (cBuffer& data) {
		const char PVRv2TexIdentifier[4] = { 'P', 'V', 'R', '!' };
		PVRv2TexHeader* header = (PVRv2TexHeader*)*data;
		uint32_t pvrTag = header->pvrTag;
		if (PVRv2TexIdentifier[0] != char((pvrTag >>  0) & 0xff) ||
				PVRv2TexIdentifier[1] != char((pvrTag >>  8) & 0xff) ||
				PVRv2TexIdentifier[2] != char((pvrTag >> 16) & 0xff) ||
				PVRv2TexIdentifier[3] != char((pvrTag >> 24) & 0xff)) {
			return false;
		}
		return true;
	}
	
	bool m_is_pvr_v3 (cBuffer& data) {
		PVRv3TexHeader *header = (PVRv3TexHeader*)*data;
		// validate version
		if (header->version == 55727696 ||
				header->version == 0x50565203) {
			return true;
		}
		return false;
	}
	
	Array<PixelData> m_decode_pvr_v2 (cBuffer& data) {
		
		Array<PixelData> rest;
		
		PVRv2TexHeader* header = (PVRv2TexHeader*)*data;
		
		uint32_t flags = header->flags;
		uint32_t formatFlags = flags & PVR2TEXTURE_FLAG_TYPE_MASK;
		
		if (formatFlags == PVR2TexturePixelFormat_PVRTC2BPP_RGBA ||
				formatFlags == PVR2TexturePixelFormat_PVRTC4BPP_RGBA) {
			
			PixelData::Format format =
				formatFlags == PVR2TexturePixelFormat_PVRTC2BPP_RGBA ?
				PixelData::PVRTCI_2BPP_RGBA : 
				PixelData::PVRTCI_4BPP_RGBA;
			
			uint32_t widthBlocks = 0;
			uint32_t heightBlocks = 0;
			uint32_t width = header->width;
			uint32_t height = header->height;
			uint32_t dataLength = header->dataLength;
			uint32_t dataOffset = 0;
			byte* bytes = ((byte*)*data) + sizeof(PVRv2TexHeader);
			
			// Calculate the data size for each texture level and respect the minimum number of blocks
			while (dataOffset < dataLength) {
				
				if (formatFlags == PVR2TexturePixelFormat_PVRTC4BPP_RGBA) {
					widthBlocks = width / 4;
					heightBlocks = height / 4;
				}
				else {
					widthBlocks = width / 8;
					heightBlocks = height / 4;
				}
				
				// Clamp to minimum number of blocks
				if (widthBlocks < 2) {
					widthBlocks = 2;
				}
				if (heightBlocks < 2) {
					heightBlocks = 2;
				}
				
				uint32_t data_size = widthBlocks * heightBlocks * 8;
				
				char* _data = new char[data_size];
				memcpy(_data, bytes + dataOffset, data_size);
				
				rest.push(PixelData(Buffer(_data, data_size), width, height, format));
				
				dataOffset += data_size;
				
				width = F_MAX(width >> 1, 1);
				height = F_MAX(height >> 1, 1);
			}
		}
		return rest;
	}
	
	Array<PixelData> m_decode_pvr_v3 (cBuffer& data) {
		
		Array<PixelData> rest;
		PVRv3TexHeader* header = (PVRv3TexHeader*)*data;
		
		// parse pixel format
		PixelData::Format pixelFormat = (PixelData::Format)header->pixelFormat;
		
		if (pixelFormat < PixelData::RGBA8888) {
			
			uint32_t width = header->width;
			uint32_t height = header->height;
			uint32_t dataOffset = 0;
			
			// flags
			uint32_t flags = header->flags;
			
			// PVRv3 specifies premultiply alpha in a flag -- should always respect this in PVRv3 files
			bool isPremultipliedAlpha = flags & PVR3TextureFlag_PremultipliedAlpha;
			
			uint32_t dataLen = data.length() - (sizeof(PVRv3TexHeader) + header->metadataLength);
			byte* bytes = ((byte*)*data) + sizeof(PVRv3TexHeader) + header->metadataLength;
			
			uint32_t numberOfMipmaps = header->numberOfMipmaps;
			
			for (uint32_t i = 0; i < numberOfMipmaps; i++) {
				
				uint32_t dataSize = m_get_texture_data_size(*header, i);
				char* new_data = new char[dataSize];
				
				memcpy(new_data, bytes + dataOffset, dataSize);
				
				rest.push(PixelData(Buffer(new_data, dataSize), width,
														height, pixelFormat, isPremultipliedAlpha));
				dataOffset += dataSize;
				
				if (dataOffset > dataLen) {
					LOG("TexurePVR: Invalid lenght");
					return rest;
				}
				width = F_MAX(width >> 1, 1);
				height = F_MAX(height >> 1, 1);
			}
		}
		else {
			LOG("TexurePVR: Invalid lenght");
		}
		
		return rest;
	}
};

Array<PixelData> PVRTCImageCodec::decode(cBuffer& data) {
	if (_inl_pvr(this)->m_is_pvr_v2(data)) {
		return _inl_pvr(this)->m_decode_pvr_v2(data);
	}
	else if (_inl_pvr(this)->m_is_pvr_v3(data)) {
		return _inl_pvr(this)->m_decode_pvr_v3(data);
	}
	F_ERR("TexurePVR: Invalid data");
	return Array<PixelData>();
}

PixelData PVRTCImageCodec::decode_header(cBuffer& data) {
	
	if (_inl_pvr(this)->m_is_pvr_v2(data)) {
		
		_Inl::PVRv2TexHeader* header = (_Inl::PVRv2TexHeader*)*data;
		uint32_t flags = header->flags;
		uint32_t formatFlags = (flags & PVR2TEXTURE_FLAG_TYPE_MASK);
		
		if (formatFlags == PVR2TexturePixelFormat_PVRTC2BPP_RGBA ||
				formatFlags == PVR2TexturePixelFormat_PVRTC4BPP_RGBA) {

			PixelData::Format format =
				formatFlags == PVR2TexturePixelFormat_PVRTC2BPP_RGBA ?
				PixelData::PVRTCI_2BPP_RGBA : PixelData::PVRTCI_4BPP_RGBA;
			
			return PixelData(Buffer(), header->width, header->height, format, false);
		}
	}
	else if (_inl_pvr(this)->m_is_pvr_v3(data)) {
		
		_Inl::PVRv3TexHeader *header = (_Inl::PVRv3TexHeader*)*data;
		
		// flags
		uint32_t flags = header->flags;
		
		// PVRv3 specifies premultiply alpha in a flag -- should always respect this in PVRv3 files
		bool isPremultipliedAlpha = flags & PVR3TextureFlag_PremultipliedAlpha;
		
		// parse pixel format
		PixelData::Format pixelFormat = (PixelData::Format)header->pixelFormat;
		
		if (pixelFormat < PixelData::RGBA8888) {
			return PixelData(Buffer(), header->width, header->height, pixelFormat, isPremultipliedAlpha);
		}
	}
	LOG("TexurePVR: Invalid data");
	return PixelData();
}

Buffer PVRTCImageCodec::encode(cPixelData& data) {
	F_UNIMPLEMENTED();
	return Buffer();
}

F_NAMESPACE_END
