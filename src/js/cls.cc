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

#include "./js_.h"

namespace qk { namespace js {

	// --------------------------- J S . C l a s s ---------------------------

	JSClass::JSClass(FunctionCallback constructor, AttachCallback attach)
		: _alias(0), _constructor(constructor), _attachConstructor(attach) {}

	void JSClass::exports(cString& name, JSObject* exports) {
		exports->setFor(_worker, name, getFunction());
	}

	JSObject* JSClass::newInstance(uint32_t argc, JSValue* argv[]) {
		auto f = getFunction();
		Qk_ASSERT( f );
		return f->newInstance(_worker, argc, argv);
	}

	void JSClass::callConstructor(FunctionArgs args) {
		auto classes = _worker->classes();
		auto mix = classes->_attachObject;
		if (mix) {
			_attachConstructor(mix);
		} else {
			Qk_ASSERT_EQ(classes->_runClass, nullptr);
			classes->_runClass = this;
			_constructor(args);
			classes->_runClass = nullptr;
		}
	}

	JsClasses::JsClasses(Worker* worker)
		: _worker(worker), _attachObject(nullptr), _runClass(nullptr)
	{}

	JsClasses::~JsClasses() {
		for ( auto i : _jsclass ) {
			i.second->destroy();
		}
	}

	JSClass* JsClasses::get(uint64_t alias) {
		JSClass *out = nullptr;
		_jsclass.get(alias, out);
		return out;
	}

	JSFunction* JsClasses::getFunction(uint64_t alias) {
		JSClass *out;
		if ( _jsclass.get(alias, out) ) {
			return out->getFunction();
		}
		return nullptr;
	}

	void JsClasses::add(uint64_t alias, JSClass *cls) throw(Error) {
		Qk_IfThrow( !_jsclass.has(alias), "Set native Constructors Alias repeat");
		cls->_alias = alias;
		_jsclass.set(alias, cls);
	}

	bool JsClasses::instanceOf(JSValue* val, uint64_t alias) {
		JSClass *out;
		if ( _jsclass.get(alias, out) )
			return out->hasInstance(val);
		return false;
	}

} }
