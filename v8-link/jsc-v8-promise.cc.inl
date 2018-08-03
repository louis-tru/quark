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


// --- P r o m i s e ---

v8_ns(internal)

class Resolver: public Wrap {
 public:
	inline Resolver(Isolate* iso)
		: Wrap(iso), m_promise(nullptr)
		, m_resolve(nullptr)
		, m_reject(nullptr) { }
	
	Resolver* Init() {
		ENV(GetIsolate());
		Local<Function> cb = Function::New(isolate->GetCurrentContext(),
																			 Callback, Cast(Handle()) ).ToLocalChecked();
		m_promise = JSObjectCallAsConstructor(ctx, isolate->Promise(), 1,
																					reinterpret_cast<JSValueRef*>(&cb),
																					OK(nullptr));
		DCHECK(m_promise);
		DCHECK(m_resolve);
		DCHECK(m_reject);
		Reference(m_promise);
		return this;
	}
	
	static void Callback(const FunctionCallbackInfo<Value>& args) {
		DCHECK(!args.Data().IsEmpty());
		DCHECK(args.Length() == 2);
		DCHECK(args[0]->IsFunction());
		DCHECK(args[1]->IsFunction());
		Resolver* self = static_cast<Resolver*>(Unwrap(i::Back<JSObjectRef>(args.Data())));
		self->m_resolve = Back<JSObjectRef>(args[0]);
		self->m_reject = Back<JSObjectRef>(args[1]);
		self->Reference(self->m_resolve);
		self->Reference(self->m_reject);
	}
	
	Maybe<bool> Resolve(Local<Value> value) {
		ENV(GetIsolate());
		JSValueRef argv[1] = { i::Back(value) };
		JSObjectCallAsFunction(ctx, m_resolve, 0, 1, argv, NOTHING);
		return Just(true);
	}
	
	Maybe<bool> Reject(Local<Value> value) {
		ENV(GetIsolate());
		JSValueRef argv[1] = { i::Back(value) };
		JSObjectCallAsFunction(ctx, m_reject, 0, 1, argv, NOTHING);
		return Just(true);
	}

	inline Local<Promise> GetPromise() {
		return Cast<Promise>(m_promise);
	}
	
 private:
	JSObjectRef m_promise;
	JSObjectRef m_resolve;
	JSObjectRef m_reject;
};

v8_ns_end

MaybeLocal<Promise::Resolver> Promise::Resolver::New(Local<Context> context) {
	auto r = new i::Resolver(ISOLATE(context->GetIsolate()));
	return i::Cast<Promise::Resolver>(r->Init());
}

Local<Promise::Resolver> Promise::Resolver::New(Isolate* isolate) {
	auto r = new i::Resolver(ISOLATE(isolate));
	return i::Cast<Promise::Resolver>(r->Init());
}

Local<Promise> Promise::Resolver::GetPromise() {
	return reinterpret_cast<i::Resolver*>(this)->GetPromise();
}

Maybe<bool> Promise::Resolver::Resolve(Local<Context> context,
																			 Local<Value> value) {
	reinterpret_cast<i::Resolver*>(this)->Resolve(value);
}

void Promise::Resolver::Resolve(Local<Value> value) {
	reinterpret_cast<i::Resolver*>(this)->Resolve(value);
}

Maybe<bool> Promise::Resolver::Reject(Local<Context> context,
																			Local<Value> value) {
	reinterpret_cast<i::Resolver*>(this)->Reject(value);
}

void Promise::Resolver::Reject(Local<Value> value) {
	reinterpret_cast<i::Resolver*>(this)->Reject(value);
}

MaybeLocal<Promise> Promise::Catch(Local<Context> context,
																	 Local<Function> handler) {
	ENV(context->GetIsolate());
	auto r = JSObjectCallAsFunction(ctx, 0, i::Back<JSObjectRef>(this),
																	1, reinterpret_cast<JSValueRef*>(&handler),
																	OK(MaybeLocal<Promise>()));
	return i::Cast<Promise>(r);
}

Local<Promise> Promise::Catch(Local<Function> handler) {
	ENV();
	JSValueRef argv[2] = { i::Back(this), i::Back(handler) };
	auto r = JSObjectCallAsFunction(ctx, isolate->promiseCatch(),
																	0, 2, argv, OK(Local<Promise>()));
	return i::Cast<Promise>(r);
}

MaybeLocal<Promise> Promise::Then(Local<Context> context,
																	Local<Function> handler) {
	ENV(context->GetIsolate());
	JSValueRef argv[2] = { i::Back(this), i::Back(handler) };
	auto r = JSObjectCallAsFunction(ctx, isolate->promiseCatch(),
																	0, 2, argv, OK(MaybeLocal<Promise>()));
	return i::Cast<Promise>(r);
}

Local<Promise> Promise::Then(Local<Function> handler) {
	RETURN_TO_LOCAL_UNCHECKED(Then(Isolate::GetCurrent()->GetCurrentContext(), handler), Promise);
}

bool Promise::HasHandler() {
	return false;
}

Local<Value> Promise::Result() {
	ENV();
	auto r = JSObjectGetProperty(ctx, i::Back<JSObjectRef>(this), i::result_s, OK(Local<Value>()));
	return i::Cast(r);
}

Promise::PromiseState Promise::State() {
	ENV();
	JSValueRef argv[1] = { i::Back(this) };
	auto r = JSObjectCallAsFunction(ctx, isolate->promiseState(), 0, 1, argv, OK(kPending));
	int num = JSValueToNumber(ctx, r, OK(kPending));
	return (Promise::PromiseState)num;
}

