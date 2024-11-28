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
		: _constructor(constructor), _attachConstructor(attach) {}

	void JSClass::exports(cString& name, JSObject* exports) {
		_func.reset(); // reset func
		exports->setProperty(_worker, name, getFunction());
	}

	JSObject* JSClass::newInstance(uint32_t argc, JSValue* argv[]) {
		auto f = getFunction();
		Qk_Assert( f );
		return f->newInstance(_worker, argc, argv);
	}

	JsClasses::JsClasses(Worker* worker)
		: _worker(worker), _isAttachFlag(false)
	{
	}

	JsClasses::~JsClasses() {
		for ( auto i : _jsclass )
			delete i.value;
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
		Qk_Check( ! _jsclass.has(alias), "Set native Constructors Alias repeat");
		cls->_alias = alias;
		_jsclass.set(alias, cls);
	}

	MixObject* JsClasses::attachObject(uint64_t alias, Object* object) {
		auto mix = reinterpret_cast<MixObject*>(object) - 1;
		Qk_Assert( !mix->worker() );
		JSClass *out;
		if ( _jsclass.get(alias, out) ) {
			_isAttachFlag = true;
			auto jsobj = out->getFunction()->newInstance(_worker);
			_isAttachFlag = false;
			out->_attachConstructor(mix);
			return mix->attach(_worker, jsobj);
		}
		return nullptr;
	}

	bool JsClasses::instanceOf(JSValue* val, uint64_t alias) {
		JSClass *out;
		if ( _jsclass.get(alias, out) )
			return out->hasInstance(val);
		return false;
	}

} }
