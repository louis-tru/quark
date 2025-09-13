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

#include "./gl_canvas.h"
#include "./gl_render.h"
#include "./gl_cmd.h"

#define isMoreSofterAA 0

namespace qk {
	GLenum gl_CheckFramebufferStatus(GLenum target);
	float get_level_font_size(float fontSize);
	GLint gl_get_texture_pixel_format(ColorType type);
	GLint gl_get_texture_data_type(ColorType format);
	void  gl_set_framebuffer_renderbuffer(GLuint b, Vec2 s, GLenum f, GLenum at);
	void  gl_set_color_renderbuffer(GLuint rbo, TexStat *rboTex, ColorType type, Vec2 size);
	void  gl_set_aaclip_buffer(GLuint tex, Vec2 size);
	void  gl_set_tex_renderbuffer(GLuint tex, Vec2 size);
	TexStat* gl_new_tex_stat();
	void setMipmap_SourceImage(ImageSource* img, bool val);

	extern const Region ZeroRegion;
#if isMoreSofterAA
	// Softer:
	extern const float  aa_fuzz_weight = 0.9;
	extern const float  aa_fuzz_width = 0.6;
#else
	// More radical:
	extern const float  aa_fuzz_weight = 1;
	extern const float  aa_fuzz_width = 0.5;
#endif
	extern const float  zDepthNextUnit = 1.0f / 5000000.0f;

	const GLenum DrawBuffers[]{
		GL_COLOR_ATTACHMENT0/*main color out*/, GL_COLOR_ATTACHMENT1/*other out*/,
	};

	class GLPathvCache: public PathvCache {
	public:
		void clear() {
			clearUnsafe(0/*max limit clear*/);
		}
		void afterClear() {
			if (_afterClear) {
				_afterClear->resolve();
				_afterClear.release();
			}
		}
	};

	class GLCFilter {
	public:
		template<typename... Args>
		static GLCFilter* Make(GLCanvas *host, const Paint &paint, Args... args);
		virtual ~GLCFilter() = default;
	};

	Qk_DEFINE_INLINE_MEMBERS(GLCanvas, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<GLCanvas::Inl*>(self)

		inline void zDepthNext() {
			_zDepth += zDepthNextUnit;
		}

		inline void zDepthNextCount(uint32_t count) {
			_zDepth += (zDepthNextUnit * count);
		}

		void setMatrixAndScale(const Mat& mat) {
			union {
				struct { float a,b,c,d; } f;
				struct { uint64_t a,b; } u;
			} constexpr const _ = {.f={1,0,0,1}};
			auto ch = *((uint64_t*)(_state->matrix.val)) != _.u.a ||
								*((uint64_t*)(_state->matrix.val+3)) != _.u.b;
			auto scale = ch ? 
				_state->matrix.mul_vec2_no_translate(1).length() / Qk_SQRT_2: 1;
			if (_scale != scale) {
				_scale = scale;
				_allScale = _surfaceScale * scale;
				_phy2Pixel = 2 / _allScale;
			}
			_cmdPack->setMatrix();
		}

		void setBlendMode(BlendMode mode) {
			if (_blendMode != mode) {
				_blendMode = mode;
				_cmdPack->setBlendMode(mode);
			}
		}

		void clipv(const Path &path, const VertexData &vertex, ClipOp op, bool antiAlias) {
			GLC_State::Clip clip{
				.matrix=_state->matrix, /*.path=path,*/ .op=op,
			};
			if (vertex.vertex.val()) { // copy vertex data
				clip.vertex = vertex;
			} else if (path.verbsLen()) {
				clip.vertex = _cache->getPathTriangles(path);
				if (!clip.vertex.vertex.val()) {
					clip.vertex = path.getTriangles();
				}
			}
			if (clip.vertex.vCount == 0) return;

			if (!_isClipState) {
				_isClipState = true;
				_cmdPack->switchState(GL_STENCIL_TEST, true); // enable stencil test
			}

			if (antiAlias && !_DeviceMsaa) {
				clip.aaclip = true;
				clip.aafuzz = _cache->getAAFuzzStrokeTriangle(path,_phy2Pixel*aa_fuzz_width);
				if (!clip.aafuzz.vertex.val() && path.verbsLen()) {
					clip.aafuzz = path.getAAFuzzStrokeTriangle(_phy2Pixel*aa_fuzz_width);
				}
				_state->aaclip++;
			}

			if (clip.op == kDifference_ClipOp) {
				if (_stencilRefDrop == 0) {
					Qk_Warn(" clip stencil ref drop value exceeds limit 0"); return;
				}
				_stencilRefDrop--;
			} else {
				if (_stencilRef == 255) {
					Qk_Warn(" clip stencil ref value exceeds limit 255"); return;
				}
				_stencilRef++;
			}

			_cmdPack->drawClip(clip, _stencilRef, _state->output.get(), false);
			_state->clips.push(std::move(clip));
			zDepthNext();
		}

		void fillPathv(const Pathv &path, const Paint &paint, bool aa) {
			if (path.vCount) {
				Qk_ASSERT(path.path.isNormalized());
				fillv(path, paint);
				if (aa) {
					drawAAFuzzStroke(path.path, paint, aa_fuzz_weight, aa_fuzz_width);
				}
				zDepthNext();
			}
		}

		void fillPath(const Path &path, const Paint &paint, bool aa) {
			Qk_ASSERT(path.isNormalized());
			auto &vertex = _cache->getPathTriangles(path);
			if (vertex.vCount) {
				fillv(vertex, paint);
				if (aa) {
					drawAAFuzzStroke(path, paint, aa_fuzz_weight, aa_fuzz_width);
				}
			}
			zDepthNext();
		}

		void fillv(const VertexData &vertex, const Paint &paint) {
			switch (paint.type) {
				case Paint::kColor_Type:
					_cmdPack->drawColor(vertex, paint.color, false); break;
				case Paint::kGradient_Type:
					_cmdPack->drawGradient(vertex, paint.gradient, paint.color.a(), false); break;
				case Paint::kBitmap_Type:
					_cmdPack->drawImage(vertex, paint.image, paint.color.a(), false); break;
				case Paint::kBitmapMask_Type:
					_cmdPack->drawImageMask(vertex, paint.image, paint.color, false); break;
			}
		}

		void strokePath(const Path &path, const Paint& paint, bool aa) {
			if (aa) {
#if isMoreSofterAA
				const float weight = 0.45f;
#else
				const float weight = 0.5f;
#endif
				auto width = paint.width - _phy2Pixel * weight;
				if (width > 0) {
					fillPath(_cache->getStrokePath(path, width, paint.cap, paint.join,0), paint, true);
				} else {
					width /= (_phy2Pixel * 1.0f - weight); // range: -1 => 0
					width = powf(width*10, 3) * 0.005; // (width*10)^3 * 0.005
					drawAAFuzzStroke(path, paint, 0.5 / (0.5 - width), 0.5);
					zDepthNext();
				}
			} else {
				fillPath(_cache->getStrokePath(path, paint.width, paint.cap, paint.join,0), paint, false);
			}
		}

		void drawAAFuzzStroke(const Path& path, const Paint& paint, float aaFuzzWeight, float aaFuzzWidth) {
			//Path newPath(path); newPath.transfrom(Mat(1,0,170,0,1,0));
			//auto &vertex = _render->getAAFuzzStrokeTriangle(newPath, _Scale);
			// _phy2Pixel*0.6=1.2/_Scale, 2.4px
			auto &vertex = _cache->getAAFuzzStrokeTriangle(path, _phy2Pixel*aaFuzzWidth);
			// Qk_DLog("%p", &vertex);
			switch (paint.type) {
				case Paint::kColor_Type:
					_cmdPack->drawColor(vertex, paint.color.mul_alpha_only(aaFuzzWeight), true); break;
				case Paint::kGradient_Type:
					_cmdPack->drawGradient(vertex, paint.gradient, aaFuzzWeight * paint.color.a(), true); break;
				case Paint::kBitmap_Type:
					_cmdPack->drawImage(vertex, paint.image, aaFuzzWeight * paint.color.a(), true); break;
				case Paint::kBitmapMask_Type:
					_cmdPack->drawImageMask(vertex, paint.image, paint.color.mul_alpha_only(aaFuzzWeight), true); break;
			}
		}

		float drawTextImage(ImageSource *textImg, float imgTop, float scale, Vec2 origin, const Paint &paint) {
			auto pix = textImg->pixel(0);
			auto scale_1 = 1.0f / scale;
			ImagePaint p;
			// default use baseline align
			Vec2 dst_start(origin.x(),
										 origin.y() - imgTop * scale_1);
			Vec2 dst_size(pix->width() * scale_1,
										pix->height() * scale_1);

			//Qk_DLog("drawTextImage0, Vec2( %f  %f), Vec2(%f %f)", dst_start.x(),
			//	dst_start.y(), dst_size.x(), dst_size.y());

			Rect rect{dst_start, dst_size};

			p.setImage(textImg, rect);
			//Qk_DLog("drawTextImage1, Vec2(%f %f), Vec2(%f %f)",
			//				p.coord.origin.x(), p.coord.origin.y(), p.coord.end.x(), p.coord.end.y());
			p.mipmapMode = ImagePaint::kLinear_MipmapMode;
			p.filterMode = ImagePaint::kLinear_FilterMode;

			Sp<GLCFilter> filter = GLCFilter::Make(this, paint, &rect);

			Vec2 top_right(dst_start.x() + dst_size.x(), dst_start.y()); // top right
			Vec2 left_bottom(dst_start.x(), dst_start.y() + dst_size.y()); // left bottom
			Vec2 right_bottom(dst_start + dst_size); // right bottom
			VertexData vertex{0,6,{
				dst_start,
				top_right, left_bottom, // triangle 0 |/
				top_right,
				right_bottom, left_bottom, // triangle 1 /|
			}};
			// auto &vertex = _cache->getRectPath({dst_start,dst_size});

			_cmdPack->drawImageMask(vertex, &p, paint.color, false);
			zDepthNext();

			return scale_1;
		}

		void drawPathvInl(const Pathv& path, const Paint& paint) {
			bool aa = paint.antiAlias && !_DeviceMsaa; // Anti-aliasing using software
			// gen stroke path and fill path and polygons
			switch (paint.style) {
				case Paint::kFill_Style:
					_this->fillPathv(path, paint, aa); break;
				case Paint::kStrokeAndFill_Style:
					_this->fillPathv(path, paint, aa);
				case Paint::kStroke_Style: {
					_this->strokePath(path.path, paint, aa); break;
				}
			}
		}

	};

	class GLCBlurFilter: public GLCFilter {
	public:
		GLCBlurFilter(GLCanvas *host, const Paint &paint, const Path *path)
			: _host(host), _size(paint.filter->val0), _bounds(path->getBounds(&host->_state->matrix))
		{
			begin();
		}
		GLCBlurFilter(GLCanvas *host, const Paint &paint, const Rect *rect)
			: _host(host), _size(paint.filter->val0), _bounds{rect->origin,rect->origin+rect->size}
		{
			if (!host->_state->matrix.is_unit_matrix()) { // Not unit matrix
				auto &mat = host->_state->matrix;
				if (mat[0] != 1 || mat[4] != 1) { // rotate or skew
					Vec2 pts[] = {
						_bounds.origin, {_bounds.end.x(),_bounds.origin.y()},
						{_bounds.end.x(),_bounds.end.y()}, {_bounds.origin.x(),_bounds.end.y()}
					};
					_bounds = Path::getBoundsFromPoints(pts, 4, &host->_state->matrix);
				} else {
					Vec2 translate(mat[2],mat[5]); // translate
					_bounds.origin += translate;
					_bounds.end += translate;
				}
			}
			begin();
		}

		~GLCBlurFilter() override {
			auto ct = _host->_cmdPack->blurFilterEnd(_bounds, _size, _host->_state->output.get());
			_inl(_host)->zDepthNextCount(ct);
		}

	private:
		void begin() {
			_size *= _host->_scale;
			_bounds = {_bounds.origin - _size, _bounds.end + _size};
			_host->_cmdPack->blurFilterBegin(_bounds, _size);
			_inl(_host)->zDepthNext();
		}
		GLCanvas *_host;
		float  _size; // blur size
		Region _bounds; // bounds for draw path
	};

	template<typename... Args>
	GLCFilter* GLCFilter::Make(GLCanvas *host, const Paint &paint, Args... args)
	{
		if (!paint.filter) {
			_inl(host)->setBlendMode(paint.blendMode); // switch blend mode
			return nullptr;
		}

		switch(paint.filter->type) {
			case PaintFilter::kBlur_Type:
				if (host->_allScale * paint.filter->val0 >= 1.0) {
					return new GLCBlurFilter(host, paint, args...);
				}
				break;
			default: break;
		}
		return nullptr;
	}

	GLCanvas::GLCanvas(GLRender *render, Render::Options opts)
		: _render(render)
		, _cache(nullptr)
		, _fbo(0), _outDepth(0)
		, _outTex(nullptr)
		, _outAAClipTex(0), _outA(0), _outB(0)
		, _stencilRef(127), _stencilRefDrop(127)
		, _zDepth(0)
		, _surfaceScale(1), _scale(1), _allScale(1), _phy2Pixel(1)
		, _rootMatrix()
		, _blendMode(kSrcOver_BlendMode)
		, _opts(opts)
		, _isClipState(false)
	{
		_DeviceMsaa = _opts.msaaSample > 1 ? _opts.msaaSample: 0;
		auto capacity = opts.maxCapacityForPathvCache ?
			opts.maxCapacityForPathvCache: 128000000/*128mb*/;
		_cache = new PathvCache(Uint32::clamp(capacity, 1024000/*1mb*/, 512000000/*512mb*/), render, this);
		_cmdPack = new GLC_CmdPack(render, this);
		_cmdPackFront = Qk_USE_GLC_CMD_QUEUE ? new GLC_CmdPack(render, this): _cmdPack;
		_stateStack.push({ .matrix=Mat(), .aaclip=0 });
		_state = &_stateStack.back();
	}

	GLCanvas::~GLCanvas() {
		GLuint fbo = _fbo,
					rbo[] = { _outDepth },
					tex[] = { _outAAClipTex, _outA, _outB };
		auto outTex = _outTex;
		auto render = _render;
		_render->post_message(Cb([render,fbo,rbo,tex,outTex](auto &e) {
			glDeleteFramebuffers(1, &fbo);
			glDeleteRenderbuffers(1, rbo);
			glDeleteTextures(3, tex);
			if (outTex)
				render->GLRender::deleteTexture(outTex);
		}));

		_mutex.mutex.lock();
		if (_cmdPack != _cmdPackFront)
			delete _cmdPackFront;
		delete _cmdPack;
		_cmdPack = _cmdPackFront = nullptr;
		_mutex.mutex.unlock();

		Releasep(_cache);
	}

	void GLCanvas::swapBuffer() {
#if Qk_USE_GLC_CMD_QUEUE
		_mutex.mutex.lock();
		auto pack = _cmdPackFront;
		_cmdPackFront = _cmdPack;
		_cmdPack = pack;
		static_cast<GLPathvCache*>(_cache)->clear();
		_mutex.mutex.unlock();
#endif
	}

	void GLCanvas::flushBuffer() { // only can rendering thread call
#if Qk_USE_GLC_CMD_QUEUE
		_mutex.mutex.lock();
		_cmdPackFront->flush(); // commit gl cmd
		_mutex.mutex.unlock();
#endif
		static_cast<GLPathvCache*>(_cache)->afterClear();
	}

	int GLCanvas::save() {
		auto &state = _stateStack.back();
		_stateStack.push({ .matrix=state.matrix,.aaclip=state.aaclip });
		_state = &_stateStack.back();
		return _stateStack.length();
	}

	void GLCanvas::restore(uint32_t count) {
		if (!count || _stateStack.length() == 1)
			return;
		count = Uint32::min(count, _stateStack.length() - 1);

		if (count > 0) {
			bool isOutput = false;
			do {
				for (int i = Qk_Minus(_state->clips.length(), 1); i >= 0; i--) {
					auto &clip = _state->clips[i];
					if (clip.op == kDifference_ClipOp) {
						_stencilRefDrop++;
					} else {
						_stencilRef--;
					}
					//_this->setMatrixAndScale(clip.matrix);
					setMatrix(clip.matrix);
					_cmdPack->drawClip(clip, _stencilRef, _state->output.get(), true);
					_this->zDepthNext();

					if (clip.aaclip) {
						_state->aaclip--;
					}
				}
				if (_state->output) {
					isOutput = true;
					_cmdPack->outputImageEnd(_state->output.get());
				}
				_stateStack.pop();
				_state = &_stateStack.back();
				count--;
			} while (count > 0);

			if (_isClipState && _stencilRef == _stencilRefDrop) { // not stencil test
				_isClipState = false;
				_cmdPack->switchState(GL_STENCIL_TEST, false); // disable stencil test
			}
			if (isOutput && _state->output) { // restore region draw
				_cmdPack->outputImageBegin(_state->output.get());
			}
			//_this->setMatrixAndScale(_state->matrix);
			setMatrix(_state->matrix);
		}
	}

	int GLCanvas::getSaveCount() const {
		return _stateStack.length() - 1;
	}

	const Mat& GLCanvas::getMatrix() const {
		return _state->matrix;
	}

	PathvCache* GLCanvas::getPathvCache() {
		return _cache;
	}

	void GLCanvas::setMatrix(const Mat& mat) {
		_state->matrix = mat;
		_this->setMatrixAndScale(mat);
	}

	void GLCanvas::translate(Vec2 val) {
		_state->matrix.translate(val);
		_cmdPack->setMatrix();
	}

	void GLCanvas::setTranslate(Vec2 val) {
		_state->matrix.set_translate(val);
		_cmdPack->setMatrix();
	}

	void GLCanvas::scale(Vec2 val) {
		_state->matrix.scale(val);
		_this->setMatrixAndScale(_state->matrix);
	}

	void GLCanvas::rotate(float z) {
		_state->matrix.rotate(z);
		_cmdPack->setMatrix();
	}

	bool GLCanvas::readPixels(uint32_t srcX, uint32_t srcY, Pixel* dst) {
#if Qk_APPLE
		GLenum format = gl_get_texture_pixel_format(dst->type());
		GLenum type = gl_get_texture_data_type(dst->type());
		if (format && dst->bytes() != dst->body().length())
			return false;
#if Qk_USE_GLC_CMD_QUEUE
		_render->lock();
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
		flushBuffer();
#endif
		// glGenBuffers(1, &readBuffer);
		// glBindBuffer(GL_ARRAY_BUFFER, readBuffer);
		// glBufferData(GL_ARRAY_BUFFER, sizeof(float) * FRAME_WIDTH * FRAME_HEIGHT * 3, NULL, GL_DYNAMIC_DRAW);
		// glBindBuffer(GL_PIXEL_PACK_BUFFER, readBuffer);
		// glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
		glReadPixels(srcX, srcY, dst->width(), dst->height(), format, type, dst->val());
#if Qk_USE_GLC_CMD_QUEUE
		_render->unlock();
#endif
		return true;
#else
		// Disable non-macOS systems temporarily,
		// as the GL context cannot be bing to multiple threads on Linux.
		return false;
#endif
	}

	void GLCanvas::clipPath(const Path& path, ClipOp op, bool antiAlias) {
		_this->clipv(path, _cache->getPathTriangles(path), op, antiAlias);
	}

	void GLCanvas::clipPathv(const Pathv& path, ClipOp op, bool antiAlias) {
		_this->clipv(path.path, path, op, antiAlias);
	}

	void GLCanvas::clipRect(const Rect& rect, ClipOp op, bool antiAlias) {
		auto &path = _cache->getRectPath(rect);
		_this->clipv(path.path, path, op, antiAlias);
	}

	void GLCanvas::clearColor(const Color4f& color) {
		_zDepth = 0; // set z depth state
		_cmdPack->clearColor(color, {}, true); // clear color/clip/depth
	}

	void GLCanvas::drawColor(const Color4f &color, BlendMode mode) {
		auto isBlend = mode != kSrc_BlendMode;// || color.a() != 1;
		if (isBlend) { // draw color
			_this->setBlendMode(mode); // switch blend mode
			_cmdPack->clearColor(color, {{},_size}, false);
			_this->zDepthNext();
		} else { // clear color
			clearColor(color);
		}
	}

	void GLCanvas::drawPathvColor(const Pathv& path, const Color4f &color, BlendMode mode) {
		_this->setBlendMode(mode); // switch blend mode
		_cmdPack->drawColor(path, color, false);
		if (!_DeviceMsaa) { // Anti-aliasing using software
			auto &vertex = _cache->getAAFuzzStrokeTriangle(path.path, _phy2Pixel*aa_fuzz_width);
			_cmdPack->drawColor(vertex, color.mul_alpha_only(aa_fuzz_weight), true);
		}
		_this->zDepthNext();
	}

	void GLCanvas::drawPathvColors(const Pathv* paths[], int count, const Color4f &color,BlendMode mode) {
		_this->setBlendMode(mode); // switch blend mode
		for (int i = 0; i < count; i++) {
			_cmdPack->drawColor(*paths[i], color, false);
		}
		if (!_DeviceMsaa) { // Anti-aliasing using software
			auto c2 = color.mul_alpha_only(aa_fuzz_weight);
			for (int i = 0; i < count; i++) {
				auto &vertex = _cache->getAAFuzzStrokeTriangle(paths[i]->path, _phy2Pixel*aa_fuzz_width);
				_cmdPack->drawColor(vertex, c2, true);
			}
		}
		_this->zDepthNext();
	}

	void GLCanvas::drawRRectBlurColor(const Rect& rect,
		const float radius[4], float blur, const Color4f &color, BlendMode mode) 
	{
		if (!rect.size.is_zero_axis()) {
			_this->setBlendMode(mode); // switch blend mode
			_cmdPack->drawRRectBlurColor(rect, radius, blur, color);
			_this->zDepthNext();
		}
	}

	void GLCanvas::drawRect(const Rect& rect, const Paint& paint) {
		auto &path = _cache->getRectPath(rect);
		Sp<GLCFilter> filter = GLCFilter::Make(this, paint, &path.rect);
		_this->drawPathvInl(path, paint);
	}

	void GLCanvas::drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint) {
		auto &path = _cache->getRRectPath(rect,radius);
		Sp<GLCFilter> filter = GLCFilter::Make(this, paint, &path.rect);
		_this->drawPathvInl(path, paint);
	}

	void GLCanvas::drawPathv(const Pathv& path, const Paint& paint) {
		Sp<GLCFilter> filter = GLCFilter::Make(this, paint, &path.path);
		_this->drawPathvInl(path, paint);
	}

	void GLCanvas::drawPath(const Path &path_, const Paint &paint) {
		bool aa = paint.antiAlias && !_DeviceMsaa; // Anti-aliasing using software
		auto &path = _cache->getNormalizedPath(path_);

		Sp<GLCFilter> filter = GLCFilter::Make(this, paint, &path);

		// gen stroke path and fill path and polygons
		switch (paint.style) {
			case Paint::kFill_Style:
				_this->fillPath(path, paint, aa); break;
			case Paint::kStrokeAndFill_Style:
				_this->fillPath(path, paint, aa);
			case Paint::kStroke_Style: {
				_this->strokePath(path, paint, aa); break;
			}
		}
	}

	float GLCanvas::drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, cArray<Vec2> *offset, const Paint &paint)
	{
		_this->setBlendMode(paint.blendMode); // switch blend mode

		auto tf = glyphs.typeface();
		auto out = tf->getImage(glyphs.glyphs(), glyphs.fontSize() * _allScale, offset, _allScale, _render);
		auto scale = _this->drawTextImage(*out.image, out.top, _allScale, origin, paint);
		return scale * out.right;
	}

	void GLCanvas::drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) {
		_this->setBlendMode(paint.blendMode); // switch blend mode

		auto genSize = get_level_font_size(_scale * fontSize) * _surfaceScale;
		//auto genSize = fontSize * _allScale;

		if (genSize == 0.0)
			return;

		if (blob->out.fontSize != genSize || !blob->out.image) { // fill text bolb
			auto tf = blob->typeface;
			Array<Vec2> *offset = blob->offset.length() > blob->glyphs.length() ? &blob->offset: NULL;
			blob->out = tf->getImage(blob->glyphs, genSize, offset, genSize / fontSize, _render);
			// Qk_DLog("GLCanvas::drawTextBlob origin, %f", origin.y());
		}
		auto img = *blob->out.image;
		if (img->width() && img->height()) {
			Qk_ASSERT(img->count(), "GLCanvas::drawTextBlob img->count()");
			_this->drawTextImage(*blob->out.image, blob->out.top, genSize / fontSize, origin, paint);
		}
	}

	void GLCanvas::drawTriangles(const Triangles& triangles, const Paint &paint) {
		_this->setBlendMode(paint.blendMode); // switch blend mode
		_cmdPack->drawTriangles(triangles, paint.image, paint.color);
		if (triangles.zDepthTotal) {
			_zDepth += triangles.zDepthTotal;
		} else {
			_this->zDepthNext();
		}
	}

	Sp<ImageSource> GLCanvas::readImage(const Rect &src, Vec2 dest, ColorType type, bool isMipmap) {
		auto o = src.origin;
		auto s = Vec2{
			Float32::min(o.x()+src.size.x(), _size.x()) - o.x(),
			Float32::min(o.y()+src.size.y(), _size.y()) - o.y()
		};
		if (s[0] > 0 && s[1] > 0 && dest[0] > 0 && dest[1] > 0) {
			auto img = ImageSource::Make(PixelInfo{
				int(Qk_Min(dest.x(),_surfaceSize.x())),int(Qk_Min(dest.y(),_surfaceSize.y())),type});
			setMipmap_SourceImage(img.get(), isMipmap);
			_cmdPack->readImage({o*_surfaceScale,s*_surfaceScale}, *img);
			_this->zDepthNext();
			return img;
		}
		return nullptr;
	}

	Sp<ImageSource> GLCanvas::outputImage(ImageSource* dest, bool isMipmap) {
		Sp<ImageSource> ret(dest);
		if (!dest) {
			ret = ImageSource::Make(PixelInfo{
				int(_surfaceSize[0]),int(_surfaceSize[1]),kRGBA_8888_ColorType
			});
		}
		_state->output = ret;
		setMipmap_SourceImage(ret.get(), isMipmap);
		_cmdPack->outputImageBegin(*ret);
		Qk_ReturnLocal(ret);
	}

	// --------------------------------------------------------

	void GLCanvas::setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 scale) {
		if (_DeviceMsaa) {
			auto msaa = ceilf(sqrtf(_DeviceMsaa));
			surfaceSize *= msaa;
			scale *= msaa;
		}
		auto chSize = surfaceSize != _surfaceSize;
		// clear all state
		_stateStack.clear();
		_stateStack.push({ .matrix=Mat(), .aaclip=0 });
		_state = &_stateStack.back();
		_stencilRef = _stencilRefDrop = 127;
		// set surface scale
		_surfaceSize = surfaceSize;
		_size = surfaceSize / scale;
		_surfaceScale = (scale[0] + scale[1]) * 0.5;
		_scale = _state->matrix.mul_vec2_no_translate(1).length() / Qk_SQRT_2;
		_allScale = _surfaceScale * _scale;
		_phy2Pixel = 2 / _allScale;
		_rootMatrix = root.transpose(); // transpose matrix

		_cmdPack->setBuffers(surfaceSize, _state->output.get(), chSize); // set buffers
		_zDepth = 0;
		_isClipState = false; // clear clip state
	}

	void GLCanvas::setBuffers(Vec2 size) {
		auto w = size.x(), h = size.y();
		auto type = _opts.colorType;
		auto init = !_fbo;

		if (init) {
			// Create a color renderbuffer of texture
			_outTex = gl_new_tex_stat();
			// Create the framebuffer
			glGenFramebuffers(1, &_fbo);
			// Create depth buffer
			glGenRenderbuffers(1, &_outDepth);
			// gen aaclip buffer tex
			// gl_set_aaclip_buffer(_outAAClipTex, size);
		}
		// Bind framebuffer future OpenGL ES framebuffer commands are directed to it.
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
		// Allocate storage for it, and attach it to the framebuffer.
		gl_set_color_renderbuffer(0, _outTex, type, size);
		gl_set_framebuffer_renderbuffer(_outDepth, size, GL_DEPTH24_STENCIL8, GL_DEPTH_ATTACHMENT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _outDepth);

		Qk_DLog("setBuffers: %f, %f", w, h);

		if (init) {
			//float depth = 0;
			//glClearBufferfv(GL_DEPTH, 0, &depth); // depth = 0
			glClearBufferfi(GL_DEPTH_STENCIL, 0, 0, 127);
			//glClear(GL_STENCIL_BUFFER_BIT); // stencil = 127
			Color4f color{0,0,0,0};
			glClearBufferfv(GL_COLOR, 0, color.val); // clear GL_COLOR_ATTACHMENT0
		}
		if (_outAAClipTex) {
			gl_set_aaclip_buffer(_outAAClipTex, size);
		}
		if (_outA) {
			gl_set_tex_renderbuffer(_outA, size);
		}
		if (_outB) {
			gl_set_tex_renderbuffer(_outB, size);
		}

		glDrawBuffers(1, DrawBuffers);
		gl_CheckFramebufferStatus(GL_FRAMEBUFFER);

#if DEBUG
		int width, height;
		// Retrieve the height and width of the color renderbuffer.
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
		Qk_DLog("GL_RENDERBUFFER_WIDTH: %d, GL_RENDERBUFFER_HEIGHT: %d", width, height);
#endif
	}

	Vec2 GLCanvas::size() {
		return _size;
	}

	bool GLCanvas::isGpu() {
		return true;
	}

	void GLCanvas::lock() {
		_mutex.mutex.lock();
	}

	void GLCanvas::unlock() {
		_mutex.mutex.unlock();
	}
}
