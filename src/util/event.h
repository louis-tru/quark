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

#ifndef __quark__util__event__
#define __quark__util__event__

#include "./util.h"
#include "./error.h"
#include "./list.h"
#include "./dict.h"
#include <functional>

#define Qk_Event(name, ...) \
	public: inline qk::EventNoticer<__VA_ARGS__>& on##name () { return _on##name; } \
	private:qk::EventNoticer<__VA_ARGS__> _on##name; public:

#define Qk_Init_Event(name)   _on##name(this)
#define Qk_On(name, ...)      on##name().on( __VA_ARGS__ )
#define Qk_Once(name, ...)    on##name().once( __VA_ARGS__ )
#define Qk_Off(name, ...)     on##name().off( __VA_ARGS__ )
#define Qk_Trigger(name, ...) on##name().trigger( __VA_ARGS__ )

namespace qk {
	class _Lock { // Empty Lock class
	public:
		inline void lock() {}
		inline void unlock() {}
	};

	template<class Sender = Object, class SendData = Object, class Origin = Object, typename RC = int> class Event;
	template<class Event = Event<>, class Lock = _Lock> class EventNoticer;

	template<class T_Sender, class T_SendData, class T_Origin, typename T_RC>
	class Event: public Object {
		Qk_HIDDEN_ALL_COPY(Event);
	public:
		typedef T_SendData       SendData;
		typedef T_Sender         Sender;
		typedef T_Origin         Origin;
		typedef T_RC             ReturnValue;
		typedef const SendData   cSendData;

		Event(cSendData& data = SendData(), Origin* origin = nullptr, const ReturnValue& rc = ReturnValue())
			: _sender(nullptr), _origin(origin), _data(&data), return_value(rc) {}

		inline Sender* sender() const { return _sender; }
		inline Origin* origin() const { return _origin; }
		inline cSendData* data() const { return _data; }

		// "new" method alloc can call，Otherwise, fatal exception will be caused
		virtual void release() {
			_sender = 0; _origin = 0; _data = 0;
			Object::release();
		}
	private:
		Sender      *_sender;
		Origin      *_origin;
		cSendData   *_data;
	public:
		ReturnValue return_value;
	};

	template<class Event, class Lock>
	class EventNoticer: public Lock {
		Qk_HIDDEN_ALL_COPY(EventNoticer);
	public:
		typedef Event EventType;
		typedef typename Event::SendData        SendData;
		typedef typename Event::cSendData       cSendData;
		typedef typename Event::Sender          Sender;
		typedef typename Event::Origin          Origin;
		typedef typename Event::ReturnValue     ReturnValue;
		typedef std::function<void(Event&)>     ListenerFunc;

		class Listener {
		public:
			inline Listener(EventNoticer* noticer, bool once)
				: _noticer(noticer), _once(once) {}
			virtual ~Listener() {}
			virtual void call(Event& evt) = 0;
			inline EventNoticer* host() { return _noticer; }
			inline bool once() { return _once; }
		protected:
			EventNoticer* _noticer;
			bool          _once;
		};

		// On
		template<class Scope> class OnListener: public Listener {
		public:
			typedef void (Scope::*ListenerFunc)(Event& evt);
			inline OnListener(EventNoticer* noticer, ListenerFunc listener, Scope* scope, bool once)
				: Listener(noticer,once), _listener(listener), _scope(scope) {}
			virtual void call(Event& evt) { (_scope->*_listener)(evt); }
			inline bool equals(ListenerFunc listener) { return _listener == listener; }
			inline bool equals(Scope* scope) { return _scope == scope; }
		protected:
			ListenerFunc _listener;
			Scope       *_scope;
		};

		// STATIC
		template<class Data> class OnStaticListener: public Listener {
		public:
			typedef void (*ListenerFunc)(Event& evt, Data* data);
			inline OnStaticListener(EventNoticer* noticer, ListenerFunc listener, Data* data, bool once)
				: Listener(noticer,once), _listener(listener), _data(data) {}
			virtual void call(Event& evt) { _listener(evt, _data); }
			inline bool equals(ListenerFunc listener) { return _listener == listener; }
			inline bool equals(Data* data) { return _data == data; }
		protected:
			ListenerFunc _listener;
			Data*        _data;
		};
		
		// Function
		class OnLambdaFunctionListener: public Listener {
		public:
			inline OnLambdaFunctionListener(EventNoticer* noticer, ListenerFunc&& listener, int id, bool once)
				: Listener(noticer,once), _listener(std::move(listener)), _id(id) {}
			virtual void call(Event& evt) { _listener(evt); }
			inline bool equals(int id) { return id == _id; }
		protected:
			ListenerFunc _listener;
			int          _id;
		};

		// SHELL
		class OnShellListener: public Listener {
		public:
			inline OnShellListener(EventNoticer* noticer, EventNoticer* shell, bool once)
				: Listener(noticer,once), _shell(shell) {}
			virtual void call(Event& evt) {
				this->_shell->trigger(evt);
				this->_noticer->set_event(evt);
			}
			inline bool equals(EventNoticer* shell) { return _shell == shell; }
		protected:
			EventNoticer* _shell;
		};
		
		EventNoticer(Sender *sender = nullptr)
			: _sender(sender), _listener(nullptr) {}

		~EventNoticer() {
			off2(true);
		}

		inline Sender* sender() const { return _sender; }

		inline int count() const {
			return _listener ? (int)_listener->length() : 0;
		}

		template<class Scope>
		void on(void (Scope::*listener)(Event&), Scope* scope) {
			add_listener(new OnListener<Scope>(this, listener, scope, 0));
		}

		template<class Scope>
		void once(void (Scope::*listener)(Event&), Scope* scope) {
			add_listener(new OnListener<Scope>(this, listener, scope, 1));
		}

		template <class Data>
		void on( void (*listener)(Event&, Data*), Data* data = nullptr) {
			add_listener(new OnStaticListener<Data>(this, listener, data, 0));
		}

		template <class Data>
		void once( void (*listener)(Event&, Data*), Data* data = nullptr) {
			add_listener(new OnStaticListener<Data>(this, listener, data, 1));
		}

		void on( ListenerFunc listener, int id = 0) {
			add_listener(new OnLambdaFunctionListener(this, std::move(listener), id, 0));
		}

		void once( ListenerFunc listener, int id = 0) {
			add_listener(new OnLambdaFunctionListener(this, std::move(listener), id, 1));
		}

		void on(EventNoticer* shell) {
			add_listener(new OnShellListener(this, shell, 0));
		}

		void once(EventNoticer* shell) {
			add_listener(new OnShellListener(this, shell, 1));
		}

		template<class Scope>
		void off( void (Scope::*listener)(Event&), Scope* scope) {
			if (_listener) {
				this->lock(); auto l = _listener;
				typedef OnListener<Scope> OnListener2;
				for ( auto &i : *l ) {
					if( i &&
							(!listener || static_cast<OnListener2*>(i)->equals(listener)) &&
							(!scope    || static_cast<OnListener2*>(i)->equals(scope)) ) {
						delete i; i = nullptr;
						break;
					}
				}
				this->unlock();
			}
		}

		void off(int id) {
			if (_listener) {
				this->lock(); auto l = _listener;
				for ( auto &i : *l ) {
					if ( i && static_cast<OnLambdaFunctionListener*>(i)->equals(id) ) {
						delete i; i = nullptr;
					}
				}
				this->unlock();
			}
		}

		template<class Scope>
		void off( void (Scope::*listener)(Event&) ) {
			off(listener, nullptr);
		}

		template<class Scope>
		void off(Scope* scope) {
			off((void (Object::*)(Event&))nullptr, scope);
		}
		
		template<class Data>
		void off( void (*listener)(Event&, Data*) ) {
			off(listener, nullptr);
		}

		template<class Data>
		void off( void (*listener)(Event&, Data*), Data* data) {
			/*if (_listener) {
				typedef OnStaticListener<Data> OnListener2;
				for ( auto &i : *_listener ) {
					if ( i &&
							static_cast<OnListener2*>(i)->equals(listener) &&
							static_cast<OnListener2*>(i)->equals(data) ) {
						delete i; i = nullptr;
						break;
					}
				}
			}*/
			off((void (Data::*)(Event&))listener, data);
		}
		
		void off(EventNoticer* shell) {
			/*if (_listener) {
				for ( auto &i : *_listener ) {
					if ( i && static_cast<OnShellListener*>(i)->equals(shell) )
					{ //
						delete i; i = nullptr;
						break;
					}
				}
			}*/
			off((void (Object::*)(Event&))shell, nullptr);
		}

		void off() {
			off2(false);
		}

		void trigger() {
			if (_listener) {
				Event evt;
				trigger(evt);
			}
		}

		void trigger(cSendData& data) {
			if (_listener) {
				Event evt(data);
				trigger(evt);
			}
		}

		void trigger(Event& evt) {
			if (_listener) {
				this->lock(); auto /*register c++17*/ l = _listener;
				if (l->length()) {
					set_event(evt);
					for (auto i = l->begin(); i != l->end(); ) {
						auto j = i++;
						auto listener = *j;
						if ( listener ) {
							listener->call(evt);
							if (listener->once())
								l->erase(j);
						} else {
							l->erase(j);
						}
					}
				}
				this->unlock();
			}
		}

	private:
		inline void set_event(Event& evt) {
			struct Ev: public Object { void *_sender; };
			reinterpret_cast<Ev*>(&evt)->_sender = _sender;
		}

		void off2(bool destroy) {
			if (_listener) {
				this->lock(); auto l = _listener;
				for ( auto &i : *l ) {
					delete i; i = nullptr;
				}
				if (destroy) {
					Release(l); _listener = nullptr;
				}
				this->unlock();
			}
		}

		inline void add_listener(Listener *l) {
			this->lock();
			if (!_listener)
				_listener = new List<Listener*>;
			_listener->push_back(l);
			this->unlock();
		}

		Sender          *_sender;
		List<Listener*> * volatile _listener;

		friend class  OnShellListener;
	};

	/**
	* @class Notification
	*/
	template<
		class Event = Event<>,
		class Name  = String,
		class Basic = Object
	>
	class Notification: public Basic {
		Qk_HIDDEN_ALL_COPY(Notification);
	public:
		typedef Event               EventType;
		typedef Name                NameType;
		typedef EventNoticer<Event> Noticer;
		typedef typename Noticer::ListenerFunc ListenerFunc;

		template<typename... Args>
		inline Notification(Args... args)
			: Basic(args...), _noticers(nullptr) {
		}

		virtual ~Notification() {
			if ( _noticers ) {
				for (auto& i: *_noticers)
					delete i.value;
				Release(_noticers); _noticers = nullptr;
			}
		}

		Noticer* get_noticer(const Name& name) const {
			if ( _noticers != nullptr ) {
				auto it = _noticers->find(name);
				if (it != _noticers->end()) {
					return it->value;
				}
			}
			return nullptr;
		}

		bool has_noticer(const Name& name) const {
			if ( _noticers != nullptr ) {
				return _noticers->find(name) != _noticers->end();
			}
			return false;
		}
		
		/**
		* 是否没有任何事件
		* @ret {bool}
		*/
		inline bool is_noticer_none() const {
			return _noticers == nullptr || _noticers->size() == 0;
		}
		
		/**
		* 侦听器变化会通知到该函数,比如添加删除事件侦听器
		* @arg name {const Type&}
		* @arg count {int}
		*/
		virtual void trigger_listener_change(const Name& name, int count, int change) {}
		
		template<class Scope>
		inline void add_event_listener(const Name& name,
									void (Scope::*listener)(Event&),
									Scope* scope) {
			auto del = get_noticer_no_null(name);
			del->on(listener, scope);
			trigger_listener_change(name, del->count(), 1);
		}
		
		template<class Scope>
		inline void add_event_listener_once(const Name& name,
										void (Scope::*listener)(Event&),
										Scope* scope) {
			auto del = get_noticer_no_null(name);
			del->once(listener, scope);
			trigger_listener_change(name, del->count(), 1);
		}
		
		template<class Data>
		inline void add_event_listener(const Name& name,
									void (*listener)(Event&, Data*),
									Data* data = nullptr) {
			auto del = get_noticer_no_null(name);
			del->once(listener, data);
			trigger_listener_change(name, del->count(), 1);
		}
		
		template<class Data>
		inline void add_event_listener_once(const Name& name,
										void (*listener)(Event&, Data*),
										Data* data = nullptr) {
			auto del = get_noticer_no_null(name);
			del->once(listener, data);
			trigger_listener_change(name, del->count(), 1);
		}
		
		inline void add_event_listener( const Name& name, ListenerFunc listener, int id = 0) {
			auto del = get_noticer_no_null(name);
			del->on(listener, id);
			trigger_listener_change(name, del->count(), 1);
		}
		
		inline void add_event_listener_once( const Name& name, ListenerFunc listener, int id = 0) {
			auto del = get_noticer_no_null(name);
			del->once(listener, id);
			trigger_listener_change(name, del->count(), 1);
		}
		
		inline void add_event_listener(const Name& name, Noticer* shell) {
			auto del = get_noticer_no_null(name);
			del->on(shell);
			trigger_listener_change(name, del->count(), 1);
		}
		
		/**
		* 添加一个侦听器,只侦听一次,后被卸载
		*/
		inline void add_event_listener_once(const Name& name, Noticer* shell) {
			auto del = get_noticer_no_null(name);
			del->once(shell);
			trigger_listener_change(name, del->count(), 1);
		}
		
		template<class Scope>
		inline void remove_event_listener(const Name& name,
										void (Scope::*listener)(Event&)
										) {
			auto del = get_noticer(name);
			if (del) {
				del->off(listener);
				trigger_listener_change(name, del->count(), -1);
			}
		}
		
		template<class Scope>
		inline void remove_event_listener(const Name& name,
										void (Scope::*listener)(Event&), Scope* scope) {
			auto del = get_noticer(name);
			if (del) {
				del->off(listener, scope);
				trigger_listener_change(name, del->count(), -1);
			}
		}
		
		template<class Data>
		inline void remove_event_listener(const Name& name,
											void (*listener)(Event&, Data*)
										) {
			auto del = get_noticer(name);
			if (del) {
				del->off(listener);
				trigger_listener_change(name, del->count(), -1);
			}
		}
		
		template<class Data>
		inline void remove_event_listener(const Name& name,
										void (*listener)(Event&, Data*),
										Data* data) {
			auto del = get_noticer(name);
			if (del) {
				del->off(listener, data);
				trigger_listener_change(name, del->count(), -1);
			}
		}
		
		/**
		* @func off
		*/
		inline void remove_event_listener(const Name& name, int id) {
			auto del = get_noticer(name);
			if (del) {
				del->off(id);
				trigger_listener_change(name, del->count(), -1);
			}
		}
		
		/**
		* @func off
		*/
		inline void remove_event_listener(int id) {
			if (_noticers) {
				for ( auto& i : *_noticers ) {
					i.value.off(id);
					trigger_listener_change(i.key, i.value.count(), -1);
				}
			}
		}
		
		/**
		* 卸载这个范围里的所有侦听器
		*/
		template<class Scope> 
		inline void remove_event_listener(Scope* scope) {
			if (_noticers) {
				for ( auto& i : *_noticers ) {
					i.value.off(scope);
					trigger_listener_change(i.ket, i.value.count(), -1);
				}
			}
		}
		
		inline void remove_event_listener(const Name& name, Noticer* shell) {
			auto del = get_noticer(name);
			if (del) {
				del->off(shell);
				trigger_listener_change(name, del->count(), -1);
			}
		}
		
	protected:
		/**
		* 卸载指定事件名称上的全部侦听函数
		*/
		inline void remove_event_listener(const Name& name) {
			auto del = get_noticer(name);
			if (del) {
				del->off();
				trigger_listener_change(name, del->count(), -1);
			}
		}

		/**
		* 卸载全部侦听函数
		*/
		inline void remove_event_listener() {
			if (_noticers) {
				for ( auto i : *_noticers ) {
					i.value.off();
					trigger_listener_change(i.key, i.value.count(), -1);
				}
			}
		}

		/**
		* 触发事件
		* @arg name {const Key&}
		* NOTE: 这个方法能创建默认事件数据
		*/
		inline void trigger(const Name& name) {
			auto del = get_noticer(name);
			if (del) del->trigger();
		}

		/*
		* 触发事件
		* @arg name {const Key&}
		* @arg evt {cSendData&}
		*/
		inline void trigger(const Name& name, typename Noticer::cSendData& data) {
			auto del = get_noticer(name);
			if (del) del->trigger(data);
		}

		/*
		* 触发事件
		* @arg name {const Key&}
		* @arg evt {Event&}
		*/
		inline void trigger(const Name& name, Event& evt) {
			auto del = get_noticer(name);
			if (del) del->trigger(evt);
		}

	private:
		typedef Dict<Name, Noticer*> Noticers;

		Noticer* get_noticer_no_null(const Name& name) {
			if (_noticers == nullptr) {
				_noticers = new Noticers();
			}
			auto it = _noticers->find(name);
			if (it != _noticers->end()) {
				return it->value;
			} else {
				return _noticers->set(name, new Noticer(static_cast<typename Noticer::Sender*>(this)));
			}
		}

		Noticers* _noticers;
	};

}
#endif
