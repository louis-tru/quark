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

#include "v8-debug.h"

namespace v8 {

	bool Debug::SetDebugEventListener(Isolate* isolate, EventCallback that, Local<Value> data) {
		return false;
	}

	void Debug::DebugBreak(Isolate* isolate) {}

	void Debug::CancelDebugBreak(Isolate* isolate) {}

	bool Debug::CheckDebugBreak(Isolate* isolate) { 
		return false;
	}

	void Debug::SetMessageHandler(Isolate* isolate, MessageHandler handler) {}

	void Debug::SendCommand(Isolate* isolate, const uint16_t* command,
													int length, ClientData* client_data) {}

	MaybeLocal<Value> Debug::Call(Local<Context> context,
																v8::Local<v8::Function> fun, Local<Value> data) {
		return fun->Call(context, Local<Value>(), data.IsEmpty() ? 0 : 1, &data);
	}

	void Debug::ProcessDebugMessages(Isolate* isolate) {}

	Local<Context> Debug::GetDebugContext(Isolate* isolate) {
		return isolate->GetCurrentContext();
	}

	MaybeLocal<Context> Debug::GetDebuggedContext(Isolate* isolate) {
		return MaybeLocal<Context>(isolate->GetCurrentContext());
	}

	void Debug::SetLiveEditEnabled(Isolate* isolate, bool enable) {}

	MaybeLocal<Array> Debug::GetInternalProperties(Isolate* isolate, Local<Value> value) {
		return MaybeLocal<Array>();
	}

	bool Debug::IsTailCallEliminationEnabled(Isolate* isolate) {
		return false;
	}

	void Debug::SetTailCallEliminationEnabled(Isolate* isolate, bool enabled) {}
	
}
