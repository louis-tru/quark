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

namespace qk {
	class RenderSurface; // platform render surface
	class Canvas;

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
		 * Create a texture resource.
		 *
		 * @param pix Source pixel data.
		 * @param levels Number of pixel levels contained in pix.
		 *        If levels is 1 and mipmap is true, mipmaps may be generated automatically.
		 * @param out Output texture state object.
		 * @param mipmap Whether mipmap generation is requested.
		 * @returns true if the request was accepted successfully.
		 *
		 * @note The backend may defer actual GPU upload.
		 *
		 * @thread Rt
		 */
		virtual bool createTexture(cPixel *pix, int levels, TexStat *&out, bool mipmap) = 0;

		/**
		 * Delete a texture resource.
		 *
		 * @param tex Texture state object returned by createTexture().
		 *
		 * @note Actual GPU destruction may be deferred.
		 *
		 * @thread Rt
		 */
		virtual void deleteTexture(TexStat *tex) = 0;
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
			ColorType  colorType;  ///< Preferred framebuffer / surface color type.
			uint16_t   msaaSample; ///< GPU MSAA sample count.
			uint16_t   fps;        ///< If 0, use vSync; otherwise limit to this FPS.
			uint32_t   maxCapacityForPathvCache; ///< Max path vertex cache size, default 128 MB.
			bool       isMipmap;   ///< Whether some color-buffer commands should generate mipmaps.
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
		 * Create backend-local GPU vertex data.
		 *
		 * @param id VertexData instance id to initialize.
		 * @returns true if creation succeeded.
		 *
		 * @note This must be called on the Qk render thread that owns this backend.
		 *       For OpenGL this object may contain context-local state such as VAO.
		 *
		 * @thread Rt
		 */
		virtual bool createVertexData(VertexData::ID *id) = 0;

		/**
		 * Delete backend-local GPU vertex data.
		 *
		 * @param id VertexData instance id created by createVertexData().
		 *
		 * @thread Rt
		 */
		virtual void deleteVertexData(VertexData::ID *id) = 0;

		/**
		 * Return platform render surface object.
		 */
		virtual RenderSurface* surface() = 0;

		// Override Object
		virtual void destroy() override;

		/**
		 * Ensure vertex data is available for GPU rendering.
		 *
		 * This helper performs lazy GPU upload when a render backend is available.
		 * If the GPU buffer already exists, it returns true immediately. Otherwise it
		 * attempts to create backend-local GPU vertex data from the CPU-side vertex
		 * array.
		 *
		 * On successful GPU upload, the CPU-side vertex array may be cleared to save
		 * memory. Software rendering paths should use VertexData::vertex directly and
		 * do not need this function.
		 *
		 * @param id VertexData GPU cache id.
		 * @returns true if GPU vertex data is valid and ready to use.
		 *
		 * @note This function may modify the VertexData::ID and may clear
		 *       id->data->vertex after upload.
		 *
		 * @thread Rt
		 */
		static bool setVertexData(const VertexData::ID *id);

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
