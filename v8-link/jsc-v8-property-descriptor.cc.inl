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


// --- P r o p e r t y   D e s c r i p t o r ---

struct v8::PropertyDescriptor::PrivateData {
	inline PrivateData()
	: get(nullptr), set(nullptr)
	, value(nullptr)
	, enumerable(false)
	, configurable(false), writable(false) { }
	JSValueRef get;
	JSValueRef set;
	JSValueRef value;
	bool enumerable;
	bool configurable;
	bool writable;
};

v8::PropertyDescriptor::PropertyDescriptor() : private_(new PrivateData()) {}

// DataDescriptor
v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> value)
: private_(new PrivateData()) {
	private_->value = i::Back(value);
}

// DataDescriptor with writable field
v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> value,
																					 bool writable)
: private_(new PrivateData()) {
	private_->value = i::Back(value);
	private_->writable = writable;
}

// AccessorDescriptor
v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> get,
																					 v8::Local<v8::Value> set)
: private_(new PrivateData()) {
	private_->get = i::Back(get);
	private_->set = i::Back(set);
}

v8::PropertyDescriptor::~PropertyDescriptor() {
	delete private_;
}

v8::Local<Value> v8::PropertyDescriptor::value() const {
	return i::Cast(private_->value);
}

v8::Local<Value> v8::PropertyDescriptor::get() const {
	return i::Cast(private_->get);
}

v8::Local<Value> v8::PropertyDescriptor::set() const {
	return i::Cast(private_->set);
}

bool v8::PropertyDescriptor::has_value() const {
	return private_->value;
}

bool v8::PropertyDescriptor::has_get() const {
	return private_->get;
}

bool v8::PropertyDescriptor::has_set() const {
	return private_->set;
}

bool v8::PropertyDescriptor::writable() const {
	return private_->writable;
}

bool v8::PropertyDescriptor::has_writable() const {
	return private_->writable;
}

void v8::PropertyDescriptor::set_enumerable(bool enumerable) {
	private_->enumerable = enumerable;
}

bool v8::PropertyDescriptor::enumerable() const {
	return private_->enumerable;
}

bool v8::PropertyDescriptor::has_enumerable() const {
	return private_->enumerable;
}

void v8::PropertyDescriptor::set_configurable(bool configurable) {
	private_->configurable = configurable;
}

bool v8::PropertyDescriptor::configurable() const {
	return private_->configurable;
}

bool v8::PropertyDescriptor::has_configurable() const {
	return private_->configurable;
}

