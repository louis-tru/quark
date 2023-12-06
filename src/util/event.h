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
	template<class Sender = Object, class SendData = Object, typename RC = int> class Event;
	template<class Event = Event<>, class Lock = _Lock> class EventNoticer;

	template<class T_Sender, class T_SendData, typename T_RC>
	class Event: public Object {
	public:
		typedef T_SendData       SendData;
		typedef T_Sender         Sender;
		typedef T_RC             ReturnValue;
		typedef const SendData   cSendData;

		Event(cSendData& data = SendData(), const ReturnValue& rc = ReturnValue())
			: _sender(nullptr), _data(&data), return_value(rc)  {}
		inline Sender* sender() const { return _sender; }
		inline cSendData* data() const { return _data; }
	protected:
		Sender    *_sender;
		cSendData *_data;
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
			Listener(uint32_t id, bool once);
			virtual ~Listener();
			inline uint32_t id() const { return _id; }
			inline bool once() const { return _once; }
			virtual void call(Object& evt) = 0;
			virtual bool match(ListenerFunc l, void* ctx);
			virtual bool match(StaticListenerFunc l, void* ctx);
		protected:
			uint32_t  _id;
			bool      _once;
		};
		// make listener
		static Listener* MakeListener(ListenerFunc l, void* ctx, bool once);
		static Listener* MakeStaticListener(StaticListenerFunc l, Object* ctx, bool once);
		static Listener* MakeLambdaListener(OnLambdaListenerFunc& l, uint32_t id, bool once);
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
		void off(uint32_t id);
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

	class Qk_EXPORT NotificationBasic {
	public:
		typedef EventNoticerBasic Basic;
		NotificationBasic();
		virtual ~NotificationBasic();
		Basic* get_noticer(uint32_t name, bool no_null = 0);
		bool has_noticer(uint32_t name) const;
		bool is_noticer_none() const;
		/**
		 * Listener changes will be notified to this function, such as adding and deleting event listeners
		 * @arg name {const Type&} hash code for name
		*/
		virtual void trigger_listener_change(uint32_t name, int count, int change);
		// get notification message sender
		virtual void* event_noticer_sender();
		//!< add event listener
		void add_event_listener(uint32_t name, Basic::Listener *l);
		//!< remove event listener
		void remove_event_listener(uint32_t name, void (Object::*listener)(Object&));
		void remove_event_listener(uint32_t name, void (Object::*listener)(Object&), void *ctx);
		void remove_event_listener_static(uint32_t name, void (*listener)(Object&, void*));
		void remove_event_listener_static(uint32_t name, void (*listener)(Object&, void*), void* ctx);
		void remove_event_listener_for_id(uint32_t name, uint32_t id);
		void remove_event_listener_shell(uint32_t name, Basic *shell);
		void remove_event_listener_for_ctx(void* ctx);
		void remove_event_listener_for_id(uint32_t id);
	protected:
		// Uninstall all listening functions on the specified event name
		void remove_event_listener_for_name(uint32_t name);
		// Uninstall all listening functions
		void remove_event_listener();

		Dict<uint32_t, Basic*>* _noticers;
	};

	template<class Event, class Lock>
	class EventNoticer: public EventNoticerBasic, public Lock {
	public:
		typedef Event EventType;
		typedef typename Event::SendData        SendData;
		typedef typename Event::cSendData       cSendData;
		typedef typename Event::Sender          Sender;
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

		void on( std::function<void(Event&)> listener, uint32_t id = 0) {
			add_listener(MakeLambdaListener(*(OnLambdaListenerFunc*)(&listener), id, 0));
		}

		void once( std::function<void(Event&)> listener, uint32_t id = 0) {
			add_listener(MakeLambdaListener(*(OnLambdaListenerFunc*)(&listener), id, 1));
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

	template<class Event = Event<>, class Name  = String, class Base = Object>
	class Notification: public Base, public NotificationBasic {
		Qk_HIDDEN_ALL_COPY(Notification);
	public:
		typedef Event               EventType;
		typedef Name                NameType;
		typedef EventNoticer<Event> Noticer;

		template<typename... Args>
		Notification(Args... args): Base(args...) {}

		virtual void* event_noticer_sender() override {
			return this;
		}

		Noticer* get_noticer(const Name& name) {
			return static_cast<Noticer*>(NotificationBasic::get_noticer(name.hashCode(), 0));
		}

		bool has_noticer(const Name& name) const {
			return NotificationBasic::has_noticer(name.hashCode());
		}
		
		void add_event_listener(const Name& name, Basic::Listener *l) {
			NotificationBasic::add_event_listener(name.hashCode(), l);
		}

		template<class Ctx>
		void add_event_listener(const Name& name, void (Ctx::*listener)(Event&), Ctx* ctx) {
			add_event_listener(name, Basic::MakeListener((Basic::ListenerFunc)listener, ctx, 0));
		}

		template<class Ctx>
		void add_event_listener_once(const Name& name, void (Ctx::*listener)(Event&), Ctx* ctx) {
			add_event_listener(name, Basic::MakeListener((Basic::ListenerFunc)listener, ctx, 1));
		}
		
		template<class Ctx>
		void add_event_listener(const Name& name, void (*listener)(Event&, Ctx*), Ctx* ctx = nullptr) {
			add_event_listener(name, Basic::MakeStaticListener((Basic::StaticListenerFunc)listener, ctx, 0));
		}
		
		template<class Ctx>
		void add_event_listener_once(const Name& name, void (*listener)(Event&, Ctx*), Ctx* ctx = nullptr) {
			add_event_listener(name, Basic::MakeStaticListener((Basic::StaticListenerFunc)listener, ctx, 1));
		}
		
		void add_event_listener( const Name& name, std::function<void(Event&)> listener, uint32_t id = 0) {
			add_event_listener(name, Basic::MakeLambdaListener(*(Basic::OnLambdaListenerFunc*)(&listener), id, 0));
		}
		
		void add_event_listener_once( const Name& name, std::function<void(Event&)> listener, uint32_t id = 0) {
			add_event_listener(name, Basic::MakeLambdaListener(*(Basic::OnLambdaListenerFunc*)(&listener), id, 1));
		}
		
		void add_event_listener(const Name& name, Noticer* shell) {
			add_event_listener(name, Basic::MakeShellListener(this, shell, 0));
		}

		void add_event_listener_once(const Name& name, Noticer* shell) {
			add_event_listener(name, Basic::MakeShellListener(this, shell, 1));
		}
		
		template<class Ctx>
		void remove_event_listener(const Name& name, void (Ctx::*listener)(Event&)) {
			remove_event_listener(name.hashCode(), (Basic::ListenerFunc)listener);
		}
		
		template<class Ctx>
		inline void remove_event_listener(const Name& name, void (Ctx::*listener)(Event&), Ctx* ctx) {
			remove_event_listener(name.hashCode(), (Basic::ListenerFunc)listener, ctx);
		}

		template<class Ctx>
		inline void remove_event_listener(const Name& name, void (*listener)(Event&, Ctx*)) {
			remove_event_listener_static(name.hashCode(), (Basic::StaticListenerFunc)listener);
		}
		
		template<class Ctx>
		inline void remove_event_listener(const Name& name, void (*listener)(Event&, Ctx*), Ctx* ctx) {
			remove_event_listener_static(name.hashCode(), (Basic::StaticListenerFunc)listener, ctx);
		}
		
		void remove_event_listener(const Name& name, uint32_t id) {
			remove_event_listener_for_id(name.hashCode(), id);
		}
		
		void remove_event_listener(const Name& name, Noticer* shell) {
			remove_event_listener_shell(name.hashCode(), shell);
		}
		
		template<class Ctx>
		void remove_event_listener(Ctx* ctx) {
			remove_event_listener_for_ctx(ctx);
		}

		void remove_event_listener(uint32_t id) {
			remove_event_listener_for_id(id);
		}
		
	protected:

		// Uninstall all listening functions on the specified event name
		void remove_event_listener(const Name& name) {
			remove_event_listener_for_name(name.hashCode());
		}

		void trigger(const Name& name) {
			auto del = get_noticer(name);
			if (del)
				del->trigger();
		}

		void trigger(const Name& name, typename Noticer::cSendData& data) {
			auto del = get_noticer(name);
			if (del)
				del->trigger(data);
		}

		void trigger(const Name& name, Event& evt) {
			auto del = get_noticer(name);
			if (del)
				del->trigger(evt);
		}
	};

}
#endif
