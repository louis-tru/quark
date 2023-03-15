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

	class Qk_EXPORT EventNoticerBasic {
		Qk_HIDDEN_ALL_COPY(EventNoticerBasic);
	public:
		typedef void (Object::*ListenerFunc)(Object& evt);
		typedef void (*StaticListenerFunc)(Object& evt, void* data);
		typedef std::function<void(Object&)> OnLambdaListenerFunc;

		class Qk_EXPORT Listener {
		public:
			Listener(uint32_t hash, bool once);
			virtual ~Listener();
			inline uint32_t hash_code() const { return _hash; }
			inline bool once() const { return _once; }
			virtual void call(Object& evt) = 0;
			virtual bool match(ListenerFunc l, void* ctx);
			virtual bool match(StaticListenerFunc l, void* ctx);
		protected:
			uint32_t  _hash;
			bool      _once;
		};
		// make listener
		static Listener* MakeListener(ListenerFunc l, void* ctx, bool once);
		static Listener* MakeStaticListener(StaticListenerFunc l, Object* ctx, bool once);
		static Listener* MakeLambdaListener(OnLambdaListenerFunc& l, uint32_t hash, bool once);
		static Listener* MakeShellListener(void *host_sender, EventNoticerBasic* shell, bool once);

		EventNoticerBasic(void *sender);
		virtual ~EventNoticerBasic();
		int count() const;
		void off_listener(ListenerFunc l, void* ctx);
		void off_listener(ListenerFunc l);
		void off_static(StaticListenerFunc l, void* ctx);
		void off_static(StaticListenerFunc l);
		void off_for_ctx(void *ctx);
		void off_shell(EventNoticerBasic* shell);
		void off(uint32_t hash);
		void off(); // off all
		void trigger_event(Object& event);
		void add_listener(Listener *l);
	protected:
		virtual void lock(); // thread safe lock
		virtual void unlock();
		void off2(bool destroy);
		void            *_sender;
		List<Listener*> * volatile _listener;
	};

	template<class Event, class Lock>
	class EventNoticer: public EventNoticerBasic, public Lock {
	public:
		typedef Event EventType;
		typedef typename Event::SendData        SendData;
		typedef typename Event::cSendData       cSendData;
		typedef typename Event::Sender          Sender;
		typedef typename Event::Origin          Origin;
		typedef typename Event::ReturnValue     ReturnValue;
		
		EventNoticer(Sender *sender = nullptr)
			: EventNoticerBasic(sender) {}

		inline Sender* sender() const { return static_cast<Sender*>(_sender); }

		template<class Ctx>
		void on(void (Ctx::*listener)(Event&), Ctx* ctx) {
			add_listener(MakeListener((ListenerFunc)listener, ctx, 0));
		}

		template<class Ctx>
		void once(void (Ctx::*listener)(Event&), Ctx* ctx) {
			add_listener(MakeListener((ListenerFunc)listener, ctx, 1));
		}

		template <class Ctx>
		void on( void (*listener)(Event&, Ctx*), Ctx* ctx = nullptr) {
			add_listener(MakeStaticListener((StaticListenerFunc)listener, ctx, 0));
		}

		template <class Ctx>
		void once( void (*listener)(Event&, Ctx*), Ctx* ctx = nullptr) {
			add_listener(MakeStaticListener((StaticListenerFunc)listener, ctx, 1));
		}

		void on( std::function<void(Event&)> listener, int hash = 0) {
			add_listener(MakeLambdaListener(*(OnLambdaListenerFunc*)(&listener), hash, 0));
		}

		void once( std::function<void(Event&)> listener, int hash = 0) {
			add_listener(MakeLambdaListener(*(OnLambdaListenerFunc*)(&listener), hash, 1));
		}

		void on(EventNoticer *shell) {
			add_listener(MakeShellListener(_sender, shell, 0));
		}

		void once(EventNoticer *shell) {
			add_listener(MakeShellListener(_sender, shell, 1));
		}

		template<class Ctx>
		void off( void (Ctx::*listener)(Event&), Ctx* ctx) {
			off_listener((ListenerFunc)listener, ctx);
		}

		template<class Ctx>
		void off( void (Ctx::*listener)(Event&) ) {
			off_listener((ListenerFunc)listener);
		}

		template<class Ctx>
		void off(Ctx* ctx) {
			off_for_ctx(ctx);
		}
		
		template<class Ctx>
		void off( void (*listener)(Event&, Ctx*) ) {
			off_static((StaticListenerFunc)listener);
		}

		template<class Ctx>
		void off( void (*listener)(Event&, Ctx*), Ctx* ctx) {
			off_static((StaticListenerFunc)listener, ctx);
		}
		
		void off(EventNoticer* shell) {
			off_shell(shell);
		}

		void trigger() {
			if (_listener) {
				Event evt;
				trigger_event(evt);
			}
		}

		void trigger(cSendData& data) {
			if (_listener) {
				Event evt(data);
				trigger_event(evt);
			}
		}
		
		void trigger(Event& event) {
			trigger_event(event);
		}
		
	protected:
		virtual void lock() override {
			Lock::lock();
		}
		virtual void unlock() override {
			Lock::unlock();
		}
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
		typedef EventNoticerBasic   _B;

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
			if ( _noticers != nullptr )
				return _noticers->find(name) != _noticers->end();
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
		
		inline void add_event_listener(const Name& name, _B::Listener *l) {
			auto del = get_noticer_no_null(name);
			del->add_listener(l);
			trigger_listener_change(name, del->count(), 1);
		}

		template<class Ctx>
		inline void add_event_listener(const Name& name,
									void (Ctx::*listener)(Event&), Ctx* ctx) {
			add_event_listener(name, _B::MakeListener((_B::ListenerFunc)listener, ctx, 0));
		}
		
		template<class Ctx>
		inline void add_event_listener_once(const Name& name,
										void (Ctx::*listener)(Event&), Ctx* ctx) {
			add_event_listener(name, _B::MakeListener((_B::ListenerFunc)listener, ctx, 1));
		}
		
		template<class Ctx>
		inline void add_event_listener(const Name& name,
									void (*listener)(Event&, Ctx*), Ctx* ctx = nullptr) {
			add_event_listener(name, _B::MakeListener((_B::StaticListenerFunc)listener, ctx, 0));
		}
		
		template<class Ctx>
		inline void add_event_listener_once(const Name& name,
										void (*listener)(Event&, Ctx*), Ctx* ctx = nullptr) {
			add_event_listener(name, _B::MakeListener((_B::StaticListenerFunc)listener, ctx, 1));
		}
		
		inline void add_event_listener( const Name& name, std::function<void(Event&)> listener, uint32_t hash = 0) {
			add_event_listener(name, _B::MakeLambdaListener(*(_B::OnLambdaListenerFunc*)(&listener), hash, 0));
		}
		
		inline void add_event_listener_once( const Name& name, std::function<void(Event&)> listener, uint32_t hash = 0) {
			add_event_listener(name, _B::MakeLambdaListener(*(_B::OnLambdaListenerFunc*)(&listener), hash, 1));
		}
		
		inline void add_event_listener(const Name& name, Noticer* shell) {
			add_event_listener(name, _B::MakeShellListener(this, shell, 0));
		}

		inline void add_event_listener_once(const Name& name, Noticer* shell) {
			add_event_listener(name, _B::MakeShellListener(this, shell, 1));
		}
		
		template<class Ctx>
		inline void remove_event_listener(const Name& name,
										void (Ctx::*listener)(Event&)
										) {
			auto del = get_noticer(name);
			if (del) {
				del->off(listener);
				trigger_listener_change(name, del->count(), -1);
			}
		}
		
		template<class Ctx>
		inline void remove_event_listener(const Name& name,
										void (Ctx::*listener)(Event&), Ctx* ctx) {
			auto del = get_noticer(name);
			if (del) {
				del->off(listener, ctx);
				trigger_listener_change(name, del->count(), -1);
			}
		}
		
		template<class Ctx>
		inline void remove_event_listener(const Name& name,
											void (*listener)(Event&, Ctx*)
										) {
			auto del = get_noticer(name);
			if (del) {
				del->off(listener);
				trigger_listener_change(name, del->count(), -1);
			}
		}
		
		template<class Ctx>
		inline void remove_event_listener(const Name& name,
										void (*listener)(Event&, Ctx*),
										Ctx* ctx) {
			auto del = get_noticer(name);
			if (del) {
				del->off(listener, ctx);
				trigger_listener_change(name, del->count(), -1);
			}
		}
		
		inline void remove_event_listener(const Name& name, uint32_t hash) {
			auto del = get_noticer(name);
			if (del) {
				del->off(hash);
				trigger_listener_change(name, del->count(), -1);
			}
		}
		
		inline void remove_event_listener(const Name& name, Noticer* shell) {
			auto del = get_noticer(name);
			if (del) {
				del->off(shell);
				trigger_listener_change(name, del->count(), -1);
			}
		}
		
		template<class Ctx>
		inline void remove_event_listener(Ctx* ctx) {
			if (_noticers) {
				for ( auto& i : *_noticers ) {
					i.value.off(ctx);
					trigger_listener_change(i.ket, i.value.count(), -1);
				}
			}
		}

		inline void remove_event_listener(uint32_t hash) {
			if (_noticers) {
				for ( auto& i : *_noticers ) {
					i.value.off(hash);
					trigger_listener_change(i.key, i.value.count(), -1);
				}
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
