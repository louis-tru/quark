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

#include "./event.h"

namespace qk {

	typedef EventNoticerBasic Basic;
	typedef EventNoticerBasic::Listener Listener;
	typedef Basic::ListenerFunc ListenerFunc;
	typedef Basic::StaticListenerFunc StaticListenerFunc;
	typedef Basic::OnLambdaListenerFunc OnLambdaListenerFunc;
	
	Listener::Listener(uint32_t id, bool once)
		: _id(id), _once(once) {}
	Listener::~Listener() {}
	
	// hash code
	uint32_t ev_hash_code(ListenerFunc listener, void* ctx) {
		struct S {
			ListenerFunc listener; void* ctx;
		} s = {listener,ctx};
		return qk::hashCode(&s, sizeof(S)) % Uint32::limit_max;
	}

	uint32_t ev_hash_code(StaticListenerFunc listener, void* ctx) {
		struct S {
			StaticListenerFunc listener; void* ctx;
		} s = {listener,ctx};
		return qk::hashCode(&s, sizeof(S)) % Uint32::limit_max;
	}

	uint32_t ev_hash_code(Basic* shell) {
		return qk::hashCode(&shell, sizeof(Basic*)) % Uint32::limit_max;
	}

	bool Listener::match(ListenerFunc l, void* ctx) {
		return ev_hash_code(l, ctx) == _id;
	}

	bool Listener::match(StaticListenerFunc l, void* ctx) {
		return ev_hash_code(l, ctx) == _id;
	}

	// On
	class OnListener: public Listener {
	public:
		OnListener(ListenerFunc listener, void* ctx, bool once)
			: Listener(ev_hash_code(listener, ctx), once), _listener(listener), _ctx(ctx) {}
		virtual void call(Object& evt) override {
			(((Object*)_ctx)->*_listener)(evt);
		}
		virtual bool match(ListenerFunc l, void* ctx) override {
			if (l && l != _listener)
				return false;
			if (ctx && ctx != _ctx)
				return false;
			return true;
		}
	protected:
		ListenerFunc _listener;
		void        *_ctx;
	};

	// STATIC
	class OnStaticListener: public Listener {
	public:
		OnStaticListener(StaticListenerFunc listener, void* ctx, bool once)
			: Listener(ev_hash_code(listener, ctx),once)
			, _listener(listener), _ctx(ctx) {}
		virtual void call(Object& evt) override {
			_listener(evt, _ctx);
		}
		virtual bool match(StaticListenerFunc l, void* ctx) override {
			if (l && l != _listener)
				return false;
			if (ctx && ctx != _ctx)
				return false;
			return true;
		}
	protected:
		StaticListenerFunc _listener;
		void              *_ctx;
	};

	// Function
	class OnLambdaFunctionListener: public Listener {
	public:
		OnLambdaFunctionListener(OnLambdaListenerFunc& listener, uint32_t id, bool once)
			: Listener(id,once), _listener(std::move(listener)) {}
		virtual void call(Object& evt) override {
			_listener(evt);
		}
	protected:
		OnLambdaListenerFunc _listener;
	};

	// SHELL
	class OnShellListener: public Listener {
	public:
		static void set_event(Object& event, void *sender) {
			struct Ev: public Object { void *_sender; };
			reinterpret_cast<Ev*>(&event)->_sender = sender;
		}
		inline OnShellListener(void* host_sender, Basic* shell, bool once)
			: Listener(ev_hash_code(shell),once), _host_sender(host_sender), _shell(shell) {}
		virtual void call(Object& evt) {
			_shell->trigger_event(evt);
			set_event(evt, _host_sender);
		}
	protected:
		void  *_host_sender;
		Basic *_shell;
	};
	
	// -------------------------- EventNoticerBasic --------------------------

	// make listener
	Listener* Basic::MakeListener(ListenerFunc listener, void* ctx, bool once) {
		return new OnListener(listener,ctx,once);
	}
	Listener* Basic::MakeStaticListener(StaticListenerFunc listener, void* ctx, bool once) {
		return new OnStaticListener(listener,ctx,once);
	}
	Listener* Basic::MakeLambdaListener(OnLambdaListenerFunc& listener, uint32_t id, bool once) {
		return new OnLambdaFunctionListener(listener,id,once);
	}
	Listener* Basic::MakeShellListener(void *host_sender, Basic* shell, bool once) {
		return new OnShellListener(host_sender,shell,once);
	}

	Basic::EventNoticerBasic(void *sender)
		: _sender(sender), _listener(nullptr) {}

	Basic::~EventNoticerBasic() {
		off2(true);
	}
	
	int Basic::count() const {
		return _listener ? (int)_listener->length() : 0;
	}

	void Basic::off() {
		off2(false);
	}

	void Basic::off(uint32_t id) {
		if (_listener) {
			lock();
			auto l = _listener;
			for ( auto &i : *l ) {
				if ( i && i->id() == id ) {
					delete i; i = nullptr;
				}
			}
			unlock();
		}
	}
	
	void Basic::off_listener( ListenerFunc listener, void* ctx) {
		if (_listener) {
			lock();
			auto l = _listener;
			for ( auto &i : *l ) {
				if( i && i->match(listener, ctx) ) {
					delete i; i = nullptr;
					break;
				}
			}
			unlock();
		}
	}
	
	void Basic::off_listener(ListenerFunc l) {
		off_listener(l, nullptr);
	}
	
	void Basic::off_static( StaticListenerFunc listener, void* ctx) {
		if (_listener) {
			lock();
			auto l = _listener;
			for ( auto &i : *l ) {
				if( i && i->match(listener, ctx) ) {
					delete i; i = nullptr;
					break;
				}
			}
			unlock();
		}
	}
	
	void Basic::off_static(StaticListenerFunc l) {
		off_static(l, nullptr);
	}

	void Basic::off_for_ctx(void *ctx) {
		off_static(nullptr, ctx);
	}

	void Basic::off_shell(EventNoticerBasic* shell) {
		off(ev_hash_code(shell));
	}

	void Basic::trigger_event(Object& event) {
		if (_listener) {
			lock();
			auto /*register c++17*/ l = _listener;
			if (l->length()) {
				OnShellListener::set_event(event, _sender);
				for (auto i = l->begin(); i != l->end(); ) {
					auto j = i++;
					auto listener = *j;
					if ( listener ) {
						listener->call(event);
						if (listener->once()) {
							delete listener;
							l->erase(j);
						}
					} else {
						l->erase(j);
					}
				}
			}
			unlock();
		}
	}

	void Basic::off2(bool destroy) {
		if (_listener) {
			lock();
			auto l = _listener;
			for ( auto &i : *l ) {
				delete i; i = nullptr;
			}
			if (destroy) {
				Release(l); _listener = nullptr;
			}
			unlock();
		}
	}

	void Basic::add_listener(Listener *l) {
		lock();
		if (!_listener)
			_listener = new List<Listener*>;
		_listener->pushBack(l);
		unlock();
	}
	
	void Basic::lock() {}
	void Basic::unlock() {}

	// ---------------------------- NotificationBasic ----------------------------

	NotificationBasic::NotificationBasic(): _noticers(nullptr) {}
	
	NotificationBasic::~NotificationBasic() {
		if ( _noticers ) {
			for (auto& i: *_noticers)
				delete i.value;
			Release(_noticers); _noticers = nullptr;
		}
	}

	bool NotificationBasic::has_noticer(uint32_t name) const {
		if ( _noticers != nullptr )
			return _noticers->find(name) != _noticers->end();
		return false;
	}

	inline bool NotificationBasic::is_noticer_none() const {
		return _noticers == nullptr || _noticers->length() == 0;
	}

	void* NotificationBasic::event_noticer_sender() {
		return this;
	}

	void NotificationBasic::trigger_listener_change(uint32_t name, int count, int change) {}
	
	void NotificationBasic::add_event_listener(uint32_t name, Basic::Listener *l) {
		auto del = get_noticer(name, true);
		del->add_listener(l);
		trigger_listener_change(name, del->count(), 1);
	}

	// remove event listener

	void NotificationBasic::remove_event_listener(uint32_t name, void (Object::*listener)(Object&)) {
		auto del = get_noticer(name);
		if (del) {
			del->off_listener(listener);
			trigger_listener_change(name, del->count(), -1);
		}
	}
	
	void NotificationBasic::remove_event_listener(uint32_t name, void (Object::*listener)(Object&), void *ctx) {
		auto del = get_noticer(name);
		if (del) {
			del->off_listener(listener, ctx);
			trigger_listener_change(name, del->count(), -1);
		}
	}
	
	void NotificationBasic::remove_event_listener_static(uint32_t name, void (*listener)(Object&, void*)) {
		auto del = get_noticer(name);
		if (del) {
			del->off_static(listener);
			trigger_listener_change(name, del->count(), -1);
		}
	}

	void NotificationBasic::remove_event_listener_static(uint32_t name, void (*listener)(Object&, void*), void* ctx) {
		auto del = get_noticer(name);
		if (del) {
			del->off_static(listener, ctx);
			trigger_listener_change(name, del->count(), -1);
		}
	}
	
	void NotificationBasic::remove_event_listener_for_id(uint32_t name, uint32_t id) {
		auto del = get_noticer(name);
		if (del) {
			del->off(id);
			trigger_listener_change(name, del->count(), -1);
		}
	}
	
	void NotificationBasic::remove_event_listener_shell(uint32_t name, Basic *shell) {
		auto del = get_noticer(name);
		if (del) {
			del->off_shell(shell);
			trigger_listener_change(name, del->count(), -1);
		}
	}
	
	void NotificationBasic::remove_event_listener_for_ctx(void* ctx) {
		if (_noticers) {
			for ( auto& i : *_noticers ) {
				i.value->off_for_ctx(ctx);
				trigger_listener_change(i.key, i.value->count(), -1);
			}
		}
	}

	void NotificationBasic::remove_event_listener_for_id(uint32_t id) {
		if (_noticers) {
			for ( auto& i : *_noticers ) {
				i.value->off(id);
				trigger_listener_change(i.key, i.value->count(), -1);
			}
		}
	}
	
	Basic* NotificationBasic::get_noticer(uint32_t name, bool no_null) {
		if ( _noticers == nullptr ) {
			if (!no_null)
				return nullptr;
			_noticers = new Dict<uint32_t, Basic*>();
		}
		auto it = _noticers->find(name);
		if (it == _noticers->end()) {
			if (!no_null)
				return nullptr;
			return _noticers->set(name, new Basic(event_noticer_sender()));
		} else {
			return it->value;
		}
	}

	void NotificationBasic::remove_event_listener_for_name(uint32_t name) {
		auto del = get_noticer(name);
		if (del) {
			del->off();
			trigger_listener_change(name, del->count(), -1);
		}
	}

	void NotificationBasic::remove_event_listener() {
		if (_noticers) {
			for ( auto i : *_noticers ) {
				i.value->off();
				trigger_listener_change(i.key, i.value->count(), -1);
			}
		}
	}

}
