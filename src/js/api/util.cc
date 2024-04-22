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

#include "../../util/string.h"
#include "../../util/fs.h"
#include "../../util/os.h"
#include "../../util/loop.h"
#include "./_view.h"
//#include "quark/util/jsx.h"
#include <native-ext-js.h>

/**
 * @ns qk::js
 */

Js_BEGIN

using namespace native_js;

extern Array<Char*>* __quark_js_argv;
extern int           __quark_js_have_debug;

typedef Object NativeObject;

class WrapNativeObject: public WrapObject {
	public:
	static void constructor(FunctionCall args) {
		Js_ATTACH(args);
		New<WrapNativeObject>(args, new NativeObject());
	}
	static void binding(Local<JSObject> exports, Worker* worker) {
		Js_Define_Class_NO_EXPORTS(NativeObject, constructor, {
			// none
		}, nullptr);
	}
};

/**
 * @class WrapSimpleHash
 */
class WrapSimpleHash: public WrapObject {
	public:
	
	static void constructor(FunctionCall args) {
		New<WrapSimpleHash>(args, new SimpleHash());
	}
	
	static void hashCode(FunctionCall args) {
		Js_Worker(args);
		Js_Self(SimpleHash);
		Js_Return( self->hashCode() );
	}
	
	static void update(FunctionCall args) {
		Js_Worker(args);
		if (  args.Length() < 1 ||
				!(args[0]->IsString(worker) || args[0]->IsBuffer(worker))
		) {
			Js_Throw("Bad argument");
		}
		Js_Self(SimpleHash);
		
		if ( args[0]->IsString(worker) ) { // 字符串
			String2 str = args[0]->ToString2Value(worker);
			self->update(*str, str.length());
		}
		else { // Buffer
			WeakBuffer buff = args[0]->AsBuffer(worker);
			self->update(*buff, buff.length());
		}
	}
	
	static void digest(FunctionCall args) {
		Js_Worker(args);
		Js_Self(SimpleHash);
		Js_Return( self->digest() );
	}
	
	static void clear(FunctionCall args) {
		Js_Self(SimpleHash);
		self->clear();
	}

	/**
	 * @func binding
	 */
	static void binding(Local<JSObject> exports, Worker* worker) {
		Js_Define_Class(SimpleHash, constructor, {
			Js_Set_Class_Method(hashCode, hashCode);
			Js_Set_Class_Method(update, update);
			Js_Set_Class_Method(digest, digest);
			Js_Set_Class_Method(clear, clear);
		}, nullptr);
	}
};

/**
 * @class NativeUtil
 */
class NativeUtil {
	public:

	static SimpleHash get_hashCode(FunctionCall args) {
		Js_Worker(args);
		SimpleHash hash;
		String2 str = args[0]->ToString2Value(worker);
		hash.update(*str, str.length());
		return hash;
	}
	
	static void hashCode(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			Js_Throw("Bad argument");
		}
		Js_Return( get_hashCode(args).hashCode() );
	}
	
	static void hash(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			Js_Throw("Bad argument");
		}
		Js_Return( get_hashCode(args).digest() );
	}
	
	static void version(FunctionCall args) {
		Js_Worker(args);
		Js_Return( qk::version() );
	}
	
	static void addNativeEventListener(FunctionCall args) {
		Js_Worker(args);
		if ( args.Length() < 3 || !args[0]->IsObject(worker) ||
				!args[1]->IsString(worker) || !args[2]->IsFunction(worker)) {
			Js_Throw("Bad argument");
		}
		if ( ! WrapObject::isPack(args[0].To<JSObject>()) ) {
			Js_Throw("Bad argument");
		}
		int id = 0;
		if ( args.Length() > 3 && args[3]->IsNumber(worker) ) {
			id = args[3]->ToNumberValue(worker);
		}
		{ HandleScope scope(worker);
			WrapObject* wrap = WrapObject::unpack(args[0].To<JSObject>());
			String name = args[1]->ToStringValue(worker,1);
			String func = String("_on").push(name).push("Native").push(String(id));
			bool ok = wrap->addEventListener(name, func, id);
			if (ok) {
				wrap->set(worker->New(func,1), args[2]);
			}
			Js_Return(ok);
		}
	}
	
	static void removeNativeEventListener(FunctionCall args) {
		Js_Worker(args);
		if ( args.Length() < 2 || !args[0]->IsObject(worker) || !args[1]->IsString(worker)) {
			Js_Throw("Bad argument");
		}
		if ( ! WrapObject::isPack(args[0].To<JSObject>()) ) {
			Js_Throw("Bad argument");
		}
		int id = 0;
		if ( args.Length() > 2 && args[2]->IsNumber(worker) ) {
			id = args[2]->ToNumberValue(worker);
		}
		{ HandleScope scope(worker);
			String name = args[1]->ToStringValue(worker,1);
			WrapObject* wrap = WrapObject::unpack(args[0].To<JSObject>());
			bool ok = wrap->removeEventListener(name, id);
			if ( ok ) {
				String func = String("_on").push(name).push("Native").push(String(id));
				wrap->del( worker->New(func) );
			}
			Js_Return(ok);
		}
	}

	static void garbageCollection(FunctionCall args) {
		Js_Worker(args); UILock lock;
		worker->garbageCollection();
		#if Qk_MEMORY_TRACE_MARK
			Array<Object*> objs = Object::mark_objects();
			Object** objs2 = &objs[0];
			Qk_LOG("All unrelease heap objects count: %d", objs.size());
		#endif
	}
	
	static void runScript(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			Js_Throw("Bad argument");
		}
		Js_Handle_Scope();
		Local<JSString> name;
		Local<JSObject> sandbox;
		if (args.Length() > 1) {
			name = args[1]->ToString(worker);
		} else {
			name = worker->New("[eval]",1);
		}
		if (args.Length() > 2 && args[2]->IsObject(worker)) {
			sandbox = args[2].To<JSObject>();
		}
		Local<JSValue> rv = worker->runScript(args[0].To<JSString>(), name, sandbox);
		if ( !rv.IsEmpty() ) { // 没有值可能有异常
			Js_Return( rv );
		}
	}

	static void next_tick(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() == 0 || ! args[0]->IsFunction(worker)) {
			Js_Throw("Bad argument");
		}
		CopyablePersistentFunc func(worker, args[0].To<JSFunction>());
		RunLoop::next_tick(Cb([worker, func](Cb::Data& e) {
			Qk_ASSERT(!func.IsEmpty());
			Js_Handle_Scope();
			Js_Callback_Scope();
			func.local()->Call(worker);
		}));
	}

	static void exit(FunctionCall args) {
		Js_Worker(args);
		int code = 0;
		if (args.Length() > 0 && args[0]->IsInt32(worker)) {
			code = args[0]->ToInt32Value(worker);
		}
		qk::exit(code);
	}

	static void extendModuleContent(FunctionCall args) {
		Js_Worker(args);
		if (args.Length() < 1 || ! args[0]->IsString(worker)) {
			Js_Throw("Bad argument");
		}
		String path = args[0]->ToStringValue(worker);
		for (int i = 0; i < EXT_native_js_count_; i++) {
			const EXT_NativeJSCode* code = EXT_native_js_ + i;
			String name(code->name);
			if (path == name || path == name + code->ext) {
				Js_Return( worker->New(code->code, code->count) );
			}
		}
		Js_Return_Null();
	}

	/**
	 * @func binding
	 */
	static void binding(Local<JSObject> exports, Worker* worker) {

		Js_Set_Method(hashCode, hashCode);
		Js_Set_Method(hash, hash);
		Js_Set_Method(version, version);
		Js_Set_Method(addNativeEventListener, addNativeEventListener);
		Js_Set_Method(removeNativeEventListener, removeNativeEventListener);
		Js_Set_Method(runScript, runScript);
		Js_Set_Method(garbageCollection, garbageCollection);
		Js_Set_Method(nextTick, next_tick);
		Js_Set_Method(_exit, exit);
		Js_Set_Property(platform, qk::platform());

		Local<JSArray> argv = worker->NewArray();
		if (__quark_js_argv) {
			for (uint32_t i = 0; i < __quark_js_argv->length(); i++) {
				argv->Set(worker, i, worker->New(__quark_js_argv->item(i)));
			}
		}
		Js_Set_Property(argv, argv);
		Js_Set_Property(debug, !!__quark_js_have_debug);

		// extendModule
		Local<JSObject> extendModule = worker->NewObject();
		for (int i = 0; i < EXT_native_js_count_; i++) {
			Local<JSObject> module = worker->NewObject();
			const EXT_NativeJSCode* code = EXT_native_js_ + i;
			module->SetProperty(worker, "filename", String(code->name) + code->ext);
			module->SetProperty(worker, "extname", code->ext);
			extendModule->SetProperty(worker, code->name, module);
		}
		Js_Set_Property(__extendModule, extendModule);
		Js_Set_Method(__extendModuleContent, extendModuleContent);

		WrapNativeObject::binding(exports, worker);
		WrapSimpleHash::binding(exports, worker);
	}
};

Js_REG_MODULE(_util, NativeUtil)
Js_END
