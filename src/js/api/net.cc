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

#include "../js_.h"
#include "../../util/net.h"

namespace qk { namespace js {

	struct SocketDelegate: Object, qk::Socket::Delegate {
		MixObject* _host; // MixSocket
		String _open;
		String _close;
		String _error;
		String _data;
		String _write;
		String _timeout;

		Worker* worker() { return _host->worker(); }

		virtual void trigger_socket_open(qk::Socket* socket) override {
			if ( !_open.isEmpty() ) {
				HandleScope scope(worker());
				_host->call( worker()->newStringOneByte(_open) );
			}
		}
		virtual void trigger_socket_close(qk::Socket* socket) override {
			if ( !_close.isEmpty() ) {
				HandleScope scope(worker());
				_host->call( worker()->newStringOneByte(_close) );
			}
			socket->release(); // TODO: js handle set weak object
		}
		virtual void trigger_socket_error(qk::Socket* socket, cError& error) override {
			if ( !_error.isEmpty() ) {
				HandleScope scope(worker());
				JSValue* arg = worker()->newValue( error );
				_host->call( worker()->newStringOneByte(_error), 1, &arg );
			}
			if (!socket->is_open()) {
				socket->release(); // TODO: js handle set weak object
			}
		}
		virtual void trigger_socket_data(qk::Socket* socket, cBuffer &buffer) override {
			if ( !_data.isEmpty() ) {
				HandleScope scope(_host->worker());
				JSValue* arg = worker()->newValue( buffer.copy() );
				_host->call( worker()->newStringOneByte(_data), 1, &arg ); 
			}
		}
		virtual void trigger_socket_write(qk::Socket* socket, Buffer& data, int flag) override {
			if ( !_write.isEmpty() ) {
				HandleScope scope(worker());
				JSValue* arg = worker()->newValue( flag );
				_host->call( worker()->newStringOneByte(_write), 1, &arg);
			}
		}
		virtual void trigger_socket_timeout(qk::Socket* socket) override {
			if ( !_timeout.isEmpty() ) {
				HandleScope scope(worker());
				_host->call( worker()->newStringOneByte(_timeout) );
			}
		}
	};

	struct JsSocket: qk::Socket {
		JsSocket(cString& hostname, int port, bool isSSL):
			qk::Socket(hostname, port, isSSL)
		{
			init_del();
		}
		void init_del() {
			_del._host = reinterpret_cast<MixObject*>(this) - 1;
			set_delegate(&_del);
		}
		SocketDelegate _del;
	};

	struct MixSocket: MixObject {
		typedef JsSocket Type;

		virtual bool addEventListener(cString& name, cString& func, int id) {
			auto _del = &self<Type>()->_del;
			if ( id != 0 )
				return 0; // 只接收id==0的监听器
			if ( name == "Open" ) {
				_del->_open = func;
			} else if ( name == "Close" ) {
				_del->_close = func;
			} else if ( name == "Error" ) {
				_del->_error = func;
			} else if ( name == "Write" ) {
				_del->_write = func;
			} else if ( name == "Data" ) {
				_del->_data = func;
			} else if ( name == "Timeout" ) {
				_del->_timeout = func;
			} else {
				return false;
			}
			return true;
		}

		virtual bool removeEventListener(cString& name, int id) {
			auto _del = &self<Type>()->_del;
			if ( id != 0 || !_del )
				return 0;
			if ( name == "Open" ) {
				_del->_open = String();
			} else if ( name == "Close" ) {
				_del->_close = String();
			} else if ( name == "Error" ) {
				_del->_error = String();
			} else if ( name == "Write" ) {
				_del->_write = String();
			} else if ( name == "Data" ) {
				_del->_data = String();
			} else if ( name == "Timeout" ) {
				_del->_timeout = String();
			} else {
				return false;
			}
			return true;
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Socket, 0, {
				if (args.length() < 1 || !args[0]->isString() || !args[1]->isUint32()) {
					Js_Throw(
						"@constructor SocketConnect(hostname,port,isSSL?)\n"
						"@param hostname:string\n"
						"@param port:Uint\n"
						"@param isSSL?:boolean\n"
					);
				}
				auto hostname = args[0]->toString(worker)->value(worker);
				auto port = args[1]->template cast<JSUint32>()->value();
				auto isSSL = args.length() > 2 ? args[2]->toBoolean(worker): false;
				New<MixSocket>(args, new Type(hostname, port, isSSL));
			});

			Js_Class_Accessor_Get(hostname, {
				Js_Self(Type);
				Js_Return( self->hostname() );
			});

			Js_Class_Accessor_Get(port, {
				Js_Self(Type);
				Js_Return( self->port() );
			});

			Js_Class_Accessor_Get(ip, {
				Js_Self(Type);
				Js_Return( self->ip() );
			});

			Js_Class_Accessor_Get(ipv6, {
				Js_Self(Type);
				Js_Return( self->ipv6() );
			});

			Js_Class_Accessor_Get(isOpen, {
				Js_Self(Type);
				Js_Return( self->is_open() );
			});

			Js_Class_Accessor_Get(isConnecting, {
				Js_Self(Type);
				Js_Return( self->is_connecting() );
			});

			Js_Class_Accessor_Get(isPause, {
				Js_Self(Type);
				Js_Return( self->is_pause() );
			});

			Js_Class_Method(setKeepAlive, {
				if (args.length() == 0/* || (args.length() > 1 && !args[1]->isNumber())*/) {
					Js_Throw(
						"@method setKeepAlive(keep_alive,keep_idle?)\n"
						"@param keep_alive:bool\n"
						"@param keep_idle?:Uint\n"
					);
				}
				Js_Self(Type);
				bool enable = args[0]->toBoolean(worker);
				uint32_t keep_idle = 0;
				if (args.length() > 1 && args[1]->asUint32(worker).to(keep_idle)) {}

				self->set_keep_alive(enable, keep_idle * 1e3);
			});

			Js_Class_Method(setNoDelay, {
				Js_Self(Type);
				bool no_delay = args.length() == 0 ? true: args[0]->toBoolean(worker);
				self->set_no_delay(no_delay);
			});

			Js_Class_Method(setTimeout, {
				if (args.length() == 0 || !args[0]->isNumber()) {
					Js_Throw(
						"@method setTimeout(time)\n"
						"@param time:Uint\n"
					);
				}
				Js_Self(Type);
				uint64_t time = args[0]->toUint32(worker)->value() * 1000;
				self->set_timeout(time);
			});

			Js_Class_Method(connect, {
				Js_Self(Type);
				self->retain(); // TODO: js handle keep active
				self->connect();
			});

			Js_Class_Method(close, {
				Js_Self(Type);
				self->close();
			});

			Js_Class_Method(pause, {
				Js_Self(Type);
				self->pause();
			});

			Js_Class_Method(resume, {
				Js_Self(Type);
				self->resume();
			});

			Js_Class_Method(write, {
				WeakBuffer wbuff;
				if (args.length() == 0 ||
					!(args[0]->asBuffer(worker).to(wbuff) || args[0]->isString())
				) {
					Js_Throw(
						"@method write(buff,flag?)\n"
						"@param buff:Uint8Array|string\n"
						"@param flag?:number\n"
						"@param cb?:Function\n"
					);
				}
				Buffer buff = *wbuff ? wbuff.buffer().copy():
					args[0]->toString(worker)->value(worker).collapse();

				int flag = 0;
				Cb cb;

				if (args.length() > 1) {
					args[1]->asInt32(worker).to(flag);
				}
				if (args.length() > 2) {
					cb = get_callback_for_none(worker, args[2]);
				}

				Js_Self(Type);
				self->write(std::move(buff), flag, *reinterpret_cast<Callback<Buffer>*>(&cb));
			});

			Js_Class_Method(disableSslVerify, {
				if (args.length() == 0) {
					Js_Throw(
						"@method disableSslVerify(disable)\n"
						"@param disable:boolean\n"
					);
				}
				bool val = args[0]->toBoolean(worker);
				Js_Self(Type);
				self->disable_ssl_verify(val);
			});

			cls->exports("Socket", exports);
		}
	};

	struct NativeNet {
		static void binding(JSObject* exports, Worker* worker) {
			MixSocket::binding(exports, worker);
		}
	};

	Js_Module(_net, NativeNet);
} }
