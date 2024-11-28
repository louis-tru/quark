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

#ifndef SRC_INSPECTOR_SOCKET_H_
#define SRC_INSPECTOR_SOCKET_H_

#include "../../util/macros.h"
#include "inspector_agent.h"
#include <http_parser.h>
#include <uv.h>
#include <string>
#include <vector>

namespace qk { namespace inspector {

	enum inspector_handshake_event {
		kInspectorHandshakeUpgrading,
		kInspectorHandshakeUpgraded,
		kInspectorHandshakeHttpGet,
		kInspectorHandshakeFailed
	};

	class InspectorSocket;

	typedef void (*inspector_cb)(InspectorSocket*, int);
	// Notifies as handshake is progressing. Returning false as a response to
	// kInspectorHandshakeUpgrading or kInspectorHandshakeHttpGet event will abort
	// the connection. inspector_write can be used from the callback.
	typedef bool (*handshake_cb)(InspectorSocket*,
															enum inspector_handshake_event state,
															const std::string& path);

	struct http_parsing_state_s {
		http_parser parser;
		http_parser_settings parser_settings;
		handshake_cb callback;
		bool done;
		bool parsing_value;
		std::string ws_key;
		std::string path;
		std::string current_header;
	};

	struct ws_state_s {
		uv_alloc_cb alloc_cb;
		uv_read_cb read_cb;
		inspector_cb close_cb;
		bool close_sent;
		bool received_close;
	};

	// HTTP Mixper around a uv_tcp_t
	class InspectorSocket {
		Qk_HIDDEN_ALL_COPY(InspectorSocket);
	public:
		InspectorSocket() : data(nullptr), http_parsing_state(nullptr),
												ws_state(nullptr), buffer(0), ws_mode(false),
												shutting_down(false), connection_eof(false) {}
		void reinit();
		void* data;
		struct http_parsing_state_s* http_parsing_state;
		struct ws_state_s* ws_state;
		std::vector<char> buffer;
		uv_tcp_t tcp;
		bool ws_mode;
		bool shutting_down;
		bool connection_eof;
	};

	int inspector_accept(uv_stream_t* server, InspectorSocket* inspector,
											handshake_cb callback);

	void inspector_close(InspectorSocket* inspector,
											inspector_cb callback);

	// Callbacks will receive stream handles. Use inspector_from_stream to get
	// InspectorSocket* from the stream handle.
	int inspector_read_start(InspectorSocket* inspector, uv_alloc_cb,
														uv_read_cb);
	void inspector_read_stop(InspectorSocket* inspector);
	void inspector_write(InspectorSocket* inspector,
			const char* data, size_t len);
	bool inspector_is_active(const InspectorSocket* inspector);

	inline InspectorSocket* inspector_from_stream(uv_tcp_t* stream) {
		return ::qk::inspector::ContainerOf(&InspectorSocket::tcp, stream);
	}

	inline InspectorSocket* inspector_from_stream(uv_stream_t* stream) {
		return inspector_from_stream(reinterpret_cast<uv_tcp_t*>(stream));
	}

	inline InspectorSocket* inspector_from_stream(uv_handle_t* stream) {
		return inspector_from_stream(reinterpret_cast<uv_tcp_t*>(stream));
	}

}  // namespace inspector
}  // namespace node


#endif  // SRC_INSPECTOR_SOCKET_H_
