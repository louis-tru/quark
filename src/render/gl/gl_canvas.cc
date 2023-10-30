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
	void  gl_set_framebuffer_renderbuffer(GLuint b, Vec2 s, GLenum f, GLenum at);
	void  gl_set_color_renderbuffer(GLuint buff, ColorType type, Vec2 size, bool texRbo);
	void  gl_set_aaclip_buffer(GLuint tex, Vec2 size);
	void  gl_set_blur_renderbuffer(GLuint tex, Vec2 size);

	extern const Region ZeroRegion;
	extern const float  aa_fuzz_weight = 0.9;
	extern const float  aa_fuzz_width = 0.6;
	extern const float  DepthNextUnit = 0.000000125f; // 1/8000000

	const GLenum DrawBuffers[]{
		GL_COLOR_ATTACHMENT0/*main color out*/, GL_COLOR_ATTACHMENT1/*aaclip out*/,
	};

	class GLPathvCache: public PathvCache {
	public:
		void clear() {
			clearUnsafe(0/*max limit clear*/);
		}
	};

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
				if (_state->aaclip == 0) {
					//_cmdPack->drawBuffers(2, DrawBuffers); // enable aaclip GL_COLOR_ATTACHMENT1
				}
				_state->aaclip++;
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
					_cmdPack->drawColor(vertex, paint.color.to_color4f_alpha(aaFuzzWeight), true); break;
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
			auto ct = _host->_cmdPack->blurFilterEnd(
				_bounds, _size, _host->_state->output ? *_host->_state->output->dest: nullptr);
			_inl(_host)->zDepthNextCount(ct);
		}

	private:
		void begin() {
			_size *= _host->_scale;
			_bounds = {_bounds.origin - _size, _bounds.end + _size};
			_host->_cmdPack->blurFilterBegin({_bounds.origin - _size, _bounds.end + _size});
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
				if (host->_fullScale * paint.filter->value >= 1.0) {
					return new GLCBlurFilter(host, paint, args...);
				}
				break;
		}
		return nullptr;
	}

	GLCanvas::GLCanvas(GLRender *render, Render::Options opts)
		: _render(render)
		, _cache(nullptr)
		, _fbo(0), _rbo(0), _depthBuffer(0), _stencilBuffer(0)
		, _aaclipTex(0)
		, _blurTex(0)
		, _stencilRef(127), _stencilRefDecr(127)
		, _zDepth(0)
		, _surfaceScale(1), _scale(1), _fullScale(1), _phy2Pixel(1)
		, _rootMatrix()
		, _blendMode(kSrcOver_BlendMode)
		, _opts(opts)
		, _isClipState(false), _isTexRender(true)
	{
		switch(_opts.colorType) {
			case kColor_Type_BGRA_8888:
				_opts.colorType = kColor_Type_RGBA_8888; break;
			case kColor_Type_BGRA_1010102:
				_opts.colorType = kColor_Type_RGBA_1010102; break;
			case kColor_Type_BGR_101010X:
				_opts.colorType = kColor_Type_RGB_101010X; break;
			default: break;
		}
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
		bool is_trbo = _isTexRender;
		GLuint fbo = _fbo,
					rbo0  = _rbo,
					rbo[] = { _depthBuffer, _stencilBuffer },
					tex[] = { _aaclipTex, _blurTex };
		_render->post_message(Cb([fbo,rbo,tex,rbo0,is_trbo](auto &e) {
			if (is_trbo) glDeleteTextures(1, &rbo0);
			else glDeleteRenderbuffers(1, &rbo0);
			glDeleteFramebuffers(1, &fbo);
			glDeleteRenderbuffers(2, rbo);
			glDeleteTextures(2, tex);
		}));

		_mutex.mutex.lock();
		if (_cmdPack != _cmdPackFront)
			delete _cmdPackFront;
		delete _cmdPack;
		_cmdPack = _cmdPackFront = nullptr;
		_mutex.mutex.unlock();

		Release(_cache); _cache = nullptr;
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
			bool restore_output = false;
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

					if (clip.aaclip) {
						_state->aaclip--;
						if (_state->aaclip == 0) {
							//_cmdPack->drawBuffers(1, DrawBuffers);
						}
					}
				}
				if (_state->output) {
					restore_output = true;
					_cmdPack->outputImageEnd(*_state->output->dest, _state->output->isMipmap);
				}
				_state = &_stateStack.pop().back();
				count--;
			} while (count > 0);

			if (_isClipState && _stencilRef == _stencilRefDecr) { // not stencil test
				_isClipState = false;
				_cmdPack->switchState(GL_STENCIL_TEST, false); // disable stencil test
			}
			if (restore_output && _state->output) { // restore region draw
				_cmdPack->outputImageBegin(*_state->output->dest, _state->output->isMipmap);
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
		if (path.vCount) {
			_this->setBlendMode(mode); // switch blend mode
			_cmdPack->drawColor(path, color, false);
			if (!_DeviceMsaa) { // Anti-aliasing using software
				auto &vertex = _cache->getAAFuzzStrokeTriangle(path.path, _phy2Pixel*aa_fuzz_width);
				_cmdPack->drawColor(vertex, color.to_color4f_alpha(aa_fuzz_weight), true);
			}
			_this->zDepthNext();
		}
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

	float GLCanvas::drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, const Array<Vec2> *offset, const Paint &paint)
	{
		_this->setBlendMode(paint.blendMode); // switch blend mode

		Sp<ImageSource> img;
		auto tf = glyphs.typeface();
		auto bound = tf->getImage(glyphs.glyphs(), glyphs.fontSize() * _fullScale, offset, &img);
		img->markAsTexture(_render);
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
			blob->image->markAsTexture(_render);
		}

		_this->drawTextImage(*blob->image, blob->imageBound.y(), _fullScale * levelScale, origin, paint);
	}

	Sp<ImageSource> GLCanvas::readImage(const Rect &src, Vec2 dest, ColorType type, bool isMipmap) {
		auto o = src.origin;
		auto s = Vec2{
			Float::min(o.x()+src.size.x(), _size.x()) - o.x(),
			Float::min(o.y()+src.size.y(), _size.y()) - o.y()
		};
		if (s[0] > 0 && s[1] > 0 && dest[0] > 0 && dest[1] > 0) {
			auto img = new ImageSource({
				int(Qk_MIN(dest.x(),_surfaceSize.x())),int(Qk_MIN(dest.y(),_surfaceSize.y())),type}, _render);
			_cmdPack->readImage({o*_surfaceScale,s*_surfaceScale}, img, isMipmap);
			_this->zDepthNext();
			return img;
		}
		return nullptr;
	}

	Sp<ImageSource> GLCanvas::outputImage(ImageSource* dest, bool isMipmap) {
		if (!dest) {
			dest = new ImageSource({
				int(_surfaceSize[0]),int(_surfaceSize[1]),kColor_Type_RGBA_8888
			}, _render);
		}
		dest->markAsTexture(_render);
		_state->output = new GLC_State::Output{dest,isMipmap};
		_cmdPack->outputImageBegin(dest, isMipmap);
		return dest;
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
		_stencilRef = _stencilRefDecr = 127;
		// set surface scale
		_surfaceSize = surfaceSize;
		_size = surfaceSize / scale;
		_surfaceScale = (scale[0] + scale[1]) * 0.5;
		_scale = _state->matrix.mul_vec2_no_translate(1).length() / Qk_SQRT_2;
		_fullScale = _surfaceScale * _scale;
		_phy2Pixel = 2 / _fullScale;
		_rootMatrix = root.transpose(); // transpose matrix

		_cmdPack->setBuffers(surfaceSize, chSize, _isClipState);// set buffers

		_isClipState = false; // clear clip state
	}

	void GLCanvas::setBuffers() {
		auto size = _surfaceSize;
		auto w = size.x(), h = size.y();
		auto type = _opts.colorType;
		auto isInit = !_fbo;

		if (isInit) {
			// Create the framebuffer and bind it so that future OpenGL ES framebuffer commands are directed to it.
			glGenFramebuffers(1, &_fbo); // _fbo
			// Create a color renderbuffer, allocate storage for it, and attach it to the framebuffer.
			_isTexRender ? glGenTextures(1, &_rbo): glGenRenderbuffers(1, &_rbo);
			// Create depth buffer
			glGenRenderbuffers(1, &_depthBuffer); // _depthBuffer
		}
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
		gl_set_color_renderbuffer(_rbo, type, size, _isTexRender);
		gl_set_framebuffer_renderbuffer(_depthBuffer, size, GL_DEPTH_COMPONENT24, GL_DEPTH_ATTACHMENT);

		if (isInit) {
			float depth = 0;
			Color4f color{0,0,0,0};
			glClearBufferfv(GL_DEPTH, 0, &depth); // depth = 0
			glClearBufferfv(GL_COLOR, 0, color.val); // clear GL_COLOR_ATTACHMENT0
		}
		if (_aaclipTex) {
			gl_set_aaclip_buffer(_aaclipTex, size);
		}
		if (_stencilBuffer) {
			gl_set_framebuffer_renderbuffer(_stencilBuffer, size, GL_STENCIL_INDEX8, GL_STENCIL_ATTACHMENT);
		}
		if (_blurTex) {
			gl_set_blur_renderbuffer(_blurTex, size);
		}

		glDrawBuffers(2, DrawBuffers);

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
