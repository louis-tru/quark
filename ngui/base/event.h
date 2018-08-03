/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __ngui__base__event__
#define __ngui__base__event__

#include "list.h"
#include "map.h"
#include "util.h"
#include "string.h"
#include "error.h"
#include <functional>

#define XX_EVENT(name, ...) \
public: inline ngui::EventNoticer<__VA_ARGS__>& name () { return _##name; } \
private:ngui::EventNoticer<__VA_ARGS__>_##name; public:

#define XX_INIT_EVENT(name)  _on##name(#name, this)
#define XX_ON(name, ...)      on##name().on( __VA_ARGS__ )
#define XX_ONCE(name, ...)    on##name().once( __VA_ARGS__ )
#define XX_OFF(name, ...)     on##name().off( __VA_ARGS__ )
#define XX_TRIGGER(name, ...)  on##name().trigger( __VA_ARGS__ )

/**
 * @ns ngui
 */

XX_NS(ngui)

template<class SendData = Object, class Sender = Object, class ReturnValue = int> class Event;
template<class Event = Event<>> class EventNoticer;

template<class T_SendData, class T_Sender, class T_ReturnValue>
class XX_EXPORT Event: public Object {
	XX_HIDDEN_ALL_COPY(Event);
 public:
	typedef T_SendData       SendData;
	typedef T_Sender         Sender;
	typedef T_ReturnValue    ReturnValue;
	typedef const SendData  cSendData;
	typedef EventNoticer<Event> Noticer;
	ReturnValue return_value;
 private:
	Noticer*    m_noticer;
	cSendData*  m_data;
 public:
	Event(cSendData& data = SendData())
	:return_value(), m_noticer(nullptr), m_data(&data) {}
	virtual void release() {
		m_noticer = nullptr;
		m_data = nullptr;
		Object::release();
	}
	inline Noticer* noticer() { return this->m_noticer; }
	inline String name() const { return this->m_noticer->m_name; }
	inline Sender* sender() { return this->m_noticer->m_sender; }
	inline cSendData* data() const { return m_data; }
	
	class BasicEventNoticer: public Object {
	protected:
		inline static void set_event(Noticer* noticer, Event& evt) {
			evt.m_noticer = noticer;
		}
	};
	friend class BasicEventNoticer;
};

template<class Event> class XX_EXPORT EventNoticer: public Event::BasicEventNoticer {
 public:
	typedef Event EventType;
	typedef typename Event::SendData        SendData;
	typedef typename Event::cSendData       cSendData;
	typedef typename Event::Sender          Sender;
	typedef typename Event::ReturnValue     ReturnValue;
	typedef std::function<void(Event&)>     ListenerFunc;
 protected:
	class Listener {
	 public:
		inline Listener(EventNoticer* noticer) { m_noticer = noticer; }
		virtual ~Listener() { }
		virtual void call(Event& evt) = 0;
		virtual bool is_on_listener() { return false; }
		virtual bool is_on_static_listener() { return false; }
		virtual bool is_on_shell_listener() { return false; }
		virtual bool is_on_func_listener() { return false; }
	 protected:
		EventNoticer* m_noticer;
	};
	
	// On
	template<class Scope> class OnListener: public Listener {
	 public:
		typedef void (Scope::*ListenerFunc)(Event& evt);
		inline OnListener(EventNoticer* noticer, ListenerFunc listener, Scope* scope)
		: Listener(noticer), m_scope(scope), m_listener( listener ){ }
		virtual bool is_on_listener() { return true; }
		virtual void call(Event& evt) { (m_scope->*m_listener)(evt); }
		inline bool equals(ListenerFunc listener) { return m_listener == listener; }
		inline bool equals(Scope* scope) { return m_scope == scope; }
	 protected:
		Scope* m_scope;
		ListenerFunc m_listener;
	};

	// ONCE
	template<class Scope> class OnceListener: public OnListener<Scope> {
	 public:
		typedef typename OnListener<Scope>::ListenerFunc ListenerFunc;
		inline OnceListener(EventNoticer* noticer, ListenerFunc listener, Scope* scope)
		: OnListener<Scope>(noticer, listener, scope) { }
		virtual void call(Event& evt) {
			(this->m_scope->*this->m_listener)(evt);
			this->m_noticer->off2(this);
		}
	};
	
	// STATIC
	template<class Data> class OnStaticListener: public Listener {
	 public:
		typedef void (*ListenerFunc)(Event& evt, Data* data);
		inline OnStaticListener(EventNoticer* noticer, ListenerFunc listener, Data* data)
		: Listener(noticer), m_listener(listener), m_data(data) { }
		virtual bool is_on_static_listener() { return true; }
		virtual void call(Event& evt) { m_listener(evt, m_data); }
		inline bool equals(ListenerFunc listener) { return m_listener == listener; }
		inline bool equals(Data* data) { return m_data == data; }
	 protected:
		ListenerFunc m_listener;
		Data* m_data;
	};
	
	// Once STATIC
	template<class Data> class OnceStaticListener: public OnStaticListener<Data> {
	 public:
		typedef typename OnStaticListener<Data>::ListenerFunc ListenerFunc;
		inline OnceStaticListener(EventNoticer* noticer, ListenerFunc listener, Data* data)
		: OnStaticListener<Data>(noticer, listener, data) { }
		virtual void call(Event& evt) {
			this->m_listener(evt);
			this->m_noticer->off2(this);
		}
	};
	
	// Function
	class OnLambdaFunctionListener: public Listener {
	 public:
		inline OnLambdaFunctionListener(EventNoticer* noticer, ListenerFunc&& listener, int id)
		: Listener(noticer), m_listener(ngui::move(listener)), m_id(id) { }
		virtual bool is_on_func_listener() { return true; }
		virtual void call(Event& evt) { m_listener(evt); }
		inline bool equals(int id) { return id == m_id; }
	 protected:
		ListenerFunc m_listener;
		int m_id;
	};
	
	// Once Function
	class OnceLambdaFunctionListener: public OnLambdaFunctionListener {
	 public:
		inline OnceLambdaFunctionListener(EventNoticer* noticer, ListenerFunc&& listener, int id)
		: OnLambdaFunctionListener(noticer, ngui::move(listener), id) { }
		virtual void call(Event& evt) {
			this->m_listener(evt);
			this->m_noticer->off2(this);
		}
	};
	
	// SHELL
	class OnShellListener: public Listener {
	 public:
		inline OnShellListener(EventNoticer* noticer, EventNoticer* shell)
		: Listener(noticer), m_shell(shell) { }
		virtual bool is_on_shell_listener() { return true; }
		virtual void call(Event& evt) {
			this->m_shell->trigger(evt);
			this->m_noticer->set_event2(evt);
		}
		inline bool equals(EventNoticer* shell) { return m_shell == shell; }
	 protected:
		EventNoticer* m_shell;
	};
	
	// Once Shell
	class OnceShellListener: public OnShellListener {
	 public:
		inline OnceShellListener(EventNoticer* noticer, EventNoticer* shell)
		: OnShellListener(noticer, shell) { }
		virtual void action(Event& evt) {
			this->m_shell->trigger(evt);
			this->m_noticer->set_event2(evt);
			this->m_noticer->off2(this);
		}
	};
	
 private:
	
	XX_HIDDEN_ALL_COPY(EventNoticer);
	typedef typename List<Listener*>::Iterator iterator;
	struct LWrap {
		Listener* listener;
		Listener* operator->() { return listener; }
		Listener* value() { return listener; }
		void del() { delete listener; listener = nullptr; }
	};
	String        m_name;
	Sender*       m_sender;
	List<LWrap>*  m_listener;
	friend class  ngui::Event<SendData, Sender, ReturnValue>;
	friend class  OnShellListener;
	friend class  OnceShellListener;
	
 public:
	
	inline EventNoticer(cString& name, Sender* sender = nullptr)
	: m_name(name)
	, m_sender(sender)
	, m_listener(nullptr) { }
	
	virtual ~EventNoticer() {
		if (m_listener) {
			off();
			Release(m_listener);
		}
	}
	
	/**
	 * @fun name
	 */
	inline String name() const { return m_name; }
	
	/**
	 * @func sender
	 */
	inline Sender* sender() const { return m_sender; }
	
	/**
	 * @fun count # 获取侦听器数量
	 */
	inline int count() const {
		return m_listener ? m_listener->length() : 0;
	}

	template<class Scope>
	void on(void (Scope::*listener)(Event&), Scope* scope) throw(Error) {
		get_listener();
		assert2(listener, scope);
		m_listener->push( { new OnListener<Scope>(this, listener, scope) } );
	}

	template <class Data>
	void on( void (*listener)(Event&, Data*), Data* data = nullptr) throw(Error) {
		get_listener();
		assert_static(listener, data);
		m_listener->push( { new OnStaticListener<Data>(this, listener, data) } );
	}

	void on( ListenerFunc listener, int id = 0) {
		get_listener();
		m_listener->push( { new OnLambdaFunctionListener(this, ngui::move(listener), id) } );
	}

	void on(EventNoticer* shell) throw(Error) {
		get_listener();
		assert_shell(shell);
		m_listener->push( { new OnShellListener(this, shell) } );
	}

	template<class Scope>
	void once(void (Scope::*listener)(Event&), Scope* scope) throw(Error) {
		get_listener();
		assert2(listener, scope);
		m_listener->push( { new OnceListener<Scope>(this, listener, scope) } );
	}
	
	template <class Data>
	void once( void (*listener)(Event&, Data*), Data* data = nullptr) throw(Error) {
		get_listener();
		assert_static(listener, data);
		m_listener->push( { new OnceStaticListener<Data>(this, listener, data) } );
	}
	
	void once( ListenerFunc listener, int id = 0) {
		get_listener();
		m_listener->push( { new OnceLambdaFunctionListener(this, ngui::move(listener), id) } );
	}
	
	void once(EventNoticer* shell) throw(Error) {
		get_listener();
		assert_shell(shell);
		m_listener->push( { new OnceShellListener(this, shell) } );
	}

	template<class Scope>
	void off( void (Scope::*listener)(Event&) ) {
		if (m_listener) {
			typedef OnListener<Scope> OnListener2;
			for ( auto& i : *m_listener ) {
				if ( i.value().value() && i.value()->is_on_listener() &&
						static_cast<OnListener2*>(i.value().value())->equals( listener ) ) {
					i.value().del();
				}
			}
		}
	}

	template<class Scope>
	void off( void (Scope::*listener)(Event&), Scope* scope) {
		if (m_listener) {
			typedef OnListener<Scope> OnListener2;
			for ( auto& i : *m_listener ) {
				if( i.value().value() && i.value()->is_on_listener() &&
						static_cast<OnListener2*>(i.value().value())->equals(listener) &&
						static_cast<OnListener2*>(i.value().value())->equals( scope ) ) {
					i.value().del();
					break;
				}
			}
		}
	}
	
	template<class Data>
	void off( void (*listener)(Event&, Data*) ) {
		if (m_listener) {
			typedef OnStaticListener<Data> OnListener2;
			for ( auto& i : *m_listener ) {
				if ( i.value().value() && i.value()->is_on_static_listener() &&
						static_cast<OnListener2*>(i.value().value())->equals(listener) ) {
					i.value().del();
					break;
				}
			}
		}
	}
	
	template<class Data>
	void off( void (*listener)(Event&, Data*), Data* data) {
		if (m_listener) {
			typedef OnStaticListener<Data> OnListener2;
			for ( auto& i : *m_listener ) {
				if ( i.value().value() && i.value()->is_on_static_listener() &&
						static_cast<OnListener2*>(i.value().value())->equals(listener) &&
						static_cast<OnListener2*>(i.value().value())->equals(data) ) {
					i.value().del();
					break;
				}
			}
		}
	}
	
	void off(int id) {
		if (m_listener) {
			for ( auto& i : *m_listener ) {
				if ( i.value().value() && i.value()->is_on_func_listener() &&
							static_cast<OnLambdaFunctionListener*>(i.value().value())->equals(id)
					 )
				{//
					i.value().del();
				}
			}
		}
	}
	
	template<class Scope>
	void off(Scope* scope) {
		if (m_listener) {
			typedef OnListener<Scope> OnListener2;
			typedef OnStaticListener<Scope> OnListener3;
			for ( auto& i : *m_listener ) {
				if ( i.value().value &&
						(
							(
								i.value()->is_on_listener() &&
								static_cast<OnListener2*>(i.value().value())->equals(scope)
							) ||
							(
								i.value()->is_on_static_listener() &&
								static_cast<OnListener3*>(i.value().value())->equals(scope)
							)
						)
					)
				{//
					i.value().del();
				}
			}
		}
	}
	
	void off(EventNoticer* shell) {
		if (m_listener) {
			for ( auto& i : *m_listener ) {
				if ( i.value().value() && i.value()->is_on_shell_listener() &&
						 static_cast<OnShellListener*>(i.value())->equals( shell ) )
				{ //
					i.value().del();
					break;
				}
			}
		}
	}

	void off() {
		if (m_listener) {
			for ( auto& i : *m_listener ) {
				i.value().del();
			}
		}
	}
	
	ReturnValue trigger() {
		if (m_listener) {
			Event evt;
			return move( trigger(evt) );
		}
		return move( ReturnValue() );
	}
	
	ReturnValue trigger(cSendData& data) {
		if (m_listener) {
			Event evt(data);
			return move( trigger(evt) );
		}
		return move( ReturnValue() );
	}
	
	ReturnValue& trigger(Event& evt) {
		if (m_listener) {
			set_event2(evt);
			for (auto i = m_listener->begin(); i != m_listener->end(); ) {
				auto j = i++;
				Listener* listener = j.value().listener;
				if ( listener ) {
					// TODO:
					// listener->mListener 如果为空指针或者野指针,会导致程序崩溃。。
					// 应该在对像释放前移除事件侦听器，这对于大型gui程序无GC的c++是非常糟糕的,
					// 在VC中有delegate,在QT中有SIGNAL/SLOT,
					// 幸好我们的程序是运行在js环境中的,无需对原生的api有过多的依赖。
					listener->call(evt);
				} else {
					m_listener->del(j);
				}
			}
		}
		return evt.return_value;
	}
	
 private:

	inline void set_event2(Event& evt) {
		this->set_event(reinterpret_cast<typename Event::Noticer*>(this), evt);
	}
	
	inline void get_listener() {
		XX_ASSERT(!m_name.is_empty());
		if (m_listener == nullptr) {
			m_listener = new List<LWrap>();
		}
	}
	
	void off2(Listener* listener) {
		for ( auto& i : *m_listener ) {
			if ( i.value().value() == listener ) {
				i.value().del();
				break;
			}
		}
	}
	
	template<class Scope>
	void assert2(void (Scope::*listener)(Event&), Scope* scope) throw(Error) {
		typedef OnListener<Scope> OnListener2;
		for ( auto& i : *m_listener ) {
			if ( i.value().value() && i.value()->is_on_listener() ) {
				XX_ASSERT_ERR( !(static_cast<OnListener2*>(i.value().value())->equals( listener ) &&
												 static_cast<OnListener2*>(i.value().value())->equals( scope )),
											ERR_DUPLICATE_LISTENER,
										 "Events have been added over the letter");
			}
		}
	}
	
	template<class Data>
	void assert_static(void (*listener)(Event&, Data*), Data* data) throw(Error) {
		typedef OnStaticListener<Data> OnStaticListener2;
		for ( auto& i : *m_listener ) {
			if ( i.value().value() && i.value()->is_on_static_listener() ) {
				XX_ASSERT_ERR( !(static_cast<OnStaticListener2*>(i.value().value())->equals( listener ) &&
												 static_cast<OnStaticListener2*>(i.value().value())->equals( data )),
											ERR_DUPLICATE_LISTENER,
											"Events have been added over the letter");
			}
		}
	}
	
	void assert_shell(EventNoticer* shell) throw(Error) {
		for ( auto& i : *m_listener ) {
			if ( i.value().value() && i.value()->is_on_shell_listener() ) {
				XX_ASSERT_ERR( !static_cast<OnShellListener*>(i.value().value())->equals( shell ),
											ERR_DUPLICATE_LISTENER,
											"Events have been added over the letter");
			}
		}
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
class XX_EXPORT Notification: public Basic {
	XX_HIDDEN_ALL_COPY(Notification);
 public:
	typedef Event EventType;
	typedef Name  NameType;
	typedef EventNoticer<Event>         Noticer;
	typedef typename Event::SendData    SendData;
	typedef typename Event::cSendData   cSendData;
	typedef typename Event::Sender      Sender;
	typedef typename Event::ReturnValue ReturnValue;
	typedef typename Noticer::ListenerFunc ListenerFunc;
 private:
	
	struct NoticerWrap {
		inline NoticerWrap() { XX_UNREACHABLE(); }
		inline NoticerWrap(const Name& t, Sender* sender)
		: name(t), value(t.to_string(), sender) { }
		Name    name;
		Noticer value;
	};
	typedef Map<Name, NoticerWrap*> Events;
	Events* m_noticers;
	
 public:
	
	inline Notification()
	: m_noticers(nullptr) {
		
	}
	
	virtual ~Notification() {
		if ( m_noticers ) {
			for (auto& i : *m_noticers) {
				delete i.value();
			}
			Release(m_noticers);
			m_noticers = nullptr;
		}
	}
	
	Noticer* noticer(const Name& name) const {
		if ( m_noticers != nullptr ) {
			auto it = m_noticers->find(name);
			if (!it.is_null()) {
				return &it.value()->value;
			}
		}
		return nullptr;
	}
	
	/**
	 * 是否没有任何事件
	 * @ret {bool}
	 */
	inline bool is_noticer_none() const {
		return m_noticers == nullptr || m_noticers->length() == 0;
	}
	
	/**
	 * 侦听器变化会通知到该函数,比如添加删除事件侦听器
	 * @arg name {const Type&}
	 * @arg count {int}
	 */
	virtual void trigger_listener_change(const Name& name, int count, int change) { }
	
	template<class Scope>
	inline void on(const Name& name,
								 void (Scope::*listener)(Event&),
								 Scope* scope) {
		auto del = get_noticer(name);
		del->on(listener, scope);
		trigger_listener_change(name, del->count(), 1);
	}
	
	template<class Scope>
	inline void once(const Name& name,
									 void (Scope::*listener)(Event&),
									 Scope* scope) {
		auto del = get_noticer(name);
		del->once(listener, scope);
		trigger_listener_change(name, del->count(), 1);
	}
	
	template<class Data>
	inline void on(const Name& name,
								 void (*listener)(Event&, Data*),
								 Data* data = nullptr) {
		auto del = get_noticer(name);
		del->once(listener, data);
		trigger_listener_change(name, del->count(), 1);
	}
	
	template<class Data>
	inline void once(const Name& name,
									 void (*listener)(Event&, Data*),
									 Data* data = nullptr) {
		auto del = get_noticer(name);
		del->once(listener, data);
		trigger_listener_change(name, del->count(), 1);
	}
	
	inline void on( const Name& name, ListenerFunc listener, int id = 0) {
		auto del = get_noticer(name);
		del->on(listener, id);
		trigger_listener_change(name, del->count(), 1);
	}
	
	inline void once( const Name& name, ListenerFunc listener, int id = 0) {
		auto del = get_noticer(name);
		del->once(listener, id);
		trigger_listener_change(name, del->count(), 1);
	}
	
	inline void on(const Name& name, Noticer* shell) {
		auto del = get_noticer(name);
		del->on(shell);
		trigger_listener_change(name, del->count(), 1);
	}
	
	/**
	 * 添加一个侦听器,只侦听一次,后被卸载
	 */
	inline void once(const Name& name, Noticer* shell) {
		auto del = get_noticer(name);
		del->once(shell);
		trigger_listener_change(name, del->count(), 1);
	}
	
	template<class Scope>
	inline void off(const Name& name,
									void (Scope::*listener)(Event&)
									) {
		auto del = noticer(name);
		if (del) {
			del->off(listener);
			trigger_listener_change(name, del->count(), -1);
		}
	}
	
	template<class Scope>
	inline void off(const Name& name,
									void (Scope::*listener)(Event&), Scope* scope) {
		auto del = noticer(name);
		if (del) {
			del->off(listener, scope);
			trigger_listener_change(name, del->count(), -1);
		}
	}
	
	template<class Data>
	inline void off(const Name& name,
									void (*listener)(Event&, Data*)
									) {
		auto del = noticer(name);
		if (del) {
			del->off(listener);
			trigger_listener_change(name, del->count(), -1);
		}
	}
	
	template<class Data>
	inline void off(const Name& name,
									void (*listener)(Event&, Data*),
									Data* data) {
		auto del = noticer(name);
		if (del) {
			del->off(listener, data);
			trigger_listener_change(name, del->count(), -1);
		}
	}
	
	/**
	 * @func off
	 */
	inline void off(const Name& name, int id) {
		auto del = noticer(name);
		if (del) {
			del->off(id);
			trigger_listener_change(name, del->count(), -1);
		}
	}
	
	/**
	 * @func off
	 */
	inline void off(int id) {
		if (m_noticers) {
			auto end = m_noticers->end();
			for (auto i = m_noticers->begin(); i != end; i++) {
				NoticerWrap* item = &i.value();
				item->value.off(id);
				trigger_listener_change(item->name, item->value.count(), -1);
			}
		}
	}
	
	/**
	 * 卸载这个范围里的所有侦听器
	 */
	template<class Scope> inline void off(Scope* scope) {
		if (m_noticers) {
			for ( auto& i : *m_noticers ) {
				NoticerWrap* inl = i.value();
				inl->value.off(scope);
				trigger_listener_change(inl->name, inl->value.count(), -1);
			}
		}
	}
	
	inline void off(const Name& name, Noticer* shell) {
		auto del = noticer(name);
		if (del) {
			del->off(shell);
			trigger_listener_change(name, del->count(), -1);
		}
	}
	
 protected:
	
	/**
	 * 卸载指定事件名称上的全部侦听函数
	 */
	inline void off(const Name& name) {
		auto del = noticer(name);
		if (del) {
			del->off();
			trigger_listener_change(name, del->count(), -1);
		}
	}
	
	/**
	 * 卸载全部侦听函数
	 */
	inline void off() {
		if (m_noticers) {
			for ( auto& i : *m_noticers ) {
				NoticerWrap* inl = i.value();
				inl->value.off();
				trigger_listener_change(inl->name, inl->value.count(), -1);
			}
		}
	}
	
	/**
	 * 触发事件
	 * @arg name {const Key&}
	 * NOTE: 这个方法能创建默认事件数据
	 */
	inline ReturnValue trigger(const Name& name) {
		auto del = noticer(name);
		return move( del ? del->trigger(): ReturnValue() );
	}
	
	/*
	 * 触发事件
	 * @arg name {const Key&}
	 * @arg evt {cSendData&}
	 */
	inline ReturnValue trigger(const Name& name, cSendData& data) {
		auto del = noticer(name);
		return move( del ? del->trigger(data): ReturnValue() );
	}
	
	/*
	 * 触发事件
	 * @arg name {const Key&}
	 * @arg evt {Event&}
	 */
	inline ReturnValue& trigger(const Name& name, Event& evt) {
		auto del = noticer(name);
		return del ? del->trigger(evt): evt.return_value;
	}
	
 private:
	
	Noticer* get_noticer(const Name& name) {
		if (m_noticers == nullptr) {
			m_noticers = new Events();
		}
		auto it = m_noticers->find(name);
		if (it != m_noticers->end()) {
			return &it.value()->value;
		} else {
			return &m_noticers->set(name, new NoticerWrap(name, static_cast<Sender*>(this)))->value;
		}
	}
	
};

XX_END
#endif
