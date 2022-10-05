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

#if USE_JSC

#include <JavaScriptCore/JavaScript.h>
#include "quark/util/util.h"
//#include "quark/util/string-builder.h"
#include "quark/util/array.h"
#include "quark/util/loop.h"

using namespace quark;

static JSClassRef NativeConstructor;
static JSObjectRef toString;
static JSObjectRef getOwnPropertyDescriptor;
static JSObjectRef hasOwnProperty;
static JSObjectRef ArrayBuffer;
static JSObjectRef SharedArrayBuffer;
// common string
static JSStringRef A_s = JSStringCreateWithUTF8CString("A");
static JSStringRef B_s = JSStringCreateWithUTF8CString("B");
static JSStringRef __native__body_s =
	JSStringCreateWithUTF8CString("return arguments.callee.__native__.apply(this,arguments)");
static JSStringRef __native__s = JSStringCreateWithUTF8CString("__native__");
static JSStringRef toString_s = JSStringCreateWithUTF8CString("toString");
static JSStringRef prototype_s = JSStringCreateWithUTF8CString("prototype");
static JSStringRef constructor_s = JSStringCreateWithUTF8CString("constructor");
static JSStringRef getOwnPropertyDescriptor_s = JSStringCreateWithUTF8CString("getOwnPropertyDescriptor");
static JSStringRef hasOwnProperty_s = JSStringCreateWithUTF8CString("hasOwnProperty");
static JSStringRef Object_s = JSStringCreateWithUTF8CString("Object");
static JSStringRef defineProperty_s = JSStringCreateWithUTF8CString("defineProperty");
static JSStringRef set_s = JSStringCreateWithUTF8CString("set");
static JSStringRef get_s = JSStringCreateWithUTF8CString("get");
static JSStringRef configurable_s = JSStringCreateWithUTF8CString("configurable");
static JSStringRef enumerable_s = JSStringCreateWithUTF8CString("enumerable");
static JSStringRef a_s = JSStringCreateWithUTF8CString("a");
static JSStringRef length_s = JSStringCreateWithUTF8CString("length");
static JSStringRef ArrayBuffer_s = JSStringCreateWithUTF8CString("ArrayBuffer");
static JSStringRef SharedArrayBuffer_s = JSStringCreateWithUTF8CString("SharedArrayBuffer");
static JSStringRef column_s = JSStringCreateWithUTF8CString("column");
static JSStringRef line_s = JSStringCreateWithUTF8CString("line");
static JSStringRef stack_s = JSStringCreateWithUTF8CString("stack");
static JSStringRef message_s = JSStringCreateWithUTF8CString("message");
static JSStringRef Proxy_s = JSStringCreateWithUTF8CString("Proxy");
static JSStringRef revocable_s = JSStringCreateWithUTF8CString("revocable");
static JSStringRef proxy_s = JSStringCreateWithUTF8CString("proxy");
static JSStringRef revoke_s = JSStringCreateWithUTF8CString("revoke");
static JSStringRef print_s = JSStringCreateWithUTF8CString("print");
static JSStringRef Uint8Array_s = JSStringCreateWithUTF8CString("Uint8Array");


static JSObjectRef B_prototyp;

static void FinalizeCallback(JSObjectRef object) {
	int* i = (int*)JSObjectGetPrivate(object);
	if ( i ) {
		Qk_LOG("FinalizeCallback, %d", *i);
		delete i;
	} else {
		Qk_LOG("FinalizeCallback");
	}
}

struct JSCStringTraits: public NonObjectTraits {
	inline static void Release(JSStringRef str) {
		if ( str )
			JSStringRelease(str);
	}
};

typedef quark::Handle<OpaqueJSString, JSCStringTraits> JSCStringPtr;

static JSValueRef ConstructorFunc(JSContextRef ctx,
																	JSObjectRef function,
																	JSObjectRef thisObject,
																	size_t argumentCount,
																	const JSValueRef arguments[], JSValueRef* exception) {
	Qk_LOG("Call Constructor func");
	JSObjectRef o = JSObjectMake(ctx, NativeConstructor, 0);
	JSObjectSetPrototype(ctx, o, JSObjectGetPrototype(ctx, thisObject));
	
	return o;
}

String to_string_utf8(JSContextRef ctx, JSValueRef val) {
	JSCStringPtr str = JSValueToStringCopy(ctx, val, 0);
	Buffer buff = Buffer::alloc((uint32_t)JSStringGetMaximumUTF8CStringSize(*str));
	size_t size = JSStringGetUTF8CString(*str, *buff, buff.length());
	buff.realloc(int(size) - 1);
	return buff.collapse_string();
}

static JSValueRef toStringCallback(JSContextRef ctx,
																	 JSObjectRef function,
																	 JSObjectRef thisObject,
																	 size_t argumentCount,
																	 const JSValueRef arguments[], JSValueRef* exception) {
	JSValueRef args[1] = { JSValueMakeString(ctx, toString_s) };
	JSValueRef r = JSObjectCallAsFunction(ctx, hasOwnProperty, thisObject, 1, args, exception);
	if (JSValueToBoolean(ctx, r)) {
		return JSValueMakeString(ctx, JSStringCreateWithUTF8CString("function() {\n  [native code]\n}"));
	} else {
		return JSObjectCallAsFunction(ctx, toString, thisObject, 0, 0, exception);
	}
}

static JSValueRef A_get_a(JSContextRef ctx,
													JSObjectRef function,
													JSObjectRef thisObject,
													size_t argumentCount,
													const JSValueRef arguments[], JSValueRef* exception) {
	Qk_LOG("get a");
	return JSValueMakeNumber(ctx, 100);
}

static JSValueRef A_set_a(JSContextRef ctx,
													JSObjectRef function,
													JSObjectRef thisObject,
													size_t argumentCount,
													const JSValueRef arguments[], JSValueRef* exception) {
	Qk_LOG("set a");
	return 0;
}

static JSObjectRef BConstructorCallback(JSContextRef ctx,
																				JSObjectRef constructor,
																				size_t argumentCount,
																				const JSValueRef arguments[], JSValueRef* exception) {
	JSObjectRef r = JSObjectMake(ctx, NativeConstructor, 0);
	auto prototype = JSObjectGetProperty(ctx, constructor, prototype_s, 0);
	JSObjectSetPrototype(ctx, r, prototype);
	return r;
}

static JSValueRef BFunctionCallback(JSContextRef ctx,
																		JSObjectRef function, JSObjectRef thisObject,
																		size_t argumentCount,
																		const JSValueRef arguments[], JSValueRef* exception) {
	Qk_LOG(JSValueIsObject(ctx, thisObject));
	return JSObjectMake(ctx, NativeConstructor, 0);
}

static JSValueRef GetPropertyCallback(JSContextRef ctx,
																			JSObjectRef object,
																			JSStringRef propertyName, JSValueRef* exception) {
	size_t size = JSStringGetMaximumUTF8CStringSize(propertyName);
	char* buffer = (char*)malloc(size);
	JSStringGetUTF8CString(propertyName, buffer, size);
	Qk_LOG("propertyName: %s", buffer);
	free(buffer);
	Qk_LOG("JSValueIsStrictEqual: %d", JSValueIsStrictEqual(ctx, B_prototyp, object));
	return nullptr;
}

static void test_jsc_class(JSGlobalContextRef ctx, JSObjectRef global) {

	JSClassDefinition def = kJSClassDefinitionEmpty;
	def.finalize = FinalizeCallback;
	NativeConstructor = JSClassCreate(&def);
	//
	JSObjectRef Object = (JSObjectRef)JSObjectGetProperty(ctx, global, Object_s, 0);
	JSObjectRef defineProperty = (JSObjectRef)JSObjectGetProperty(ctx, Object, defineProperty_s, 0);
	getOwnPropertyDescriptor = (JSObjectRef)JSObjectGetProperty(ctx, Object, getOwnPropertyDescriptor_s, 0);
	hasOwnProperty = (JSObjectRef)JSObjectGetProperty(ctx, Object, hasOwnProperty_s, 0);
	toString = (JSObjectRef)JSObjectGetProperty(ctx, Object, toString_s, 0);
	JSObjectRef toString2 = JSObjectMakeFunctionWithCallback(ctx, 0, toStringCallback);
	
	JSPropertyAttributes attrs =
		kJSPropertyAttributeDontEnum |
		kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete;
	
	// class A
	JSObjectRef A = JSObjectMakeFunction(ctx, A_s, 0, 0, __native__body_s , 0, 1, 0);
	JSObjectRef __native__ = JSObjectMakeFunctionWithCallback(ctx, 0, ConstructorFunc);
	JSObjectSetProperty(ctx, A, __native__s, __native__, attrs, 0);
	JSObjectSetProperty(ctx, A, toString_s, toString2, kJSPropertyAttributeDontEnum, 0);
	JSObjectRef A_prototype = (JSObjectRef)JSObjectGetProperty(ctx, A, prototype_s, 0);
	// define property accessor
	JSObjectRef get_a_func = JSObjectMakeFunctionWithCallback(ctx, 0, A_get_a);
	JSObjectRef set_a_func = JSObjectMakeFunctionWithCallback(ctx, 0, A_set_a);
	JSObjectRef desc = JSObjectMake(ctx, 0, 0);
	JSObjectSetProperty(ctx, desc, get_s, get_a_func, 0, 0);
	JSObjectSetProperty(ctx, desc, set_s, set_a_func, 0, 0);
	JSValueRef arguments[3];
	arguments[0] = A_prototype;
	arguments[1] = JSValueMakeString(ctx, a_s);
	arguments[2] = desc;
	JSObjectCallAsFunction(ctx, defineProperty, 0, 3, arguments, 0);
	// end
	
	// class B
	def = kJSClassDefinitionEmpty;
	def.className = "Function";
	def.attributes = kJSClassAttributeNoAutomaticPrototype;
	def.callAsConstructor = BConstructorCallback;
	def.callAsFunction = BFunctionCallback;
	JSClassRef BCls = JSClassCreate(&def);
	JSObjectRef B = JSObjectMake(ctx, BCls, nullptr);
	JSObjectSetProperty(ctx, B, toString_s, toString2, kJSPropertyAttributeDontEnum, 0);
	def.getProperty = GetPropertyCallback;
	JSClassRef BPrototypeCls = JSClassCreate(&def);
	B_prototyp = JSObjectMake(ctx, BPrototypeCls, nullptr);
	JSObjectSetProperty(ctx, B_prototyp, constructor_s, B, kJSPropertyAttributeDontEnum, 0);
	JSObjectSetPrototype(ctx, B_prototyp, A_prototype); // inherit A.prototype
	JSObjectSetProperty(ctx, B, prototype_s, B_prototyp, kJSPropertyAttributeDontEnum, 0);
	JSObjectSetPrototype(ctx, B, JSObjectGetPrototype(ctx, A)); // __proto__ inherit function
	// end
	
	JSObjectSetProperty(ctx, global, A_s, A, 0, 0);
	JSObjectSetProperty(ctx, global, B_s, B, 0, 0);
	
	cChar* script =
		"function run(){\n"
			"var e, i;\n"
			"class C extends B { constructor() { super(); this.a = 100; } };\n"
			"function C2(){ this.a = 100; }\n"
			"C2.prototype.__proto__ = A.prototype;\n"
			"var func = { get Func() {\n"
			" return (function(){\n"
			"   //new KK;\n"
			"   var e = new Error('error'); \n"
			"   return '\\n\\nline: ' + e.line + \n"
			"          ', column: ' + e.column + \n"
			"          ', message: ' + e.message + \n"
			"          ', stack: ' + e.stack + '\\n'; \n"
			" })();\n"
			"}, get AA() { return 100; } }\n"
			"var L = eval('(function B() { this.stack = (new Error()).stack })');\n"
			"try {\n"
				"var a = new A(0,1);\n"
				"i = a.a = 200; var t = a.a;\n"
				"var c = new C();\n"
			"} catch(e_) { e = e_; }\n"
			"var s = [\n"
				"'error: ' +                  (e?e.message:'none'),\n"
				"'test_err: ' +               func.Func(),\n"
				"'test_err_B: ' +             '\\n' + new L().stack + '\\n',\n"
				"'typeof A: ' +               (typeof A),\n"
				"'String(A): ' +              (String(A)),\n"
				"'typeof B: ' +               typeof B,\n"
				"'String(B): ' +              String(B),\n"
				"'typeof C: ' +               typeof C,\n"
				"'String(C): ' +              String(C),\n"
				"'A instanceof Function: ' +  (A instanceof Function),\n"
				"'a instanceof A: ' +         (a instanceof A),\n"
				"'c instanceof A: ' +         (c instanceof A),\n"
				"'c instanceof B: ' +         (c instanceof B),\n"
				"'c instanceof C: ' +         (c instanceof C),\n"
				"'c.a: ' +                    (c?c.a:c),\n"
				"'a.a: ' +                    (a?a.a:a),\n"
				"'i: ' +                      (i),\n"
				"'A.prototype: ' +            A.prototype,\n"
				"'B.prototype: ' +            B.prototype,\n"
			"];\n"
			"return '\\n' + s.join('\\n') + '\\n';\n"
		"}\n"
		"run();\n";
	
	JSValueRef exception = 0;
	JSStringRef url = JSStringCreateWithUTF8CString("[test.js]");
	JSStringRef jscript = JSStringCreateWithUTF8CString(script);
	JSValueRef rv = JSEvaluateScript(ctx, jscript, 0, url, 1, &exception);
	
	if (exception) { // err
		JSValueRef column = JSObjectGetProperty(ctx, (JSObjectRef)exception, column_s, 0);
		JSValueRef line = JSObjectGetProperty(ctx, (JSObjectRef)exception, line_s, 0);
		JSValueRef message = JSObjectGetProperty(ctx, (JSObjectRef)exception, message_s, 0);
		JSValueRef stack = JSObjectGetProperty(ctx, (JSObjectRef)exception, stack_s, 0);
		Qk_LOG("");
		Qk_LOG("line: %s, column: %s", *to_string_utf8(ctx, line), *to_string_utf8(ctx, column));
		Qk_LOG("");
		Qk_LOG(to_string_utf8(ctx, message));
		Qk_LOG("");
		Qk_LOG(to_string_utf8(ctx, stack));
		Qk_LOG("");
		//LOG(JSValueIsObject(ctx, stack));
	}
	
	Qk_LOG(to_string_utf8(ctx, rv));
}

static void test_jsc_proxy(JSGlobalContextRef ctx, JSObjectRef global) {
	// test Proxy
	JSValueRef Null = JSValueMakeNull(ctx);
	JSValueRef Undefined = JSValueMakeUndefined(ctx);
	JSObjectRef Proxy = (JSObjectRef)JSObjectGetProperty(ctx, global, Proxy_s, 0);
	JSObjectRef revocable = (JSObjectRef)JSObjectGetProperty(ctx, Proxy, revocable_s, 0);
	JSObjectRef target = JSObjectMake(ctx, 0, 0);
	JSObjectRef handle = JSObjectMake(ctx, 0, 0);
	JSValueRef argv[2] = { target, handle };
	JSValueRef ex = nullptr;
	//JSObjectRef proxy = JSObjectCallAsConstructor(ctx, Proxy, 2, argv0, &ex);
	JSObjectRef proxy_warp = (JSObjectRef)JSObjectCallAsFunction(ctx, revocable, 0, 2, argv, &ex);
	JSObjectRef proxy = (JSObjectRef)JSObjectGetProperty(ctx, proxy_warp, proxy_s, 0);
	JSObjectRef revoke = (JSObjectRef)JSObjectGetProperty(ctx, proxy_warp, revoke_s, 0);
	
	struct JSProxy {
		void* p0;
		void* p1;
		JSObjectRef target;
		JSObjectRef handle;
	};
	
	if (ex) {
		Qk_LOG(to_string_utf8(ctx, ex));
	}
	Qk_LOG(proxy);
	
	JSProxy* p = reinterpret_cast<JSProxy*>(proxy);
	
	JSObjectCallAsFunction(ctx, revoke, proxy_warp, 0, 0, &ex);
	Qk_Assert(ex == nullptr);
	Qk_Assert(p->handle == Null);
	
	Qk_LOG("end");
}

static void ArrayBytesDeallocator(void* bytes, void* deallocatorContext) {
	Qk_LOG(bytes);
	free(bytes);
}

static void test_jsc_buffer(JSGlobalContextRef ctx, JSObjectRef global) {
	// test Buffer
	::ArrayBuffer = (JSObjectRef)JSObjectGetProperty(ctx, global, ArrayBuffer_s, 0);
	SharedArrayBuffer = (JSObjectRef)JSObjectGetProperty(ctx, global, SharedArrayBuffer_s, 0);
	JSObjectRef bf = JSObjectMakeArrayBufferWithBytesNoCopy
	(ctx, malloc(8), 8, ArrayBytesDeallocator, nullptr, nullptr);
	Qk_LOG(JSObjectGetArrayBufferByteLength(ctx, bf, 0));
	Qk_LOG(JSObjectGetArrayBufferBytesPtr(ctx, bf, 0));
	JSObjectRef Int8Array = JSObjectMakeTypedArrayWithArrayBuffer
	(ctx, kJSTypedArrayTypeInt8Array, bf, 0);
	Qk_LOG(JSObjectGetTypedArrayBytesPtr(ctx, Int8Array, 0));
	Qk_LOG(JSObjectGetTypedArrayLength(ctx, Int8Array, 0));
	JSObjectRef Int16Array = JSObjectMakeTypedArrayWithArrayBuffer
	(ctx, kJSTypedArrayTypeInt16Array, bf, 0);
	Qk_LOG(JSObjectGetTypedArrayBytesPtr(ctx, Int16Array, 0));
	Qk_LOG(JSObjectGetTypedArrayLength(ctx, Int16Array, 0));
	JSObjectRef Int32Array = JSObjectMakeTypedArrayWithArrayBuffer
	(ctx, kJSTypedArrayTypeInt32Array, bf, 0);
	Qk_LOG(JSObjectGetTypedArrayBytesPtr(ctx, Int32Array, 0));
	Qk_LOG(JSObjectGetTypedArrayLength(ctx, Int32Array, 0));
	Qk_LOG(JSObjectGetTypedArrayByteLength(ctx, Int32Array, 0));
	JSObjectRef Float64Array = JSObjectMakeTypedArrayWithArrayBuffer
	(ctx, kJSTypedArrayTypeFloat64Array, bf, 0);
	Qk_LOG(JSObjectGetTypedArrayBytesPtr(ctx, Float64Array, 0));
	Qk_LOG(JSObjectGetTypedArrayLength(ctx, Float64Array, 0));
	// shared array buffer
	JSValueRef argv[1] = {
		JSValueMakeNumber(ctx, 8),
	};
	auto sbf = JSObjectCallAsConstructor(ctx, SharedArrayBuffer, 1, argv, 0);
	Qk_LOG(JSObjectGetArrayBufferBytesPtr(ctx, sbf, 0));
	Qk_LOG(JSObjectGetArrayBufferByteLength(ctx, sbf, 0));
	//
}

JSValueRef PrintCallback(JSContextRef ctx, JSObjectRef function,
												 JSObjectRef thisObject, size_t
												 argumentCount, const JSValueRef arguments[], JSValueRef* exception) {
	String s;
	for (int i = 0; i < argumentCount; i++) {
		s.append(to_string_utf8(ctx, arguments[i]));
	}
	Qk_LOG(s);
	return nullptr;
}

void test_jsc(int argc, char **argv) {
	JSContextGroupRef group = JSContextGroupCreate();
	JSGlobalContextRef ctx = JSGlobalContextCreateInGroup(group, nullptr);
	JSGlobalContextRef ctx1 = JSGlobalContextCreateInGroup(group, nullptr);
	JSObjectRef global = JSContextGetGlobalObject(ctx);
	JSObjectRef global1 = JSContextGetGlobalObject(ctx1);
	JSObjectRef global_wrapper = JSObjectMake(ctx, 0, 0);
	JSObjectSetPrototype(ctx, global_wrapper, JSObjectGetPrototype(ctx, global));
	JSObjectSetPrototype(ctx, global, global_wrapper);
	
	//
	auto print = JSObjectMakeFunctionWithCallback(ctx, 0, PrintCallback);
	JSValueProtect(ctx1, print);
	JSObjectSetProperty(ctx, global_wrapper, print_s, print, 0, 0);
	JSObjectSetProperty(ctx, global1, print_s, print, 0, 0);
	JSObjectSetProperty(ctx, global_wrapper, column_s, JSValueMakeNumber(ctx, 200), 0, 0);
	JSValueRef JSObject = JSObjectGetProperty(ctx, global, Object_s, 0);
	JSValueRef JSObject1 = JSObjectGetProperty(ctx1, global, Object_s, 0);
	Qk_Assert(JSObject == JSObject1);
	Qk_LOG(to_string_utf8(ctx, JSObjectGetProperty(ctx, global, column_s, 0)));
	
	JSCStringPtr js = JSStringCreateWithUTF8CString("var a = 100; b = 200; print(a, b)");
	JSEvaluateScript(ctx, *js, 0, 0, 0, 0);
	JSCStringPtr js2 = JSStringCreateWithUTF8CString("print(this.a, this.b)");
	JSEvaluateScript(ctx1, *js2, 0, 0, 0, 0);
	
	auto Object_0 = JSObjectGetProperty(ctx, global, Object_s, 0);
	auto Object_1 = JSObjectGetProperty(ctx1, global1, Object_s, 0);
	Qk_LOG(Object_0 == Object_1);
	auto Uint8Array_0 = JSObjectGetProperty(ctx, global, Uint8Array_s, 0);
	auto Uint8Array_1 = JSObjectGetProperty(ctx1, global1, Uint8Array_s, 0);
	Qk_LOG(Uint8Array_0 == Uint8Array_1);
	
	
	//
	int o;
	Qk_LOG(sscanf( "100v.v009", "%d", &o));
	Qk_LOG(o);
	
	test_jsc_proxy(ctx, global);
	test_jsc_buffer(ctx, global);
	test_jsc_class(ctx, global);
	
	JSValueProtect(ctx, print);
	JSGlobalContextRelease(ctx1);
	Qk_LOG(to_string_utf8(ctx, JSObjectGetProperty(ctx, global1, print_s, 0)));
	JSGlobalContextRelease(ctx);
	JSContextGroupRelease(group);
}

#endif

