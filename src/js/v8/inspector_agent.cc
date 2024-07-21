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

#include "../../util/loop.h"
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

#if defined(_MSC_VER)
# define getpid GetCurrentProcessId
#endif

#if Qk_POSIX
# include <limits.h>
# include <unistd.h>  // setuid, getuid
#endif  // __POSIX__

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

		static uv_sem_t start_io_thread_semaphore;
		static uv_async_t start_io_thread_async;

		class StartIoTask : public v8::Task {
		public:
			StartIoTask(Agent* agent) : agent_(agent) {}
			void Run() override {
				agent_->StartIoThread(false);
			}
		private:
			Agent* agent_;
		};
		
#if Qk_POSIX
		void RegisterSignalHandler(int signal, void (*handler)(int signal), bool reset_handler) {
			struct sigaction sa;
			memset(&sa, 0, sizeof(sa));
			sa.sa_handler = handler;
		#ifndef __FreeBSD__
			// FreeBSD has a nasty bug with SA_RESETHAND reseting the SA_SIGINFO, that is
			// in turn set for a libthr wrapper. This leads to a crash.
			// Work around the issue by manually setting SIG_DFL in the signal handler
			sa.sa_flags = reset_handler ? SA_RESETHAND : 0;
		#endif
			sigfillset(&sa.sa_mask);
			Qk_Assert_Eq(sigaction(signal, &sa, nullptr), 0);
		}
#endif

		std::unique_ptr<StringBuffer> ToProtocolString(Isolate* isolate,
																									Local<Value> value) {
			auto str = value->ToString(isolate);
			if (!str->Length())
				return StringBuffer::create(StringView());
			ArrayBuffer<uint16_t> buffer(str->Length());
			str->Write(isolate, *buffer, 0, str->Length());
			return StringBuffer::create(StringView(*buffer, buffer.length()));
		}

		// Called on the main thread.
		void StartIoThreadAsyncCallback(uv_async_t* handle) {
			static_cast<Agent*>(handle->data)->StartIoThread(false);
		}

		void StartIoInterrupt(Isolate* isolate, void* agent) {
			static_cast<Agent*>(agent)->StartIoThread(false);
		}

#if Qk_POSIX
		static void StartIoThreadWakeup(int signo) {
			uv_sem_post(&start_io_thread_semaphore);
		}

		inline void* StartIoThreadMain(void* unused) {
			for (;;) {
				uv_sem_wait(&start_io_thread_semaphore);
				Agent* agent = static_cast<Agent*>(start_io_thread_async.data);
				if (agent != nullptr)
					agent->RequestIoThreadStart();
			}
			return nullptr;
		}

		static int StartDebugSignalHandler() {
			// Start a watchdog thread for calling v8::Debug::DebugBreak() because
			// it's not safe to call directly from the signal handler, it can
			// deadlock with the thread it interrupts.
			Qk_Assert_Eq(0, uv_sem_init(&start_io_thread_semaphore, 0));
			pthread_attr_t attr;
			Qk_Assert_Eq(0, pthread_attr_init(&attr));
			// Don't shrink the thread's stack on FreeBSD.  Said platform decided to
			// follow the pthreads specification to the letter rather than in spirit:
			// https://lists.freebsd.org/pipermail/freebsd-current/2014-March/048885.html
		#ifndef __FreeBSD__
			Qk_Assert_Eq(0, pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN));
		#endif  // __FreeBSD__
			Qk_Assert_Eq(0, pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED));
			sigset_t sigmask;
			// Mask all signals.
			sigfillset(&sigmask);
			Qk_Assert_Eq(0, pthread_sigmask(SIG_SETMASK, &sigmask, &sigmask));
			pthread_t thread;
			const int err = pthread_create(&thread, &attr,
																		StartIoThreadMain, nullptr);
			// Restore original mask
			Qk_Assert_Eq(0, pthread_sigmask(SIG_SETMASK, &sigmask, nullptr));
			Qk_Assert_Eq(0, pthread_attr_destroy(&attr));
			if (err != 0) {
				fprintf(stderr, "node[%d]: pthread_create: %s\n", getpid(), strerror(err));
				fflush(stderr);
				// Leave SIGUSR1 blocked.  We don't install a signal handler,
				// receiving the signal would terminate the process.
				return -err;
			}
			RegisterSignalHandler(SIGUSR1, StartIoThreadWakeup, false);
			// Unblock SIGUSR1.  A pending SIGUSR1 signal will now be delivered.
			sigemptyset(&sigmask);
			sigaddset(&sigmask, SIGUSR1);
			Qk_Assert_Eq(0, pthread_sigmask(SIG_UNBLOCK, &sigmask, nullptr));
			return 0;
		}
#endif  // __POSIX__

#ifdef _WIN32
		DWORD WINAPI StartIoThreadProc(void* arg) {
			Agent* agent = static_cast<Agent*>(start_io_thread_async.data);
			if (agent != nullptr)
				agent->RequestIoThreadStart();
			return 0;
		}

		static int GetDebugSignalHandlerMappingName(DWORD pid, wchar_t* buf,
																								size_t buf_len) {
			return _snwprintf(buf, buf_len, L"node-debug-handler-%u", pid);
		}

		static int StartDebugSignalHandler() {
			wchar_t mapping_name[32];
			HANDLE mapping_handle;
			DWORD pid;
			LPTHREAD_START_ROUTINE* handler;

			pid = GetCurrentProcessId();

			if (GetDebugSignalHandlerMappingName(pid,
																					mapping_name,
																					arraysize(mapping_name)) < 0) {
				return -1;
			}

			mapping_handle = CreateFileMappingW(INVALID_HANDLE_VALUE,
																					nullptr,
																					PAGE_READWRITE,
																					0,
																					sizeof *handler,
																					mapping_name);
			if (mapping_handle == nullptr) {
				return -1;
			}

			handler = reinterpret_cast<LPTHREAD_START_ROUTINE*>(
					MapViewOfFile(mapping_handle,
												FILE_MAP_ALL_ACCESS,
												0,
												0,
												sizeof *handler));
			if (handler == nullptr) {
				CloseHandle(mapping_handle);
				return -1;
			}

			*handler = StartIoThreadProc;

			UnmapViewOfFile(static_cast<void*>(handler));

			return 0;
		}
#endif  // _WIN32

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

			bool waitForFrontendMessage() {
				return delegate_->WaitForFrontendMessageWhilePaused();
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
				Qk_Assert_Ne(timer_, nullptr);
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
			client_ = V8Inspector::create(agent->isolate(), this);
			contextCreated(agent->context(), "Quark.js Main Context");
		}

		void runMessageLoopOnPause(int context_group_id) override {
			Qk_Assert_Ne(channel_, nullptr);
			if (running_nested_loop_)
				return;
			terminated_ = false;
			running_nested_loop_ = true;
			while (!terminated_ && channel_->waitForFrontendMessage()) {
				// TODO ...
				// platform_->FlushForegroundTasksInternal();
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
			client_->contextCreated(info);
		}

		void contextDestroyed(Local<Context> context) {
			client_->contextDestroyed(context);
		}

		void quitMessageLoopOnPause() override {
			terminated_ = true;
		}

		void connectFrontend(InspectorSessionDelegate* delegate) {
			Qk_Assert_Eq(channel_, nullptr);
			channel_ = std::unique_ptr<ChannelImpl>(
					new ChannelImpl(client_.get(), delegate));
		}

		void disconnectFrontend() {
			quitMessageLoopOnPause();
			channel_.reset();
		}

		void dispatchMessageFromFrontend(const StringView& message) {
			Qk_Assert_Ne(channel_, nullptr);
			channel_->dispatchProtocolMessage(message);
		}

		Local<Context> ensureDefaultContextInGroup(int contextGroupId) override {
			return agent_->context();
		}

		void installAdditionalCommandLineAPI(Local<Context> context, Local<Object> target) override {
			// TODO ... require
			// Local<Object> console_api = env_->inspector_console_api_object();
			// Local<Array> properties =
			// 		console_api->GetOwnPropertyNames(context).ToLocalChecked();
			// for (uint32_t i = 0; i < properties->Length(); ++i) {
			// 	Local<Value> key = properties->Get(context, i).ToLocalChecked();
			// 	target->Set(context,
			// 							key,
			// 							console_api->Get(context, key).ToLocalChecked()).FromJust();
			// }
		}

		void FatalException(Local<Value> error, Local<v8::Message> message) {
			Local<Context> context = agent_->context();

			int script_id = message->GetScriptOrigin().ScriptID()->Value();

			Local<v8::StackTrace> stack_trace = message->GetStackTrace();

			if (!stack_trace.IsEmpty() &&
					stack_trace->GetFrameCount() > 0 &&
					script_id == stack_trace->GetFrame(agent_->isolate(), 0)->GetScriptId()) {
				script_id = 0;
			}

			const uint8_t DETAILS[] = "Uncaught";

			Isolate* isolate = context->GetIsolate();

			client_->exceptionThrown(
					context,
					StringView(DETAILS, sizeof(DETAILS) - 1),
					error,
					ToProtocolString(isolate, message->Get())->string(),
					ToProtocolString(isolate, message->GetScriptResourceName())->string(),
					message->GetLineNumber(context).FromMaybe(0),
					message->GetStartColumn(context).FromMaybe(0),
					client_->createStackTrace(stack_trace),
					script_id);
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
			client_->asyncTaskScheduled(task_name, task, recurring);
		}

		void AsyncTaskCanceled(void* task) {
			client_->asyncTaskCanceled(task);
		}

		void AsyncTaskStarted(void* task) {
			client_->asyncTaskStarted(task);
		}

		void AsyncTaskFinished(void* task) {
			client_->asyncTaskFinished(task);
		}

		void AllAsyncTasksCanceled() {
			client_->allAsyncTasksCanceled();
		}

	private:
		Agent *agent_;
		bool terminated_;
		bool running_nested_loop_;
		std::unique_ptr<V8Inspector> client_;
		std::unique_ptr<ChannelImpl> channel_;
		std::unordered_map<void*, InspectorTimerHandle> timers_;
	};

	Agent::Agent(Worker *worker, v8::Platform* platform) : worker_(worker),
																	client_(nullptr),
																	platform_(platform),
																	enabled_(false),
																	next_context_number_(1),
																	pending_enable_async_hook_(false),
																	pending_disable_async_hook_(false),
																	event_loop_(RunLoop::first()->uv_loop())
																	{}

	bool Agent::Start(const char* path, DebugOptions opts) {
		path_ = path == nullptr ? "" : path;
		debug_options_ = opts;
		client_ = std::unique_ptr<InspectorClient>(new InspectorClient(this));
		Qk_Assert_Eq(0, uv_async_init(uv_default_loop(),
															&start_io_thread_async,
															StartIoThreadAsyncCallback));
		start_io_thread_async.data = this;
		uv_unref(reinterpret_cast<uv_handle_t*>(&start_io_thread_async));

		// Ignore failure, SIGUSR1 won't work, but that should not block node start.
		StartDebugSignalHandler();
			// This will return false if listen failed on the inspector port.
		return StartIoThread(IsWaitingForConnect());
	}

	bool Agent::StartIoThread(bool wait_for_connect) {
		if (io_ != nullptr)
			return true;

		Qk_Assert_Ne(client_, nullptr);

		enabled_ = true;
		io_ = std::unique_ptr<InspectorIo>(new InspectorIo(this, wait_for_connect));

		if (!io_->Start()) {
			client_.reset();
			return false;
		}

		v8::Isolate* isolate = this->isolate();
		HandleScope handle_scope(isolate);
		auto context = this->context();

		// TODO ...
		// Send message to enable debug in workers
		/*
		Local<Object> process_object = parent_env_->process_object();
		Local<Value> emit_fn =
				process_object->Get(context, FIXED_ONE_BYTE_STRING(isolate, "emit"))
						.ToLocalChecked();
		// In case the thread started early during the startup
		if (!emit_fn->IsFunction())
			return true;

		Local<Object> message = Object::New(isolate);
		message->Set(context, FIXED_ONE_BYTE_STRING(isolate, "cmd"),
								FIXED_ONE_BYTE_STRING(isolate, "NODE_DEBUG_ENABLED")).FromJust();
		Local<Value> argv[] = {
			FIXED_ONE_BYTE_STRING(isolate, "internalMessage"),
			message
		};
		MakeCallback(parent_env_->isolate(), process_object, emit_fn.As<Function>(),
								arraysize(argv), argv, {0, 0});
		*/

		return true;
	}

	void Agent::Stop() {
		if (io_ != nullptr) {
			io_->Stop();
			io_.reset();
			enabled_ = false;
		}
	}

	void Agent::Connect(InspectorSessionDelegate* delegate) {
		enabled_ = true;
		client_->connectFrontend(delegate);
	}

	bool Agent::IsConnected() {
		return io_ && io_->IsConnected();
	}

	void Agent::WaitForDisconnect() {
		Qk_Assert_Ne(client_, nullptr);
		client_->contextDestroyed(this->context());
		if (io_ != nullptr) {
			io_->WaitForDisconnect();
		}
	}

	v8::Isolate* Agent::isolate() {
		return js::getIsolate(worker_);
	}

	v8::Local<v8::Context> Agent::context() {
		return js::getContext(worker_);
	}

	void Agent::FatalException(Local<Value> error, Local<v8::Message> message) {
		if (!IsStarted())
			return;
		client_->FatalException(error, message);
		WaitForDisconnect();
	}

	void Agent::Dispatch(const StringView& message) {
		Qk_Assert_Ne(client_, nullptr);
		client_->dispatchMessageFromFrontend(message);
	}

	void Agent::Disconnect() {
		Qk_Assert_Ne(client_, nullptr);
		client_->disconnectFrontend();
	}

	void Agent::RunMessageLoop() {
		Qk_Assert_Ne(client_, nullptr);
		client_->runMessageLoopOnPause(CONTEXT_GROUP_ID);
	}

	InspectorSessionDelegate* Agent::delegate() {
		Qk_Assert_Ne(client_, nullptr);
		ChannelImpl* channel = client_->channel();
		if (channel == nullptr)
			return nullptr;
		return channel->delegate();
	}

	void Agent::PauseOnNextJavascriptStatement(const std::string& reason) {
		ChannelImpl* channel = client_->channel();
		if (channel != nullptr)
			channel->schedulePauseOnNextStatement(reason);
	}

	void Agent::RegisterAsyncHook(Isolate* isolate,
																v8::Local<v8::Function> enable_function,
																v8::Local<v8::Function> disable_function) {
		enable_async_hook_function_.Reset(isolate, enable_function);
		disable_async_hook_function_.Reset(isolate, disable_function);
		if (pending_enable_async_hook_) {
			Qk_Assert(!pending_disable_async_hook_);
			pending_enable_async_hook_ = false;
			EnableAsyncHook();
		} else if (pending_disable_async_hook_) {
			Qk_Assert(!pending_enable_async_hook_);
			pending_disable_async_hook_ = false;
			DisableAsyncHook();
		}
	}

	void Agent::EnableAsyncHook() {
		if (!enable_async_hook_function_.IsEmpty()) {
			Isolate* isolate = this->isolate();
			ToggleAsyncHook(isolate, enable_async_hook_function_.Get(isolate));
		} else if (pending_disable_async_hook_) {
			Qk_Assert(!pending_enable_async_hook_);
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
			Qk_Assert(!pending_disable_async_hook_);
			pending_enable_async_hook_ = false;
		} else {
			pending_disable_async_hook_ = true;
		}
	}

	void Agent::ToggleAsyncHook(Isolate* isolate, Local<Function> fn) {
		HandleScope handle_scope(isolate);
		auto context = this->context();
		auto result = fn->Call(context, Undefined(isolate), 0, nullptr);
		if (result.IsEmpty()) {
			Qk_FATAL("node::inspector::Agent::ToggleAsyncHook, Cannot toggle Inspector's AsyncHook, please report this.");
		}
	}

	void Agent::AsyncTaskScheduled(const StringView& task_name, void* task,
																bool recurring) {
		client_->AsyncTaskScheduled(task_name, task, recurring);
	}

	void Agent::AsyncTaskCanceled(void* task) {
		client_->AsyncTaskCanceled(task);
	}

	void Agent::AsyncTaskStarted(void* task) {
		client_->AsyncTaskStarted(task);
	}

	void Agent::AsyncTaskFinished(void* task) {
		client_->AsyncTaskFinished(task);
	}

	void Agent::AllAsyncTasksCanceled() {
		client_->AllAsyncTasksCanceled();
	}

	void Agent::RequestIoThreadStart() {
		// We need to attempt to interrupt V8 flow (in case Node is running
		// continuous JS code) and to wake up libuv thread (in case Node is waiting
		// for IO events)
		uv_async_send(&start_io_thread_async);
		v8::Isolate* isolate = this->isolate();
		platform_->CallOnForegroundThread(isolate, new StartIoTask(this));
		isolate->RequestInterrupt(StartIoInterrupt, this);
		uv_async_send(&start_io_thread_async);
	}

	void Agent::ContextCreated(Local<Context> context) {
		if (client_ == nullptr)  // This happens for a main context
			return;
		std::ostringstream name;
		name << "VM Context " << next_context_number_++;
		client_->contextCreated(context, name.str());
	}

	bool Agent::IsWaitingForConnect() {
		return debug_options_.break_first_line;
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
			Qk_Assert(status >= 0);  // Cannot fail.
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

