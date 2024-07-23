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

#include "../../util/codec.h"
#include "inspector_agent.h"
#include "inspector_io.h"
#include "inspector_socket_server.h"
#include <v8-inspector.h>
#include <v8-platform.h>
#include <zlib.h>
#include <uv.h>
#include <sstream>
#include <string.h>
#include <vector>

namespace qk { namespace inspector {
	using AsyncAndAgent = std::pair<uv_async_t, Agent*>;
	using v8_inspector::StringBuffer;
	using v8_inspector::StringView;

	template<typename Transport>
	using TransportAndIo = std::pair<Transport*, InspectorIo*>;

	std::string GetProcessTitle() {
		char title[2048];
		int err = uv_get_process_title(title, sizeof(title));
		if (err == 0) {
			return title;
		} else {
			// Title is too long, or could not be retrieved.
			return "Quark.js";
		}
	}

	// UUID RFC: https://www.ietf.org/rfc/rfc4122.txt
	// Used ver 4 - with numbers
	std::string GenerateID() {
		uint16_t buffer[8];
		Qk_Assert(EntropySource(reinterpret_cast<unsigned char*>(buffer), sizeof(buffer)));

		char uuid[256];
		snprintf(uuid, sizeof(uuid), "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
						buffer[0],  // time_low
						buffer[1],  // time_mid
						buffer[2],  // time_low
						(buffer[3] & 0x0fff) | 0x4000,  // time_hi_and_version
						(buffer[4] & 0x3fff) | 0x8000,  // clk_seq_hi clk_seq_low
						buffer[5],  // node
						buffer[6],
						buffer[7]);
		return uuid;
	}

	std::string StringViewToUtf8(const StringView& view) {
		if (view.is8Bit()) {
			return std::string(reinterpret_cast<const char*>(view.characters8()),
												view.length());
		}
		const uint16_t* source = view.characters16();
		auto buff = codec_encode(kUTF8_Encoding,
			codec_decode_form_utf16(ArrayWeak<uint16_t>(source, view.length()).buffer())
		);
		return std::string(*buff, buff.length());
	}

	std::unique_ptr<StringBuffer> Utf8ToStringView(const std::string& message) {
		auto utf16 = codec_encode_to_utf16(codec_decode_to_uint32(
			kUTF8_Encoding, WeakBuffer(message.c_str(), message.length()).buffer()
		));
		return StringBuffer::create(StringView(*utf16, utf16.length()));
	}

	void HandleSyncCloseCb(uv_handle_t* handle) {
		*static_cast<bool*>(handle->data) = true;
	}

	int CloseAsyncAndLoop(uv_async_t* async) {
		bool is_closed = false;
		async->data = &is_closed;
		uv_close(reinterpret_cast<uv_handle_t*>(async), HandleSyncCloseCb);
		while (!is_closed)
			uv_run(async->loop, UV_RUN_ONCE);
		async->data = nullptr;
		return uv_loop_close(async->loop);
	}

	// Delete main_thread_req_ on async handle close
	void ReleasePairOnAsyncClose(uv_handle_t* async) {
		AsyncAndAgent* pair = ContainerOf(&AsyncAndAgent::first,
																			reinterpret_cast<uv_async_t*>(async));
		delete pair;
	}

	class IoSessionDelegate : public InspectorSessionDelegate {
	public:
		explicit IoSessionDelegate(InspectorIo* io) : io_(io) {}
		void WaitForFrontendMessage() override;
		void SendMessageToFrontend(const v8_inspector::StringView& message) override;
	private:
		InspectorIo* io_;
	};

	// Passed to InspectorSocketServer to handle WS inspector protocol events,
	// mostly session start, message received, and session end.
	class InspectorIoDelegate: public inspector::SocketServerDelegate {
	public:
		InspectorIoDelegate(InspectorIo* io, const std::string& script_path,
												const std::string& script_name, bool wait);
		// Calls PostIncomingMessage() with appropriate InspectorAction:
		//   kStartSession
		bool StartSession(int session_id, const std::string& target_id) override;
		//   kSendMessage
		void MessageReceived(int session_id, const std::string& message) override;
		//   kEndSession
		void EndSession(int session_id) override;

		std::vector<std::string> GetTargetIds() override;
		std::string GetTargetTitle(const std::string& id) override;
		std::string GetTargetUrl(const std::string& id) override;
		bool IsConnected() { return connected_; }
		void ServerDone() override {
			io_->ServerDone();
		}

	private:
		InspectorIo* io_;
		bool connected_;
		int session_id_;
		const std::string script_name_;
		const std::string script_path_;
		const std::string target_id_;
		bool waiting_;
	};

	InspectorIo::InspectorIo(Agent *agent, bool wait_for_connect)
													: agent_(agent), thread_(), delegate_(nullptr),
														state_(State::kNew), thread_req_(),
														dispatching_messages_(false), session_id_(0),
														wait_for_connect_(wait_for_connect), port_(-1) {
		main_thread_req_ = new AsyncAndAgent({uv_async_t(), agent});
		Qk_Assert_Eq(0, uv_async_init(agent->event_loop(), &main_thread_req_->first,
															InspectorIo::MainThreadReqAsyncCb));
		uv_unref(reinterpret_cast<uv_handle_t*>(&main_thread_req_->first));
		Qk_Assert_Eq(0, uv_sem_init(&thread_start_sem_, 0));
	}

	InspectorIo::~InspectorIo() {
		if (state_ == State::kAccepting ||
			state_ == State::kConnected) {
			Stop();
		}
		uv_sem_destroy(&thread_start_sem_);
		uv_close(reinterpret_cast<uv_handle_t*>(&main_thread_req_->first), ReleasePairOnAsyncClose);
	}

	bool InspectorIo::Start() {
		Qk_Assert_Eq(state_, State::kNew);
		Qk_Assert_Eq(0, uv_thread_create(&thread_, InspectorIo::ThreadMain, this));
		uv_sem_wait(&thread_start_sem_);

		if (state_ == State::kError) {
			return false;
		}
		state_ = State::kAccepting;
		if (wait_for_connect_) {
			DispatchMessages();
		}
		return true;
	}

	void InspectorIo::Stop() {
		Qk_Assert(state_ == State::kAccepting || state_ == State::kConnected);
		Write(TransportAction::kKill, 0, StringView());
		int err = uv_thread_join(&thread_);
		Qk_Assert_Eq(err, 0);
		state_ = State::kShutDown;
		DispatchMessages();
	}

	bool InspectorIo::IsConnected() {
		return delegate_ != nullptr && delegate_->IsConnected();
	}

	bool InspectorIo::IsStarted() {
		return agent_->IsStarted();
	}

	void InspectorIo::WaitForDisconnect() {
		if (state_ == State::kAccepting)
			state_ = State::kDone;
		if (state_ == State::kConnected) {
			state_ = State::kShutDown;
			Write(TransportAction::kStop, 0, StringView());
			fprintf(stderr, "Waiting for the debugger to disconnect...\n");
			fflush(stderr);
			agent_->RunMessageLoop();
		}
	}

	// static
	void InspectorIo::ThreadMain(void* io) {
		static_cast<InspectorIo*>(io)->ThreadMain<InspectorSocketServer>();
	}

	// static
	template <typename Transport>
	void InspectorIo::IoThreadAsyncCb(uv_async_t* async) {
		TransportAndIo<Transport>* transport_and_io =
				static_cast<TransportAndIo<Transport>*>(async->data);
		if (transport_and_io == nullptr) {
			return;
		}
		Transport* transport = transport_and_io->first;
		InspectorIo* io = transport_and_io->second;
		MessageQueue<TransportAction> outgoing_message_queue;
		io->SwapBehindLock(&io->outgoing_message_queue_, &outgoing_message_queue);
		for (const auto& outgoing : outgoing_message_queue) {
			switch (std::get<0>(outgoing)) {
			case TransportAction::kKill:
				transport->TerminateConnections();
				// Fallthrough
			case TransportAction::kStop:
				transport->Stop(nullptr);
				break;
			case TransportAction::kSendMessage:
				std::string message = StringViewToUtf8(std::get<2>(outgoing)->string());
				transport->Send(std::get<1>(outgoing), message);
				break;
			}
		}
	}

	template<typename Transport>
	void InspectorIo::ThreadMain() {
		uv_loop_t loop;
		loop.data = nullptr;
		Qk_Assert_Eq(uv_loop_init(&loop), 0);
		thread_req_.data = nullptr;
		Qk_Assert_Eq(uv_async_init(&loop, &thread_req_, IoThreadAsyncCb<Transport>), 0);
		auto scropt_path = agent_->options().script_path.c_str();
		InspectorIoDelegate delegate(this,
			scropt_path, fs_basename(scropt_path).c_str(), wait_for_connect_);
		delegate_ = &delegate;
		Transport server(&delegate, &loop,
			agent_->options().host_name.c_str(), agent_->options().port);
		TransportAndIo<Transport> queue_transport(&server, this);
		thread_req_.data = &queue_transport;
		if (!server.Start()) {
			state_ = State::kError;  // Safe, main thread is waiting on semaphore
			Qk_Assert_Eq(0, CloseAsyncAndLoop(&thread_req_));
			uv_sem_post(&thread_start_sem_);
			return;
		}
		port_ = server.Port();  // Safe, main thread is waiting on semaphore.
		if (!wait_for_connect_) {
			uv_sem_post(&thread_start_sem_);
		}
		uv_run(&loop, UV_RUN_DEFAULT);
		thread_req_.data = nullptr;
		Qk_Assert_Eq(uv_loop_close(&loop), 0);
		delegate_ = nullptr;
	}

	template <typename ActionType>
	bool InspectorIo::AppendMessage(MessageQueue<ActionType>* queue,
																	ActionType action, int session_id,
																	std::unique_ptr<StringBuffer> buffer) {
		ScopeLock scoped_lock(state_lock_);
		bool trigger_pumping = queue->empty();
		queue->push_back(std::make_tuple(action, session_id, std::move(buffer)));
		return trigger_pumping;
	}

	template <typename ActionType>
	void InspectorIo::SwapBehindLock(MessageQueue<ActionType>* vector1,
																	MessageQueue<ActionType>* vector2) {
		ScopeLock scoped_lock(state_lock_);
		vector1->swap(*vector2);
	}

	void InspectorIo::PostIncomingMessage(InspectorAction action, int session_id,
																				const std::string& message) {
		if (AppendMessage(&incoming_message_queue_, action, session_id,
											Utf8ToStringView(message))) {
			// send async msg, call DispatchMessages()
			Qk_Assert_Eq(0, uv_async_send(&main_thread_req_->first));
		}
		NotifyMessageReceived();
	}

	void InspectorIo::NotifyMessageReceived() {
		ScopeLock scoped_lock(state_lock_);
		incoming_message_cond_.notify_all();
	}

	void InspectorIo::WaitForFrontendMessage() {
		dispatching_messages_ = false;
		{
			Lock scoped_lock(state_lock_);
			if (incoming_message_queue_.empty())
				incoming_message_cond_.wait(scoped_lock);
		}
		if (!incoming_message_queue_.empty()) {
			DispatchMessages();
		}
	}

	void InspectorIo::DispatchMessages() {
		// This function can be reentered if there was an incoming message while
		// V8 was processing another inspector request (e.g. if the user is
		// evaluating a long-running JS code snippet). This can happen only at
		// specific points (e.g. the lines that call inspector_ methods)
		if (dispatching_messages_)
			return;
		dispatching_messages_ = true;
		bool had_messages = false;
		do {
			if (dispatching_message_queue_.empty())
				SwapBehindLock(&incoming_message_queue_, &dispatching_message_queue_);
			had_messages = !dispatching_message_queue_.empty();
			while (!dispatching_message_queue_.empty()) {
				MessageQueue<InspectorAction>::value_type task;
				std::swap(dispatching_message_queue_.front(), task);
				dispatching_message_queue_.pop_front();
				StringView message = std::get<2>(task)->string();
				switch (std::get<0>(task)) {
				case InspectorAction::kStartSession:
					Qk_Assert_Eq(session_delegate_, nullptr);
					session_id_ = std::get<1>(task);
					state_ = State::kConnected;
					fprintf(stderr, "Debugger attached.\n");
					session_delegate_ = std::unique_ptr<InspectorSessionDelegate>(
							new IoSessionDelegate(this));
					agent_->Connect(session_delegate_.get());
					break;
				case InspectorAction::kEndSession:
					Qk_Assert_Ne(session_delegate_, nullptr);
					if (state_ == State::kShutDown) {
						state_ = State::kDone;
					} else {
						state_ = State::kAccepting;
					}
					agent_->Disconnect();
					session_delegate_.reset();
					break;
				case InspectorAction::kSendMessage:
					agent_->Dispatch(message);
					break;
				}
			}
		} while (had_messages);
		dispatching_messages_ = false;
	}

	// static
	void InspectorIo::MainThreadReqAsyncCb(uv_async_t* req) {
		AsyncAndAgent* pair = ContainerOf(&AsyncAndAgent::first, req);
		// Note that this may be called after io was closed or even after a new
		// one was created and ran.
		InspectorIo* io = pair->second->io();
		if (io != nullptr)
			io->DispatchMessages();
	}

	void InspectorIo::Write(TransportAction action, int session_id,
													const StringView& inspector_message) {
		//Qk_DEBUG("Send_To_Inspector, %s \n", StringViewToUtf8(inspector_message).c_str());
		AppendMessage(&outgoing_message_queue_, action, session_id,
									StringBuffer::create(inspector_message));
		int err = uv_async_send(&thread_req_);
		Qk_Assert_Eq(0, err);
	}

	InspectorIoDelegate::InspectorIoDelegate(InspectorIo* io,
																					const std::string& script_path,
																					const std::string& script_name,
																					bool wait)
																					: io_(io),
																						connected_(false),
																						session_id_(0),
																						script_name_(script_name),
																						script_path_(script_path),
																						target_id_(GenerateID()),
																						waiting_(wait) { }


	bool InspectorIoDelegate::StartSession(int session_id,
																				const std::string& target_id) {
		if (connected_)
			return false;
		connected_ = true;
		session_id_++;
		io_->PostIncomingMessage(InspectorAction::kStartSession, session_id, "");
		return true;
	}

	void InspectorIoDelegate::EndSession(int session_id) {
		connected_ = false;
		io_->PostIncomingMessage(InspectorAction::kEndSession, session_id, "");
	}

	void InspectorIoDelegate::MessageReceived(int session_id,
																						const std::string& message) {
		// TODO(pfeldman): Instead of blocking execution while debugger
		// engages, node should wait for the run callback from the remote client
		// and initiate its startup. This is a change to node.cc that should be
		// upstreamed separately.
		if (waiting_) {
			if (message.find("\"Runtime.runIfWaitingForDebugger\"") != std::string::npos) {
				waiting_ = false;
				io_->ResumeStartup();
			}
		}
		io_->PostIncomingMessage(InspectorAction::kSendMessage, session_id, message);
	}

	std::vector<std::string> InspectorIoDelegate::GetTargetIds() {
		return { target_id_ };
	}

	std::string InspectorIoDelegate::GetTargetTitle(const std::string& id) {
		return script_name_.empty() ? GetProcessTitle() : script_name_;
	}

	std::string InspectorIoDelegate::GetTargetUrl(const std::string& id) {
		return script_path_;
	}

	void IoSessionDelegate::WaitForFrontendMessage() {
		io_->WaitForFrontendMessage();
	}

	void IoSessionDelegate::SendMessageToFrontend(
			const v8_inspector::StringView& message) {
		io_->Write(TransportAction::kSendMessage, io_->session_id_, message);
	}

	std::vector<std::string> InspectorIo::GetTargetIds() const {
		return delegate_ ? delegate_->GetTargetIds() : std::vector<std::string>();
	}

}  // namespace inspector
}  // namespace node
