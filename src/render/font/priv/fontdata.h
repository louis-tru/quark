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

//@private head

#ifndef __quark__font__priv__fontdata__
#define __quark__font__priv__fontdata__

#include "./util.h"
#include "./mutex.h"
#include "./arguments.h"

using namespace qk;

class QkStream {
public:
	virtual ~QkStream() = default;
	virtual cVoid* getMemoryBase() { return nullptr; }
	virtual QkStream* duplicate() const = 0;
	virtual int read(void* dest, int64_t size, int64_t offset = -1) = 0;
	inline size_t getLength() const { return _length; }
	static QkStream* Make(cString& path);
	static QkStream* Make(Buffer buffer);
protected:
	size_t _length;
};

class QkFontData {
public:
	/** Makes a copy of the data in 'axis'. */
	QkFontData(Sp<QkStream> stream, int index, const QkFixed* axis, int axisCount)
		: fStream(stream), fIndex(index), fAxisCount(axisCount), fAxis(axisCount)
	{
		for (int i = 0; i < axisCount; ++i) {
			fAxis[i] = axis[i];
		}
	}
	QkFontData(Sp<QkStream> stream, FontArguments args)
		: fStream(stream), fIndex(args.getCollectionIndex())
		, fAxisCount(args.getVariationDesignPosition().coordinateCount)
		, fAxis(args.getVariationDesignPosition().coordinateCount)
	{
		for (int i = 0; i < fAxisCount; ++i) {
			fAxis[i] = QkFloatToFixed(args.getVariationDesignPosition().coordinates[i].value);
		}
	}
	QkFontData(const QkFontData& that)
		: fStream(that.fStream->duplicate())
		, fIndex(that.fIndex)
		, fAxisCount(that.fAxisCount)
		, fAxis(fAxisCount)
	{
		for (int i = 0; i < fAxisCount; ++i) {
			fAxis[i] = that.fAxis[i];
		}
	}
	bool hasStream() const { return fStream.get() != nullptr; }
	Sp<QkStream> detachStream() { return fStream; }
	QkStream* getStream() { return fStream.get(); }
	QkStream const* getStream() const { return fStream.get(); }
	int getIndex() const { return fIndex; }
	int getAxisCount() const { return fAxisCount; }
	const QkFixed* getAxis() const { return fAxis.val(); }

private:
	Sp<QkStream> fStream;
	int fIndex, fAxisCount;
	Array<QkFixed> fAxis;
};

#endif
