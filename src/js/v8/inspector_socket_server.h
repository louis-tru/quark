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

// @private head

#ifndef SRC_INSPECTOR_SOCKET_SERVER_H_
#define SRC_INSPECTOR_SOCKET_SERVER_H_

#include "inspector_agent.h"
#include "inspector_socket.h"
#include <uv.h>
#include <map>
#include <string>
#include <vector>

namespace qk { namespace inspector {

	class Closer;
	class SocketSession;
	class ServerSocket;

	class SocketServerDelegate {
	public:
		virtual bool StartSession(int session_id, const std::string& target_id) = 0;
		virtual void EndSession(int session_id) = 0;
		virtual void MessageReceived(int session_id, const std::string& message) = 0;
		virtual std::vector<std::string> GetTargetIds() = 0;
		virtual std::string GetTargetTitle(const std::string& id) = 0;
		virtual std::string GetTargetUrl(const std::string& id) = 0;
		virtual void ServerDone() = 0;
	};

	// HTTP Server, writes messages requested as TransportActions, and responds
	// to HTTP requests and WS upgrades.

	class InspectorSocketServer {
	public:
		using ServerCallback = void (*)(InspectorSocketServer*);
		InspectorSocketServer(SocketServerDelegate* delegate,
													uv_loop_t* loop,
													const std::string& host,
													int port,
													FILE* out = stderr);
		// Start listening on host/port
		bool Start();

		// Called by the TransportAction sent with InspectorIo::Write():
		//   kKill and kStop
		void Stop(ServerCallback callback);
		//   kSendMessage
		void Send(int session_id, const std::string& message);
		//   kKill
		void TerminateConnections();

		int Port() const;

		// Server socket lifecycle. There may be multiple sockets
		void ServerSocketListening(ServerSocket* server_socket);
		void ServerSocketClosed(ServerSocket* server_socket);

		// Session connection lifecycle
		bool HandleGetRequest(InspectorSocket* socket, const std::string& path);
		bool SessionStarted(SocketSession* session, const std::string& id);
		void SessionTerminated(SocketSession* session);
		void MessageReceived(int session_id, const std::string& message) {
			delegate_->MessageReceived(session_id, message);
		}

		int GenerateSessionId() {
			return next_session_id_++;
		}

	private:
		void SendListResponse(InspectorSocket* socket);
		bool TargetExists(const std::string& id);

		enum class ServerState {kNew, kRunning, kStopping, kStopped};
		uv_loop_t* loop_;
		SocketServerDelegate* const delegate_;
		const std::string host_;
		int port_;
		std::string path_;
		std::vector<ServerSocket*> server_sockets_;
		Closer* closer_;
		std::map<int, SocketSession*> connected_sessions_;
		int next_session_id_;
		FILE* out_;
		ServerState state_;

		friend class Closer;
	};

}  // namespace inspector
}  // namespace node

#endif  // SRC_INSPECTOR_SOCKET_SERVER_H_
