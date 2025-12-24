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

#include "./typeface.h"
#include "./style.h"
#include "./pool.h"
#include "./priv/util.h"
#include "./priv/mutex.h"
#include "../render.h"
#include "../sdf.h"

namespace qk {

	Typeface::Typeface(FontStyle fs): _fontStyle(fs), _unitsPerEm(0), _Mutex(new SharedMutex)
	{
		_metrics.fAscent = 0;
	}
	
	Typeface::~Typeface() {
		Releasep(_Mutex);
	}

	int Typeface::countGlyphs() const {
		return onCountGlyphs();
	}

	int Typeface::countTables() const {
		return onGetTableTags(nullptr);
	}

	int Typeface::getTableTags(FontTableTag tags[]) const {
		return onGetTableTags(tags);
	}

	Buffer Typeface::getTableData(FontTableTag tag) const {
		size_t size = getTableSize(tag);
		Buffer buf = Buffer::alloc((uint32_t)size);
		onGetTableData(tag, 0, size, *buf);
		return buf;
	}

	size_t Typeface::getTableSize(FontTableTag tag) const {
		return onGetTableData(tag, 0, ~0U, nullptr);
	}

	int Typeface::getUnitsPerEm() const {
		if (_unitsPerEm == 0)
			_unitsPerEm = onGetUPEM();
		Qk_ASSERT_NE(0, _unitsPerEm);
		return _unitsPerEm;
	}

	String Typeface::getFamilyName() const {
		return onGetFamilyName();
	}

	bool Typeface::getPostScriptName(String* name) const {
		return onGetPostScriptName(name);
	}

	void Typeface::unicharsToGlyphs(const Unichar unichar[], uint32_t count, GlyphID glyphs[]) const {
		//Qk_DLog("Typeface::unicharsToGlyphs, %s", *getFamilyName());
		onCharsToGlyphs(unichar, count, glyphs);
	}

	Array<GlyphID> Typeface::unicharsToGlyphs(cArray<Unichar>& unichar) const {
		//Qk_DLog("Typeface::unicharsToGlyphs, %s", *getFamilyName());
		if (unichar.length() > 0) {
			Array<GlyphID> result(unichar.length());
			onCharsToGlyphs(*unichar, unichar.length(), *result);
			Qk_ReturnLocal(result);
		}
		return Array<GlyphID>();
	}

	GlyphID Typeface::unicharToGlyph(Unichar unichar) const {
		GlyphID id = 0;
		onCharsToGlyphs(&unichar, 1, &id);
		return id;
	}

	const Path& Typeface::getPath(GlyphID glyph) {
		{
			AutoSharedMutexShared ama(mutex());
			auto it = _pathsCache.find(glyph);
			if (it != _pathsCache.end())
				return it->second;
		}
		{
			AutoSharedMutexExclusive asme(mutex());
			Path path;
			onGetPath(glyph, &path);
			_pathsCache.set(glyph, path.normalizedPath());
			return _pathsCache[glyph];
		}
	}

	const FontGlyphMetrics& Typeface::getGlyphMetrics(GlyphID glyph) {
		{
			AutoSharedMutexShared ama(mutex());
			auto it = _glyphsCache.find(glyph);
			if (it != _glyphsCache.end()) {
				return it->second;
			}
		}
		{
			AutoSharedMutexExclusive asme(mutex());
			FontGlyphMetrics fgm;
			onGetGlyphMetrics(glyph, &fgm);
			return _glyphsCache.set(glyph, fgm);
		}
	}

	Array<FontGlyphMetrics> Typeface::getGlyphsMetrics(cArray<GlyphID>& glyphs) {
		Array<FontGlyphMetrics> result;
		{
			AutoSharedMutexShared ama(mutex());
			for (auto gid: glyphs) {
				auto it = _glyphsCache.find(gid);
				if (it == _glyphsCache.end())
					goto rest;
				result.push(it->second);
			}
			Qk_ReturnLocal(result);
		}

	 rest:
		AutoSharedMutexExclusive asme(mutex());

		for (int i = result.length(), len = glyphs.length(); i < len; i++) {
			auto gid = glyphs[i];
			FontGlyphMetrics fgm;
			if (!_glyphsCache.get(gid, fgm)) {
				onGetGlyphMetrics(gid, &fgm);
				_glyphsCache.set(gid, fgm);
			}
			result.push(fgm);
		}
		Qk_ReturnLocal(result);
	}

	float Typeface::getMetrics(FontMetrics* metrics, float fontSize) {
		if (!metrics) return 0;

		getMetrics((FontMetricsBase*)0, 0); // init _metrics

		memcpy(metrics, &_metrics, sizeof(FontMetrics));

		float scale = fontSize / 64.0f;

		// scale font metrics
		metrics->fTop *= scale;
		metrics->fAscent *= scale;
		metrics->fDescent *= scale;
		metrics->fBottom *= scale;
		metrics->fLeading *= scale;
		metrics->fAvgCharWidth *= scale;
		metrics->fMaxCharWidth *= scale;
		metrics->fXMin *= scale;
		metrics->fXMax *= scale;
		metrics->fXHeight *= scale;
		metrics->fCapHeight *= scale;
		metrics->fUnderlineThickness *= scale;
		metrics->fUnderlinePosition *= scale;
		metrics->fStrikeoutThickness *= scale;
		metrics->fStrikeoutPosition *= scale;

		return metrics->fDescent - metrics->fAscent + metrics->fLeading;
	}

	static float computeMetricsBase(FontMetrics *ref, FontMetricsBase* out, float fontSize) {
		if (!out)
			return 0;
		float scale = fontSize / 64.0f;

		out->fAscent = ref->fAscent * scale;
		out->fDescent = ref->fDescent * scale;
		out->fLeading = ref->fLeading * scale;

		return out->fDescent - out->fAscent + out->fLeading;
	}

	float Typeface::getMetrics(FontMetricsBase* metrics, float fontSize) {
		if (_metrics.fAscent == 0) {
			AutoSharedMutexExclusive asme(mutex());
			if (_metrics.fAscent == 0) {
				FontMetrics metrics;
				onGetMetrics(&metrics);
				_metrics = metrics;
			}
		}
		return computeMetricsBase(&_metrics, metrics, fontSize);
	}

	float FontPool::getUnitMetrics(FontMetricsBase* out, float fontSize) const {
		return computeMetricsBase(&_UnitMetrics64, out, fontSize);
	}

	Typeface::TextImage Typeface::getImage(cArray<GlyphID>& glyphs, float fontSize,
			cArray<Vec2> *offset, RenderBackend *render)
	{
		if (offset) {
			Qk_ASSERT(offset->length() >= glyphs.length());
		}
		if (glyphs.length() == 0) {
			return {ImageSource::Make(PixelInfo())};
		}
		return onGetImage(glyphs, fontSize, offset, 0.1, true, render);
	}

	Typeface::TextImage Typeface::getSDFImage(cArray<GlyphID> &glyphs,
		float fontSize, cArray<Vec2> *offset, bool is_signed, RenderBackend *render) {
		if (offset) {
			Qk_ASSERT(offset->length() >= glyphs.length());
		}
		if (glyphs.length() == 0) {
			return {ImageSource::Make(PixelInfo())};
		}
		auto out = onGetImage(glyphs, fontSize, offset, 0.4, true, nullptr);
		auto w = out.image->width();
		auto h = out.image->height();

		switch(out.image->type()) {
			case kAlpha_8_ColorType:
				out.image = ImageSource::Make(compute_distance_f32(
						out.image->pixel(0)->val(), w, h, 1, is_signed), render);
				break;
			case kRGBA_8888_ColorType: {
				auto val = out.image->pixel(0)->val();
				out.image = ImageSource::Make(compute_distance_f32(val + 3, w, h, 4, is_signed), render);
				break;
			}
			default: break;
		}
		Qk_ReturnLocal(out);
	}

}
