/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, Louis.chu
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

#include "../util/thread.h"
#include "./render.h"
#include "./source.h"
#include "./pathv_cache.h"
#include "./arguments.h"

namespace qk {
	// application run arguments, set by main() and used by RenderBackend and RenderResource.
	const RunArguments *runArguments = nullptr;
	// settings Image Sources internal texture stat, used by Render Resource to create GPU texture.
	void setTexUnsafe_SourceImage(ImageSource* img, const TexStat *tex);

	uint32_t msaaSample(uint32_t n) {
		// n = integerExp(n);
		// n = Qk_Min(n, 9); // max sample count is 9
		// return n > 1 ? n: 1;
		// disable MSAA for now
		return 1;
	}

	Sp<ImageSource> RenderResource::createTexture(Vec2 size, ColorType type, uint8_t flags) {
		auto src = ImageSource::Make(PixelInfo{(int)size.x(), (int)size.y(), type, kPremul_AlphaType}, nullptr);
		src->set_mipmap(flags & kMipmap_TextureFlags);
		auto stat = createTextureStat(size, type, flags);
		setTexUnsafe_SourceImage(src.get(), &stat);
		return src;
	}

	void RenderBackend::Delegate::onRenderBackendReload(Vec2 size) {}
	bool RenderBackend::Delegate::onRenderBackendDisplay() { return false; }

	// setting and use gpu vertex data
	bool RenderBackend::useVertexData(const VertexData::ID *id) {
		if (id) {
			if (id->ptr) {
				return true;
			} else if (id->host->_render) {
				if (id->host->_render->uploadVertexData(const_cast<VertexData::ID*>(id))) {
					Qk_ASSERT_NE(id->ptr, nullptr, "create vertex data failed, gpu buffer id is empty");
					return true;
				}
			}
		}
		return false;
	}

	RenderBackend::RenderBackend(Options opts)
		: _opts(opts)
		, _canvas(nullptr)
		, _delegate(nullptr)
	{
		static Delegate defaultDelegate;
		_delegate = &defaultDelegate;
	}

	void RenderBackend::destroy() {
		Qk_CHECK(_canvas == nullptr);
		_delegate = nullptr;
		// NOTE:
		// This does NOT free the RenderBackend memory.
		// Instances are kept in the resident pool and reused via placement new.
		// After release(), the backend becomes a passive shell: it may still receive
		// post_message() calls, but no rendering work or resource operations are performed.
	}

	// Resident pool for RenderBackend storage blocks.
	// RenderBackend instances are never deleted; memory is kept for the entire
	// process lifetime and reused to avoid use-after-free from delayed async
	// callbacks or weak references.
	static Array<RenderBackend*>* residentPool = nullptr;

	// alloc RenderBackend memory from resident pool or allocate new memory if no reusable block is available
	void* acquireRenderBackendStorage(size_t typeHash, size_t size) {
		// Must be called from the main/UI thread.
		// RenderBackend storage management is not thread-safe and is only expected
		// to occur during window/render initialization.
		check_is_first_loop();

		// Lazily initialize the resident pool.
		if (!residentPool) {
			residentPool = new Array<RenderBackend*>();
		}

		RenderBackend* mem = nullptr;

		// Find a reusable backend of the same concrete type.
		// A backend is considered free when it is not attached to a Canvas.
		for (auto r : *residentPool) {
			if (!r->_canvas && typeid(*r).hash_code() == typeHash) {
				mem = r;
				break;
			}
		}

		// If none found, allocate a new resident memory block.
		if (!mem) {
			mem = (RenderBackend*)Object::operator new(size);
			residentPool->push(mem);
		}

		// Mark as in-use with a temporary non-null canvas flag.
		// The real Canvas will be assigned later in the constructor.
		mem->_canvas = (Canvas*)1;
		return mem;
	}

	// make render backend for each platform
	Render* make_metal_render(Render::Options opts);
	Render* make_vulkan_render(Render::Options opts);
	Render* make_gl_render(Render::Options opts);

	// get shared render resource for each backend
	RenderResource* get_shared_metal_render_resource();
	RenderResource* get_shared_vulkan_render_resource();
	RenderResource* get_shared_gl_render_resource();

	// get shared render resource for each platform
	RenderResource* getSharedRenderResource() {
		RenderResource* r = nullptr;
#if Qk_ENABLE_GL
		static bool useGL = runArguments && runArguments->options.has("gl");
		if (useGL)
			return get_shared_gl_render_resource();
#endif
#if Qk_ENABLE_VULKAN
		if (!r) r = get_shared_vulkan_render_resource();
#endif
#if Qk_ENABLE_METAL
		if (!r) r = get_shared_metal_render_resource();
#endif
#if Qk_ENABLE_GL
		if (!r) r = get_shared_gl_render_resource();
#endif
		return r;
	}

	// make render backend for each platform
	Render* Render::Make(Options opts) {
		Render* r = nullptr;
#if Qk_ENABLE_GL
		if (runArguments && runArguments->options.has("gl"))
			return make_gl_render(opts);
#endif
#if Qk_ENABLE_VULKAN
		if (!r) r = make_vulkan_render(opts);
#endif
#if Qk_ENABLE_METAL
		if (!r) r = make_metal_render(opts);
#endif
#if Qk_ENABLE_GL
		if (!r) r = make_gl_render(opts);
#endif
		if (!r) {
			Qk_DLog("No render backend available for this platform");
		}
		return r;
	}
}
