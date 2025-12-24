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

#include "../../util/thread.h"
#include "inspector_agent.h"
#include "inspector_io.h"
#include <v8-inspector.h>
#include <v8-platform.h>
#include <libplatform/libplatform.h>
#include <string.h>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <openssl/rand.h>

namespace qk { namespace js {
	v8::Isolate* getIsolate(Worker* worker);
	v8::Local<v8::Context> getContext(Worker* worker);
} }

namespace qk { namespace inspector {
	namespace {
		using v8::Array;
		using v8::Context;
		using v8::Function;
		using v8::HandleScope;
		using v8::Isolate;
		using v8::Local;
		using v8::Object;
		using v8::Value;

		using v8_inspector::StringBuffer;
		using v8_inspector::StringView;
		using v8_inspector::V8Inspector;
		using v8_inspector::V8InspectorClient;

		std::unique_ptr<StringBuffer> ToProtocolString(Isolate* isolate, Local<Value> value) {
			auto str = value->ToString(isolate);
			if (!str->Length())
				return StringBuffer::create(StringView());
			ArrayBuffer<uint16_t> buffer(str->Length());
			str->Write(isolate, *buffer, 0, str->Length());
			return StringBuffer::create(StringView(*buffer, buffer.length()));
		}

		// Used in InspectorClient::currentTimeMS() below.
		const int NANOS_PER_MSEC = 1000000;
		const int CONTEXT_GROUP_ID = 1;

		class ChannelImpl final : public v8_inspector::V8Inspector::Channel {
		public:
			explicit ChannelImpl(V8Inspector* inspector,
													InspectorSessionDelegate* delegate)
													: delegate_(delegate) {
				session_ = inspector->connect(1, this, StringView());
			}

			virtual ~ChannelImpl() {}

			void dispatchProtocolMessage(const StringView& message) {
				session_->dispatchProtocolMessage(message);
			}

			void waitForFrontendMessage() {
				delegate_->WaitForFrontendMessage();
			}

			void schedulePauseOnNextStatement(const std::string& reason) {
				std::unique_ptr<StringBuffer> buffer = Utf8ToStringView(reason);
				session_->schedulePauseOnNextStatement(buffer->string(), buffer->string());
			}

			InspectorSessionDelegate* delegate() {
				return delegate_;
			}

		private:
			void sendResponse(
					int callId,
					std::unique_ptr<v8_inspector::StringBuffer> message) override {
				sendMessageToFrontend(message->string());
			}

			void sendNotification(
					std::unique_ptr<v8_inspector::StringBuffer> message) override {
				sendMessageToFrontend(message->string());
			}

			void flushProtocolNotifications() override { }

			void sendMessageToFrontend(const StringView& message) {
				delegate_->SendMessageToFrontend(message);
			}

			InspectorSessionDelegate* const delegate_;
			std::unique_ptr<v8_inspector::V8InspectorSession> session_;
		};

		class InspectorTimer {
		public:
			InspectorTimer(uv_loop_t* loop,
										double interval_s,
										V8InspectorClient::TimerCallback callback,
										void* data) : timer_(),
																	callback_(callback),
																	data_(data) {
				uv_timer_init(loop, &timer_);
				int64_t interval_ms = 1000 * interval_s;
				uv_timer_start(&timer_, OnTimer, interval_ms, interval_ms);
			}

			InspectorTimer(const InspectorTimer&) = delete;

			void Stop() {
				uv_timer_stop(&timer_);
				uv_close(reinterpret_cast<uv_handle_t*>(&timer_), TimerClosedCb);
			}

		private:
			static void OnTimer(uv_timer_t* uvtimer) {
				InspectorTimer* timer = ContainerOf(&InspectorTimer::timer_, uvtimer);
				timer->callback_(timer->data_);
			}

			static void TimerClosedCb(uv_handle_t* uvtimer) {
				InspectorTimer* timer =
						ContainerOf(&InspectorTimer::timer_,
															reinterpret_cast<uv_timer_t*>(uvtimer));
				delete timer;
			}

			~InspectorTimer() {}

			uv_timer_t timer_;
			V8InspectorClient::TimerCallback callback_;
			void* data_;
		};

		class InspectorTimerHandle {
		public:
			InspectorTimerHandle(uv_loop_t* loop, double interval_s,
													V8InspectorClient::TimerCallback callback, void* data) {
				timer_ = new InspectorTimer(loop, interval_s, callback, data);
			}

			InspectorTimerHandle(const InspectorTimerHandle&) = delete;

			~InspectorTimerHandle() {
				Qk_ASSERT_NE(timer_, nullptr);
				timer_->Stop();
				timer_ = nullptr;
			}
		private:
			InspectorTimer* timer_;
		};
	} // namespace

	class InspectorClient : public V8InspectorClient {
	public:
		InspectorClient(Agent *agent)
				: agent_(agent), terminated_(false), running_nested_loop_(false) {
			inspector_ = V8Inspector::create(agent->isolate(), this);
			contextCreated(agent->firstContext(), "Quark.js Main Context");
		}

		void runMessageLoopOnPause(int context_group_id) override {
			// Run on frontend main thread 
			Qk_ASSERT_NE(channel_, nullptr);
			if (running_nested_loop_)
				return;
			terminated_ = false;
			running_nested_loop_ = true;
			while (!terminated_) {
				channel_->waitForFrontendMessage();
			}
			terminated_ = false;
			running_nested_loop_ = false;
		}

		double currentTimeMS() override {
			return uv_hrtime() * 1.0 / NANOS_PER_MSEC;
		}

		void maxAsyncCallStackDepthChanged(int depth) override {
			if (depth == 0) {
				agent_->DisableAsyncHook();
			} else {
				agent_->EnableAsyncHook();
			}
		}

		void contextCreated(Local<Context> context, const std::string& name) {
			std::unique_ptr<StringBuffer> name_buffer = Utf8ToStringView(name);
			v8_inspector::V8ContextInfo info(context, CONTEXT_GROUP_ID,
																			name_buffer->string());
			inspector_->contextCreated(info);
		}

		void contextDestroyed(Local<Context> context) {
			inspector_->contextDestroyed(context);
		}

		void quitMessageLoopOnPause() override {
			terminated_ = true;
		}

		void connectFrontend(InspectorSessionDelegate* delegate) {
			Qk_ASSERT_EQ(channel_, nullptr);
			channel_ = std::unique_ptr<ChannelImpl>(new ChannelImpl(inspector_.get(), delegate));
		}

		void disconnectFrontend() {
			quitMessageLoopOnPause();
			channel_.reset();
		}

		void dispatchMessageFromFrontend(const StringView& message) {
			Qk_ASSERT_NE(channel_, nullptr);
			channel_->dispatchProtocolMessage(message);
		}

		Local<Context> ensureDefaultContextInGroup(int contextGroupId) override {
			return agent_->firstContext();
		}

		void installAdditionalCommandLineAPI(Local<Context> context, Local<Object> target) override {
			auto worker = agent_->worker();
			auto require = worker->bindingModule("_pkg")->
				cast<js::JSObject>()->get(worker, "Module")->
				cast<js::JSObject>()->get(worker, "require");
			auto func = *reinterpret_cast<v8::Local<v8::Function>*>(&require);
			auto str = v8::String::NewFromOneByte(agent_->isolate(),
				(const uint8_t*)"require", v8::NewStringType::kNormal).ToLocalChecked();
			target->Set(context, str, func).Check();
		}

		void FatalException(Local<Value> error, Local<v8::Message> message) {
			Local<Context> context = agent_->firstContext();
			Local<v8::StackTrace> stack_trace = message->GetStackTrace();
			auto script_id = message->GetScriptOrigin().ScriptID()->Value();

			if (!stack_trace.IsEmpty() &&
					stack_trace->GetFrameCount() > 0 &&
					script_id == stack_trace->GetFrame(agent_->isolate(), 0)->GetScriptId()) {
				script_id = 0;
			}

			const uint8_t DETAILS[] = "Uncaught";

			Isolate* isolate = context->GetIsolate();

			inspector_->exceptionThrown(
					context,
					StringView(DETAILS, sizeof(DETAILS) - 1),
					error,
					ToProtocolString(isolate, message->Get())->string(),
					ToProtocolString(isolate, message->GetScriptResourceName())->string(),
					message->GetLineNumber(context).FromMaybe(0),
					message->GetStartColumn(context).FromMaybe(0),
					inspector_->createStackTrace(stack_trace),
					int(script_id));
		}

		ChannelImpl* channel() {
			return channel_.get();
		}

		void startRepeatingTimer(double interval_s,
														TimerCallback callback,
														void* data) override {
			timers_.emplace(std::piecewise_construct, std::make_tuple(data),
											std::make_tuple(agent_->event_loop(), interval_s, callback, data));
		}

		void cancelTimer(void* data) override {
			timers_.erase(data);
		}

		// Async stack traces instrumentation.
		void AsyncTaskScheduled(const StringView& task_name, void* task,
														bool recurring) {
			inspector_->asyncTaskScheduled(task_name, task, recurring);
		}

		void AsyncTaskCanceled(void* task) {
			inspector_->asyncTaskCanceled(task);
		}

		void AsyncTaskStarted(void* task) {
			inspector_->asyncTaskStarted(task);
		}

		void AsyncTaskFinished(void* task) {
			inspector_->asyncTaskFinished(task);
		}

		void AllAsyncTasksCanceled() {
			inspector_->allAsyncTasksCanceled();
		}

	private:
		Agent *agent_;
		bool terminated_;
		bool running_nested_loop_;
		std::unique_ptr<V8Inspector> inspector_;
		std::unique_ptr<ChannelImpl> channel_;
		std::unordered_map<void*, InspectorTimerHandle> timers_;
	};

	Agent::Agent(Worker *worker)
		: worker_(worker),
		cli_(nullptr),
		next_context_number_(1),
		pending_enable_async_hook_(false),
		pending_disable_async_hook_(false),
		event_loop_(RunLoop::current()->uv_loop())
	{}
	
	Agent::~Agent() {
	}

	bool Agent::Start(const DebugOptions &opts) {
		if (io_ != nullptr)
			return true;

		debug_options_ = opts;
		cli_ = std::unique_ptr<InspectorClient>(new InspectorClient(this));
		io_ = std::unique_ptr<InspectorIo>(new InspectorIo(this, debug_options_.waiting_for_connect));

		// This will return false if listen failed on the inspector port.
		if (!io_->Start()) {
			io_.reset();
			cli_.reset();
			return false;
		}
		return true;
	}

	void Agent::Stop() {
		if (io_ != nullptr) {
			io_->Stop();
			io_.reset();
			cli_.reset();
		}
	}

	void Agent::Connect(InspectorSessionDelegate* delegate) {
		Qk_ASSERT_NE(cli_, nullptr);
		cli_->connectFrontend(delegate);
	}

	void Agent::Disconnect() {
		Qk_ASSERT_NE(cli_, nullptr);
		cli_->disconnectFrontend();
	}

	bool Agent::IsConnected() {
		return io_ && io_->IsConnected();
	}

	void Agent::WaitForDisconnect() {
		Qk_ASSERT_NE(cli_, nullptr);
		cli_->contextDestroyed(firstContext());
		if (io_ != nullptr) {
			io_->WaitForDisconnect();
		}
	}

	void Agent::FatalException(Local<Value> error, Local<v8::Message> message) {
		if (!IsStarted())
			return;
		cli_->FatalException(error, message);
		WaitForDisconnect();
	}

	void Agent::Dispatch(const StringView& message) {
		Qk_ASSERT_NE(cli_, nullptr);
		cli_->dispatchMessageFromFrontend(message);
	}

	void Agent::RunMessageLoop() {
		Qk_ASSERT_NE(cli_, nullptr);
		cli_->runMessageLoopOnPause(CONTEXT_GROUP_ID);
	}

	InspectorSessionDelegate* Agent::delegate() {
		Qk_ASSERT_NE(cli_, nullptr);
		auto channel = cli_->channel();
		return channel ? channel->delegate(): nullptr;
	}

	void Agent::PauseOnNextJavascriptStatement(const std::string& reason) {
		ChannelImpl* channel = cli_->channel();
		if (channel != nullptr)
			channel->schedulePauseOnNextStatement(reason);
	}

	void Agent::RegisterAsyncHook(Isolate* isolate,
																v8::Local<v8::Function> enable_function,
																v8::Local<v8::Function> disable_function) {
		enable_async_hook_function_.Reset(isolate, enable_function);
		disable_async_hook_function_.Reset(isolate, disable_function);
		if (pending_enable_async_hook_) {
			Qk_ASSERT(!pending_disable_async_hook_);
			pending_enable_async_hook_ = false;
			EnableAsyncHook();
		} else if (pending_disable_async_hook_) {
			Qk_ASSERT(!pending_enable_async_hook_);
			pending_disable_async_hook_ = false;
			DisableAsyncHook();
		}
	}

	void Agent::EnableAsyncHook() {
		if (!enable_async_hook_function_.IsEmpty()) {
			Isolate* isolate = this->isolate();
			ToggleAsyncHook(isolate, enable_async_hook_function_.Get(isolate));
		} else if (pending_disable_async_hook_) {
			Qk_ASSERT(!pending_enable_async_hook_);
			pending_disable_async_hook_ = false;
		} else {
			pending_enable_async_hook_ = true;
		}
	}

	void Agent::DisableAsyncHook() {
		if (!disable_async_hook_function_.IsEmpty()) {
			Isolate* isolate = this->isolate();
			ToggleAsyncHook(isolate, disable_async_hook_function_.Get(isolate));
		} else if (pending_enable_async_hook_) {
			Qk_ASSERT(!pending_disable_async_hook_);
			pending_enable_async_hook_ = false;
		} else {
			pending_disable_async_hook_ = true;
		}
	}

	void Agent::ToggleAsyncHook(Isolate* isolate, Local<Function> fn) {
		HandleScope handle_scope(isolate);
		auto context = firstContext();
		auto result = fn->Call(context, Undefined(isolate), 0, nullptr);
		if (result.IsEmpty()) {
			Qk_Fatal("node::inspector::Agent::ToggleAsyncHook, Cannot toggle Inspector's AsyncHook, please report this.");
		}
	}

	void Agent::AsyncTaskScheduled(const StringView& task_name, void* task,
																bool recurring) {
		cli_->AsyncTaskScheduled(task_name, task, recurring);
	}

	void Agent::AsyncTaskCanceled(void* task) {
		cli_->AsyncTaskCanceled(task);
	}

	void Agent::AsyncTaskStarted(void* task) {
		cli_->AsyncTaskStarted(task);
	}

	void Agent::AsyncTaskFinished(void* task) {
		cli_->AsyncTaskFinished(task);
	}

	void Agent::AllAsyncTasksCanceled() {
		cli_->AllAsyncTasksCanceled();
	}

	void Agent::ContextCreated(Local<Context> context) {
		if (cli_ == nullptr)  // This happens for a main context
			return;
		std::ostringstream name;
		name << "VM Context " << next_context_number_++;
		cli_->contextCreated(context, name.str());
	}

	v8::Isolate* Agent::isolate() {
		return js::getIsolate(worker_);
	}

	v8::Local<v8::Context> Agent::firstContext() {
		return js::getContext(worker_);
	}

	char ToLower(char c) {
		return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
	}

	bool StringEqualNoCaseN(const char* a, const char* b, size_t length) {
		for (size_t i = 0; i < length; i++) {
			if (ToLower(a[i]) != ToLower(b[i]))
				return false;
			if (a[i] == '\0')
				return true;
		}
		return true;
	}

	// Ensure that OpenSSL has enough entropy (at least 256 bits) for its PRNG.
	// The entropy pool starts out empty and needs to fill up before the PRNG
	// can be used securely.  Once the pool is filled, it never dries up again;
	// its contents is stirred and reused when necessary.
	//
	// OpenSSL normally fills the pool automatically but not when someone starts
	// generating random numbers before the pool is full: in that case OpenSSL
	// keeps lowering the entropy estimate to thwart attackers trying to guess
	// the initial state of the PRNG.
	//
	// When that happens, we will have to wait until enough entropy is available.
	// That should normally never take longer than a few milliseconds.
	//
	// OpenSSL draws from /dev/random and /dev/urandom.  While /dev/random may
	// block pending "true" randomness, /dev/urandom is a CSPRNG that doesn't
	// block under normal circumstances.
	//
	// The only time when /dev/urandom may conceivably block is right after boot,
	// when the whole system is still low on entropy.  That's not something we can
	// do anything about.
	inline void CheckEntropy() {
		for (;;) {
			int status = RAND_status();
			Qk_ASSERT(status >= 0);  // Cannot fail.
			if (status != 0)
				break;

			// Give up, RAND_poll() not supported.
			if (RAND_poll() == 0)
				break;
		}
	}

	bool EntropySource(unsigned char* buffer, size_t length) {
		// Ensure that OpenSSL's PRNG is properly seeded.
		CheckEntropy();
		// RAND_bytes() can return 0 to indicate that the entropy data is not truly
		// random. That's okay, it's still better than V8's stock source of entropy,
		// which is /dev/urandom on UNIX platforms and the current time on Windows.
		return RAND_bytes(buffer, length) != -1;
	}

}  // namespace inspector
}  // namespace node

