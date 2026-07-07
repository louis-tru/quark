/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark__render_render__
#define __quark__render_render__

#include "../util/thread.h"
#include "./path.h"
#include "./pixel.h"
#include "src/util/object.h"

namespace qk {
	class RenderSurface; // platform render surface
	class Canvas;
	struct TexStat; // texture state

	/**
	 * @enum TextureFlags
	 * Flags used to specify texture properties and usage.
	 */
	enum TextureFlags: uint8_t {
		kNone_TextureFlags = 0,
		kMipmap_TextureFlags = 1 << 0,
		kComputeWrite_TextureFlags = 1 << 1,
	};

	/**
	 * @class RenderResource
	 * GPU resource management interface.
	 *
	 * @note These operations must be called on the Qk render thread that owns the
	 *       RenderBackend instance (typically one per window).
	 *
	 * @thread Rt
	 */
	class RenderResource: public PostMessage {
	public:

		/**
		 * Upload texture data to backend-local GPU resources.
		 *
		 * @param pix Source pixel data.
		 * @param levels Number of pixel levels contained in pix.
		 *        If levels is 1 and mipmap is true, mipmaps may be generated automatically.
		 * @param tex Texture state container provided and owned by the caller.
		 * @param mipmap Whether mipmap generation is requested.
		 * @returns true if upload/creation succeeded.
		 *
		 * @note This method uploads or initializes backend-local GPU texture resources
		 *       using the pixel data provided by pix.
		 *
		 *       The TexStat container itself is provided and owned by the caller.
		 *       Backend-local texture handles or pointers are stored inside tex.
		 *
		 *       Backend implementations may defer actual GPU upload.
		 *
		 * @thread Rt
		 */
		virtual bool uploadTexture(cPixel *pix, int levels, TexStat *tex, bool mipmap) = 0;

		/**
		 * Release backend-local GPU texture resources.
		 *
		 * @param tex Texture state container previously uploaded by uploadTexture().
		 *
		 * @note This method releases backend-local GPU texture resources stored in tex,
		 *       then clears the backend id/pointer fields inside the container.
		 *
		 *       The TexStat container itself is owned by the caller
		 *       and is not deleted by this method.
		 *
		 *       Actual GPU destruction may be deferred.
		 *
		 * @thread Rt
		 */
		virtual void unloadTexture(TexStat *tex) = 0;

		/**
		 * Create a GPU texture and return a reference-counted ImageSource wrapper.
		 * @param flags {TextureFlags} to specify texture properties and usage.
				- kMipmap_TextureFlags: whether mipmap levels should be generated for this texture.
				- kComputeWrite_TextureFlags: whether the texture will be written by compute shaders, 
					which may require special usage flags or memory properties on some platforms.
		 */
		Sp<ImageSource> createTexture(Vec2 size, ColorType type, uint8_t flags);

	protected:
		/**
		 * Create a GPU texture and return its backend-local state.
		*/
		virtual TexStat createTextureStat(Vec2 size, ColorType type, uint8_t flags) = 0;
	};

	/**
	 * @class RenderBackend
	 * GPU rendering backend.
	 *
	 * RenderBackend is the central GPU backend object for a Qk render surface.
	 * It manages the platform render surface, the render-thread scheduling loop,
	 * backend messages, Canvas objects, and GPU resources.
	 *
	 * A RenderBackend usually belongs to one Qk render thread and one render
	 * surface, typically one per window. Canvas commands and backend messages are
	 * eventually consumed by this backend on its render thread.
	 *
	 * The backend also provides the extension point for lower-level GPU drawing
	 * APIs, such as custom shaders, pipelines, buffers, or other backend-specific
	 * GPU capabilities.
	 *
	 * @thread Rt
	 */
	class Qk_EXPORT RenderBackend: public Object, public RenderResource {
	public:
		struct Options {
			ColorType colorType = kInvalid_ColorType; ///< Preferred framebuffer / surface color type.
			uint32_t  maxCapacityForPathvCache = 0; ///< Max path vertex cache size, default 128 MB.
			bool      mipmap = false; ///< Whether some color-buffer commands should generate mipmaps.
			bool      enableCAPA = true; ///< Whether to enable CAPA for GPU rendering.
			bool      enableCAPAQuantizeCoverage = false; ///< Whether to enable CAPA quantized coverage for GPU rendering.
		};

		class Delegate {
		public:
			/**
			 * Called after the render backend surface has been reloaded.
			 *
			 * @param size New surface size.
			 */
			virtual void onRenderBackendReload(Vec2 size) = 0;

			/**
			 * Called when the backend is ready to display one frame.
			 *
			 * @returns true if display should continue.
			 */
			virtual bool onRenderBackendDisplay() = 0;
		};

		/**
		 * Create a new render backend object.
		 */
		static RenderBackend* Make(Options opts, Delegate *delegate);

		/**
		 * Return backend options.
		 */
		const Options& options() const { return _opts; }

		/**
		 * Reload the render surface.
		 *
		 * Usually called when the native surface, size, scale, or swapchain changes.
		 * Implementations should update _surfaceSize and notify the delegate.
		 */
		virtual void reload() = 0;

		/**
		 * Return current render surface size.
		 */
		inline Vec2 surfaceSize() { return _surfaceSize; }

		/**
		 * Return the default main canvas.
		 */
		inline Canvas* getCanvas() { return _canvas; }

		/**
		 * Create a sub canvas object.
		 *
		 * This method has no Qk render-thread restriction. The implementation performs
		 * internal mutex protection during creation.
		 *
		 * @note A Canvas object, including the default Canvas returned by getCanvas(),
		 *       may be used from non-render threads. Rendering commands are exchanged
		 *       through Canvas::swapBuffer(), which is internally protected by a mutex.
		 *
		 * @note If the same Canvas is used by multiple user threads, the caller must
		 *       provide external synchronization for Canvas command recording.
		 */
		virtual Canvas* createCanvas(Options opts) = 0;

		/**
		 * Whether the backend has already released its canvas/resources.
		 */
		inline bool isReleased() const { return !_canvas; }

		/**
		 * Return platform render surface object.
		 */
		virtual RenderSurface* surface() = 0;

		// Override Object
		virtual void destroy() override;

		/**
		 * Upload vertex data to backend-local GPU.
		 *
		 * @param id VertexData container provided and owned by the caller.
		 * @returns true if upload/creation succeeded.
		 *
		 * @note This method uploads or initializes backend-local GPU vertex resources
		 *       using the data stored in id.
		 *
		 *       The VertexData::ID container itself is provided and owned by the caller.
		 *       Backend-local handles or pointers are stored inside id.
		 *
		 *       Backend implementations may create internal objects such as:
		 *         - OpenGL VBO / VAO
		 *         - Metal buffers
		 *         - Vulkan vertex buffers
		 *
		 * @thread Rt
		 */
		virtual bool uploadVertexData(VertexData::ID *id) = 0;

		/**
		 * Release backend-local GPU vertex.
		 *
		 * @param id VertexData container previously uploaded by uploadVertexData().
		 *
		 * @note This method releases backend-local GPU vertex resources stored in id,
		 *       then clears the backend id/pointer fields inside the container.
		 *
		 *       The VertexData::ID container itself is owned by the caller
		 *       and is not deleted by this method.
		 *
		 * @thread Rt
		 */
		virtual void unloadVertexData(VertexData::ID *id) = 0;

		/**
		 * Ensure vertex data is uploaded and available for GPU rendering.
		 *
		 * This helper performs lazy GPU upload when a render backend is available.
		 * If backend-local GPU vertex resources already exist, it returns true immediately.
		 * Otherwise it attempts to upload/create backend-local GPU vertex resources
		 * from the CPU-side vertex array.
		 *
		 * On successful GPU upload, the CPU-side vertex array may optionally be cleared
		 * to save memory. Software rendering paths should use VertexData::vertex directly
		 * and do not need this function.
		 *
		 * @param id VertexData GPU cache id.
		 * @returns true if backend-local GPU vertex resources are valid and ready to use.
		 *
		 * @note This function may modify VertexData::ID and may clear
		 *       id->data->vertex after successful upload.
		 *
		 * @thread Rt
		 */
		static bool useVertexData(const VertexData::ID *id);

	protected:
		RenderBackend(Options opts);

		/**
		 * Return current platform surface size.
		 *
		 * Used by reload() to query the native surface size and scale-dependent
		 * dimensions.
		 */
		virtual Vec2 getSurfaceSize() = 0;

		// Backend properties.
		Options   _opts;
		Canvas    *_canvas;      ///< Default main canvas.
		Delegate  *_delegate;    ///< Render backend event delegate.
		Vec2      _surfaceSize;  ///< Current surface size.

		friend void* acquireRenderBackendStorage(size_t typeHash, size_t size);
	};

	typedef RenderBackend Render;
}
#endif
