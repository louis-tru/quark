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

#include "./fontdata.h"
#include "../../../util/fs.h"

class QkFileStream: public QkStream {
public:
	QkFileStream(cString& path);
	QkStream* duplicate() const override;
	int read(void* dest, int64_t size, int64_t offset = -1) override;
private:
	FileSync _file;
};

class QkMemoryStream: public QkStream {
public:
	QkMemoryStream(Buffer &buffer);
	cVoid* getMemoryBase() override;
	QkStream* duplicate() const override;
	int read(void* dest, int64_t size, int64_t offset = -1) override;
private:
	struct Data: Reference { Buffer buffer; };
	QkMemoryStream(Data *data); // copy
	Sp<Data> _data;
	uint32_t _offset;
};

QkFileStream::QkFileStream(cString& path): _file(path) {
	if (_file.open() == 0)
		_length = fs_stat_sync(path).size;
}

QkStream* QkFileStream::duplicate() {
	return new QkFileStream(_file.path());
}

int QkFileStream::read(void* dest, int64_t size, int64_t offset) {
	return _file.read(dest, size, offset);
}

QkMemoryStream::QkMemoryStream(Buffer &buffer): _data(new Data), _offset(0) {
	_data->buffer = std::move(buffer);
	_length = _buffer.length();
}

QkMemoryStream::QkMemoryStream(Data *data) {
	_data = data;
	_length = _buffer.length();
}

cVoid* QkMemoryStream::getMemoryBase() {
	return _data->buffer.val();
}

QkStream* QkMemoryStream::duplicate() const {
	return new QkMemoryStream(_data.value());
}

int QkMemoryStream::read(void* dest, int64_t size, int64_t offset) {
	if (offset == -1) {
		offset = _offset;
	}
	if (offset < _length) {
		auto n = _length - offset - size;
		if (i < 0)
			size -= n;
		memcpy(dest, _data->buffer.val() + offset, size);
		_offset += size;
		return size;
	}
	return 0;
}

QkStream* QkStream::Make(cString& path) {
	return new QkFileStream(path);
}

QkStream* QkStream::Make(Buffer buffer) {
	return new QkMemoryStream(buffer);
}
