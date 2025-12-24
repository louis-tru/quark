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

// @private head

#ifndef SRC_INSPECTOR_AGENT_H_
#define SRC_INSPECTOR_AGENT_H_

#include "../js_.h"
#include <memory>
#include <stddef.h>
#include <v8.h>
#include <uv.h>

namespace v8_inspector {
	class StringView;
} // namespace v8_inspector

namespace qk {
namespace js {
	class Worker;
}

namespace inspector {
	using js::Worker;
	using js::DebugOptions;

	class InspectorIo;
	class InspectorClient;

	class InspectorSessionDelegate {
	public:
		virtual ~InspectorSessionDelegate() = default;
		virtual void WaitForFrontendMessage() = 0;
		virtual void SendMessageToFrontend(const v8_inspector::StringView& message) = 0;
	};

	template<typename Inner, typename Outer>
	inline Outer* ContainerOf(Inner Outer::*field, Inner* pointer) {
		return reinterpret_cast<Outer*>(
			reinterpret_cast<uintptr_t>(pointer) -
			reinterpret_cast<uintptr_t>(&(reinterpret_cast<Outer*>(0)->*field))
		);
	}
	bool EntropySource(unsigned char* buffer, size_t length);
	bool StringEqualNoCaseN(const char* a, const char* b, size_t length);

	class Agent {
	public:
		Agent(Worker* worker);
		~Agent();

		// Create client_, may create io_ if option enabled
		bool Start(const DebugOptions &opts);
		// Stop and destroy io_
		void Stop();

		bool IsStarted() { return !!cli_; }

		// IO thread started, and client connected
		bool IsConnected();

		void FatalException(v8::Local<v8::Value> error, v8::Local<v8::Message> message);

		// Async stack traces instrumentation.
		void AsyncTaskScheduled(const v8_inspector::StringView& taskName, void* task,
														bool recurring);
		void AsyncTaskCanceled(void* task);
		void AsyncTaskStarted(void* task);
		void AsyncTaskFinished(void* task);
		void AllAsyncTasksCanceled();
		void EnableAsyncHook();
		void DisableAsyncHook();
		void RegisterAsyncHook(v8::Isolate* isolate,
			v8::Local<v8::Function> enable_function, v8::Local<v8::Function> disable_function);

		void ContextCreated(v8::Local<v8::Context> context);
		void PauseOnNextJavascriptStatement(const std::string& reason);

		InspectorSessionDelegate* delegate();
		InspectorIo* io() { return io_.get(); }
		DebugOptions& options() { return debug_options_; }
		uv_loop_t* event_loop() { return event_loop_; }
		Worker* worker() { return worker_; }
		v8::Isolate* isolate();
		v8::Local<v8::Context> firstContext();

	private:
		void ToggleAsyncHook(v8::Isolate* isolate, v8::Local<v8::Function> fn);

		// These methods are called by the WS protocol and JS binding to create
		// inspector sessions.  The inspector responds by using the delegate to send
		// messages back.
		void Connect(InspectorSessionDelegate* delegate);
		void Disconnect();
		void RunMessageLoop();
		void Dispatch(const v8_inspector::StringView& message);
		void WaitForDisconnect();

		js::Worker *worker_;
		std::unique_ptr<InspectorClient> cli_;
		std::unique_ptr<InspectorIo> io_;
		DebugOptions debug_options_;
		int next_context_number_;

		uv_loop_t* event_loop_;

		bool pending_enable_async_hook_, pending_disable_async_hook_;
		v8::Persistent<v8::Function> enable_async_hook_function_;
		v8::Persistent<v8::Function> disable_async_hook_function_;

		friend class InspectorIo;
	};

}  // namespace inspector
}  // namespace node

#endif  // SRC_INSPECTOR_AGENT_H_
