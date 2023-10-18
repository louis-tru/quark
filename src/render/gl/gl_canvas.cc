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

namespace qk {

	float get_level_font_size(float fontSize);
	GLint gl_get_texture_pixel_format(ColorType type);
	GLint gl_get_texture_data_type(ColorType format);
	void  gl_set_blend_mode(BlendMode blendMode);
	void  gl_setColorRenderBuffer(GLuint buff, ColorType type, Vec2 size, int msaaSample);
	void  gl_setFramebufferRenderbuffer(GLuint b, Vec2 s, GLenum f, GLenum at, int msaa);
	void  gl_setAAClipBuffer(GLuint tex, Vec2 size, int msaaSample);
	void  gl_setBlurRenderBuffer(GLuint tex, Vec2 size, int msaaSample);
	void  gl_textureBarrier();

	extern const Region ZeroRegion;
	extern const float  aa_fuzz_weight = 0.9;
	extern const float  aa_fuzz_width = 0.6;
	extern const float  DepthNextUnit = 0.000000125f; // 1/8000000

	class GLCFilter {
	public:
		typedef NonObjectTraits Traits;
		template<typename... Args>
		static GLCFilter* Make(GLCanvas *host, const Paint &paint, Args... args);
		virtual ~GLCFilter() = default;
	};

	Qk_DEFINE_INLINE_MEMBERS(GLCanvas, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<GLCanvas::Inl*>(self)

		inline void zDepthNext() {
			_zDepth += DepthNextUnit;
		}

		inline void zDepthNextCount(uint32_t count) {
			_zDepth += (DepthNextUnit * count);
		}

		void setMatrixInl(const Mat& mat) {
			auto ch = _state->matrix[0] != 1 || _state->matrix[4] != 1;
			auto scale = ch ? _state->matrix.mul_vec2_no_translate(1).length() / Qk_SQRT_2: 1;
			//auto scale1 = ch ? _state->matrix.mul_vec2_no_translate(Vec2(1,-1)).length() / Qk_SQRT_2: 1;
			//auto scale2 = (scale + scale1) * 0.5; // this scale2 is a more accurate value
			if (_scale != scale) {
				_scale = scale;
				_fullScale = _surfaceScale * scale;
				_phy2Pixel = 2 / _fullScale;
			}
			_cmdPack->setMetrix();
		}

		void setBlendMode(BlendMode mode) {
			if (_blendMode != mode) {
				_blendMode = mode;
				_cmdPack->setBlendMode(mode);
			}
		}

		void clipv(const Path &path, const VertexData &vertex, ClipOp op, bool antiAlias) {
			GLC_State::Clip clip{
				.matrix=_state->matrix, .path=path, .op=op,
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

			if (antiAlias && !_DeviceMsaa) {
				clip.aafuzz = _cache->getAAFuzzStrokeTriangle(path,_phy2Pixel*aa_fuzz_width);
				if (!clip.aafuzz.vertex.val() && path.verbsLen()) {
					clip.aafuzz = path.getAAFuzzStrokeTriangle(_phy2Pixel*aa_fuzz_width);
				}
				_state->aaclip++;
			}

			if (!_isClipState) {
				_isClipState = true;
				_cmdPack->switchState(GL_STENCIL_TEST, true); // enable stencil test
			}

			if (clip.op == kDifference_ClipOp) {
				if (_stencilRefDecr == 0) {
					Qk_WARN(" clip stencil ref decr value exceeds limit 0"); return;
				}
				_stencilRefDecr--;
			} else {
				if (_stencilRef == 255) {
					Qk_WARN(" clip stencil ref value exceeds limit 255"); return;
				}
				_stencilRef++;
			}

			_cmdPack->drawClip(clip, _stencilRef, false);
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
					_cmdPack->drawColor4f(vertex, paint.color, false); break;
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
				auto width = paint.width - _phy2Pixel * 0.45f;
				if (width > 0) {
					fillPath(_cache->getStrokePath(path, width, paint.cap, paint.join,0), paint, true);
				} else {
					width /= (_phy2Pixel * 0.65f); // range: -1 => 0
					width = powf(width*10, 3) * 0.005; // (width*10)^3 * 0.006
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
			// Qk_DEBUG("%p", &vertex);
			switch (paint.type) {
				case Paint::kColor_Type:
					_cmdPack->drawColor4f(vertex, paint.color.to_color4f_alpha(aaFuzzWeight), true); break;
				case Paint::kGradient_Type:
					_cmdPack->drawGradient(vertex, paint.gradient, aaFuzzWeight * paint.color.a(), true); break;
				case Paint::kBitmap_Type:
					_cmdPack->drawImage(vertex, paint.image, aaFuzzWeight * paint.color.a(), true); break;
				case Paint::kBitmapMask_Type:
					_cmdPack->drawImageMask(vertex, paint.image, paint.color.to_color4f_alpha(aaFuzzWeight), true); break;
			}
		}

		float drawTextImage(ImageSource *textImg, float imgTop, float scale, Vec2 origin, const Paint &paint) {
			auto &pix = textImg->pixels().front();
			auto scale_1 = 1.0 / scale;
			ImagePaint p;
			// default use baseline align
			Vec2 dst_start(origin.x(), origin.y() - imgTop * scale_1);
			Vec2 dst_size(pix.width() * scale_1, pix.height() * scale_1);
			Rect rect{dst_start, dst_size};

			Sp<GLCFilter> filter = GLCFilter::Make(this, paint, &rect);

			p.setImage(textImg, rect);

			Vec2 v1(dst_start.x() + dst_size.x(), dst_start.y());
			Vec2 v2(dst_start.x(), dst_start.y() + dst_size.y());
			Vec2 v3(dst_start + dst_size);
			VertexData vertex{0,6,{
				dst_start, v1, v2, // triangle 0
				v2, v3, v1, // triangle 1
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
			: _host(host), _size(paint.filter->value), _bounds(path->getBounds(&host->_state->matrix))
		{
			begin();
		}
		GLCBlurFilter(GLCanvas *host, const Paint &paint, const Rect *rect)
			: _host(host), _size(paint.filter->value), _bounds{rect->origin,rect->origin+rect->size}
		{
			if (!host->_state->matrix.is_unit_matrix()) {
				auto &mat = host->_state->matrix;
				if (mat[0] != 1 || mat[4] != 1) { // rotate or skew
					Vec2 pts[] = {
						_bounds.origin, {_bounds.end.x(),_bounds.origin.y()},
						{_bounds.end.x(),_bounds.end.y()}, {_bounds.origin.x(),_bounds.end.y()}
					};
					_bounds = Path::getBoundsFromPoints(pts, 4, &host->_state->matrix);
				} else {
					_bounds.origin += {mat[2],mat[5]}; // translate
				}
			}
			begin();
		}

		~GLCBlurFilter() override {
			auto ct = _host->_cmdPack->blurFilterEnd(_bounds, _size);
			_inl(_host)->zDepthNextCount(ct);
		}

	private:
		void begin() {
			_size *= _host->_scale;
			_bounds = {_bounds.origin - _size, _bounds.end + _size};
			_host->_cmdPack->blurFilterBegin(_bounds);
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
				return new GLCBlurFilter(host, paint, args...);
			default:
				return nullptr;
		}
	}

	GLCanvas::GLCanvas(GLRender *render, Render::Options opts)
		: _render(render)
		, _stateStack{{ .matrix=Mat(), .aaclip=0 }}
		, _state(&_stateStack.back())
		, _cache(new PathvCache(render))
		, _fbo(0), _rbo(0), _depthBuffer(0), _stencilBuffer(0)
		, _aaclipTex(0)
		, _blurTex(0)
		, _stencilRef(127), _stencilRefDecr(127)
		, _zDepth(0)
		, _surfaceScale(1), _scale(1), _fullScale(1), _phy2Pixel(1)
		, _rootMatrix()
		, _blendMode(kSrcOver_BlendMode)
		, _opts(opts)
		, _isClipState(false)
		, _DeviceMsaa(0)
	{
		// Create the framebuffer and bind it so that future OpenGL ES framebuffer commands are directed to it.
		glGenFramebuffers(1, &_fbo); // _fbo
		// Create a color renderbuffer, allocate storage for it, and attach it to the framebuffer.
		glGenRenderbuffers(2, &_rbo); // _rbo,_depthBuffer

		_opts.doubleCmds = _opts.doubleCmds && Qk_USE_GLC_CMD_QUEUE;
		_cmdPack = new GLC_CmdPack(render, this);
		_cmdPackFront = _opts.doubleCmds ? new GLC_CmdPack(render, this): _cmdPack;
		setMatrix(_state->matrix); // init shader matrix
	}

	GLCanvas::~GLCanvas() {
		delete _cmdPack;
		if (_cmdPack != _cmdPackFront)
			delete _cmdPackFront;
		_cmdPack = _cmdPackFront = nullptr;
		Release(_cache); _cache = nullptr;

		glDeleteFramebuffers(1, &_fbo); // _fbo
		glDeleteRenderbuffers(3, &_rbo); // _rbo,_depthBuffer,_stencilBuffer
		glDeleteTextures(2, &_aaclipTex); // _aaclipTex, _blurTex
	}

	void GLCanvas::swapBuffer() {
		if (_opts.doubleCmds) {
			_mutex.lock();
			auto pack = _cmdPackFront;
			_cmdPackFront = _cmdPack;
			_cmdPack = pack;
			_mutex.unlock();
		}
	}

	void GLCanvas::flushBuffer() { // only can rendering thread call
		_mutex.lock();
		_cmdPackFront->flush(); // commit gl cmd
		_mutex.unlock();
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
			do {
				for (int i = _state->clips.length() - 1; i >= 0; i--) {
					auto &clip = _state->clips[i];
					if (clip.op == kDifference_ClipOp) {
						_stencilRefDecr++;
					} else {
						_stencilRef--;
					}
					_this->setMatrixInl(clip.matrix);
					_cmdPack->drawClip(clip, _stencilRef, true);
					_this->zDepthNext();
				}
				_state = &_stateStack.pop().back();
				count--;
			} while (count > 0);

			if (_isClipState && _stencilRef == 127 && _stencilRefDecr == 127) { // not stencil test
				_isClipState = false;
				_cmdPack->switchState(GL_STENCIL_TEST, false); // disable stencil test
			}
			_this->setMatrixInl(_state->matrix);
		}
	}

	int GLCanvas::getSaveCount() const {
		return _stateStack.length() - 1;
	}

	const Mat& GLCanvas::getMatrix() const {
		return _state->matrix;
	}

	PathvCache* GLCanvas::gtePathvCache() {
		return _cache;
	}

	void GLCanvas::setMatrix(const Mat& mat) {
		_state->matrix = mat;
		_this->setMatrixInl(mat);
	}

	void GLCanvas::translate(float x, float y) {
		_state->matrix.translate({x, y});
		_this->setMatrixInl(_state->matrix);
	}

	void GLCanvas::scale(float x, float y) {
		_state->matrix.scale({x, y});
		_this->setMatrixInl(_state->matrix);
	}

	void GLCanvas::rotate(float z) {
		_state->matrix.rotate(z);
		_this->setMatrixInl(_state->matrix);
	}

	bool GLCanvas::readPixels(uint32_t srcX, uint32_t srcY, Pixel* dst) {
		GLenum format = gl_get_texture_pixel_format(dst->type());
		GLenum type = gl_get_texture_data_type(dst->type());
		if (format && dst->bytes() != dst->body().size())
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
		_cmdPack->clearColor4f(color, {}, true); // clear color/clip/depth
	}

	void GLCanvas::drawColor(const Color4f &color, BlendMode mode) {
		auto isBlend = mode != kSrc_BlendMode;// || color.a() != 1;
		if (isBlend) { // draw color
			_this->setBlendMode(mode); // switch blend mode
			_cmdPack->clearColor4f(color, {{},_size}, false);
			_this->zDepthNext();
		} else { // clear color
			clearColor(color);
		}
	}

	void GLCanvas::drawPathvColor(const Pathv& path, const Color4f &color, BlendMode mode) {
		if (path.vCount) {
			_this->setBlendMode(mode); // switch blend mode
			_cmdPack->drawColor4f(path, color, false);
			if (!_DeviceMsaa) { // Anti-aliasing using software
				auto &vertex = _cache->getAAFuzzStrokeTriangle(path.path, _phy2Pixel*aa_fuzz_width);
				_cmdPack->drawColor4f(vertex, color.to_color4f_alpha(aa_fuzz_weight), true);
			}
			_this->zDepthNext();
		}
	}

	void GLCanvas::drawRRectBlurColor(const Rect& rect,
		const float radius[4], float blur, const Color4f &color, BlendMode mode) {
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

	float GLCanvas::drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, const Array<Vec2> *offset, const Paint &paint)
	{
		_this->setBlendMode(paint.blendMode); // switch blend mode

		Sp<ImageSource> img;
		auto tf = glyphs.typeface();
		auto bound = tf->getImage(glyphs.glyphs(), glyphs.fontSize() * _fullScale, offset, &img);
		img->mark_as_texture(_render);
		auto scale_1 = _this->drawTextImage(*img, bound.y(), _fullScale, origin, paint);
		return scale_1 * bound.x();
	}

	void GLCanvas::drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) {
		_this->setBlendMode(paint.blendMode); // switch blend mode

		fontSize *= _scale;
		auto levelSize = get_level_font_size(fontSize);
		auto levelScale = fontSize / levelSize;
		auto imageFontSize = levelSize * _surfaceScale;

		if (imageFontSize == 0.0)
			return;

		if (blob->imageFontSize != imageFontSize || !blob->image) { // fill text bolb
			auto tf = blob->typeface;
			auto offset = blob->offset.length() == blob->glyphs.length() ? &blob->offset: NULL;
			blob->imageBound = tf->getImage(blob->glyphs,imageFontSize, offset, &blob->image);
			blob->image->mark_as_texture(_render);
		}

		_this->drawTextImage(*blob->image, blob->imageBound.y(), _fullScale * levelScale, origin, paint);
	}

	Sp<ImageSource> GLCanvas::readImage(const Rect &rect, ColorType type) {
		auto o = rect.origin;
		auto w = Float::min(o.x()+rect.size.x(), _size.x()) - o.x(),
				 h = Float::min(o.y()+rect.size.y(), _size.y()) - o.y();
		if (w > 0 && h > 0) {
			Sp<ImageSource> src = new ImageSource({
				int(w*_surfaceScale),int(h*_surfaceScale),type}, _render);
			_cmdPack->readImage(rect, *src);
			Qk_ReturnLocal(src);
		}
		return nullptr;
	}

	// --------------------------------------------------------

	void GLCanvas::onSurfaceReload(const Mat4& root, Vec2 surfaceScale, Vec2 size) {
		_mutex.lock();
		_render->lock();

		// clear state all
		_stateStack = {{ .matrix=Mat(), .aaclip=0 }}; // init state
		_state = &_stateStack.back();
		_stencilRef = _stencilRefDecr = 127;
		_isClipState = false; // clear clip state
		// set surface scale
		_surfaceSize = size;
		_size = size / surfaceScale;
		_surfaceScale = (surfaceScale[0] + surfaceScale[1]) * 0.5;
		_scale = _state->matrix.mul_vec2_no_translate(1).length() / Qk_SQRT_2;
		_fullScale = _surfaceScale * _scale;
		_phy2Pixel = 2 / _fullScale;
		_rootMatrix = root.transpose(); // transpose matrix

		setBuffers(size); // set buffers
		// update shader root matrix and clear all save state
		glBindBuffer(GL_UNIFORM_BUFFER, _render->_rootMatrixBlock);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 16, _rootMatrix.val, GL_STREAM_DRAW);
		if (!_DeviceMsaa && _aaclipTex) { // clear aa clip tex buffer
			float color[] = {1.0f,1.0f,1.0f,1.0f};
			glClearBufferfv(GL_COLOR, 1, color); // clear GL_COLOR_ATTACHMENT1
			gl_textureBarrier(); // ensure clip texture clear can be executed correctly in sequence
		}
		if (_stencilBuffer) {
			glClear(GL_STENCIL_BUFFER_BIT); // clear stencil buffer
			glDisable(GL_STENCIL_TEST); // disable stencil test
		}
		_render->unlock();
		_mutex.unlock();
	}

	void GLCanvas::setBuffers(Vec2 size) {
		auto w = size.x(), h = size.y();
		auto type = _opts.colorType;
		auto msaa = _opts.msaa;

		Qk_ASSERT(w, "Invalid viewport size width");
		Qk_ASSERT(h, "Invalid viewport size height");

		glViewport(0, 0, w, h);
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

		if (msaa > 1) {
			do { // enable multisampling
				gl_setColorRenderBuffer(_rbo, type, size, msaa);
				// Test the framebuffer for completeness.
				if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE )
					break;
				msaa >>= 1;
			} while (msaa > 1);
		}
		_DeviceMsaa = msaa > 1 ? msaa: 0;

		if (!_DeviceMsaa) {
			gl_setColorRenderBuffer(_rbo, type, size, 0);
		}
		if (_aaclipTex) {
			gl_setAAClipBuffer(_aaclipTex, size, _DeviceMsaa);
		}
		if (_stencilBuffer) {
			gl_setFramebufferRenderbuffer(_stencilBuffer, size, GL_STENCIL_INDEX8, GL_STENCIL_ATTACHMENT, _DeviceMsaa);
		}
		if (_blurTex) {
			gl_setBlurRenderBuffer(_blurTex, size, _DeviceMsaa);
		}
		gl_setFramebufferRenderbuffer(_depthBuffer, size, GL_DEPTH_COMPONENT24, GL_DEPTH_ATTACHMENT, _DeviceMsaa);

		const GLenum buffers[]{
			GL_COLOR_ATTACHMENT0/*main color out*/, GL_COLOR_ATTACHMENT1/*aaclip out*/,
		};
		glDrawBuffers(2, buffers);

		if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) {
			Qk_FATAL("failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		}

#if DEBUG
		int width, height;
		// Retrieve the height and width of the color renderbuffer.
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
		Qk_DEBUG("GL_RENDERBUFFER_WIDTH: %d, GL_RENDERBUFFER_HEIGHT: %d", width, height);
#endif
	}

}
