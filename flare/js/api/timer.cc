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

#include "../js.h"
#include "../../util/loop.h"

/**
 * @ns flare::js
 */

JS_BEGIN

class Timer: public Reference {
	public:
	typedef void (*CallbackPtr)(Timer* timer);
	uint32_t      _timer_id;  // id
	uint64_t    _timeout;   // 超时时间
	int       _loop;      // -1 为无限循环
	RunLoop*  _run_loop;  // 消息队列
	Callback<> _cb;
	CallbackPtr _cb_ptr;
	Callback<> _cb2;
	
	void _run_cb(CbData& d) {
		
		if ( _cb_ptr ) {
			_cb_ptr(this);
		} else {
			CbData evt = { 0, this }; _cb->call(evt);
		}
		
		_timer_id = 0;
		
		if (_loop > 0) {
			_loop--;
		}
		if (_loop) {
			_run();
		} else {
			_cb2 = Cb(); // destroy callback
		}
	}
	
	void _run() {
		if ( !_timer_id ) {
			_timer_id = _run_loop->post(_cb2, _timeout * 1000);
		}
	}
	
	Timer(RunLoop* loop, Cb cb)
	: _timer_id(0)
	, _timeout(0)
	, _loop(1)
	, _run_loop(loop)
	, _cb(cb) {
		
	}
	
	Timer(RunLoop* loop, CallbackPtr cb)
	: _timer_id(0)
	, _timeout(0)
	, _loop(1)
	, _run_loop(loop)
	, _cb_ptr(cb) {
		
	}
	
	virtual ~Timer() {
		stop();
	}
	
	int loop() const {
		return _loop;
	}
	
	void loop(int loop) {
		_loop = FX_MAX(-1, loop);
	}
	
	void run(uint64_t timeout, int loop = 1) {
		_timeout = timeout;
		_loop = loop;
		if ( !_timer_id ) {
			_cb2 = Cb(&Timer::_run_cb, this);
			_run();
		}
	}
	
	void stop() {
		if ( _loop && _timer_id ) {
			_run_loop->cancel( _timer_id );
			_loop = 0;
			_timer_id = 0;
			_cb2 = Cb(); // destroy callback
		}
	}

};

class WrapTimer: public WrapObject {
	public:
	
	static void constructor(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsFunction(worker)) {
			JS_THROW_ERR("Bad argument");
		}
		RunLoop* loop = RunLoop::current();
		if (!loop) { // 没有消息队列无法执行这个操作
			JS_THROW_ERR("Unable to obtain thread run queue");
		}
		
		Wrap<Timer>* wrap = nullptr;
		// 小心循环引用
		wrap = WrapObject::New<WrapTimer>(args, new Timer(loop, &timer_callback_func));
		wrap->set(worker->New("__native_handle_cb__",1), args[0]);
	}
	
	static void timer_callback_func(Timer* timer) {
		Wrap<Timer>* wrap = Wrap<Timer>::pack(timer);
		Worker* worker = wrap->worker();
		
		/*
		 * native从顶级堆栈调用javascript必需声明句柄范围, 
		 * 否则v8声明的临时变量会存储在顶级句柄范围中,
		 * 因为顶级范围句柄从来不会释放,这会造成局部变量也不会释放最终导致内存泄漏
		 */
		JS_HANDLE_SCOPE();
		JS_CALLBACK_SCOPE();
		wrap->call(wrap->worker()->New("__native_handle_cb__",1));
	}
	
	static void Loop(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Timer);
		JS_RETURN( self->loop() );
	}
	
	static void SetLoop(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args);
		if (!value->IsInt32(worker)) {
			JS_THROW_ERR("Bad argument");
		}
		JS_SELF(Timer);
		self->loop(value->ToInt32Value(worker));
	}
	
	static void Run(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() == 0 || ! args[0]->IsNumber(worker)) {
			JS_THROW_ERR("Bad argument");
		}
		uint64_t timeout = FX_MAX(0, args[0]->ToNumberValue(worker));
		int loop = 1;
		if (args.Length() > 1 && args[1]->IsInt32(worker)) {
			loop = args[1]->ToInt32Value(worker);
		}
		JS_SELF(Timer);
		self->run(timeout, loop);
	}
	
	static void Stop(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Timer);
		self->stop();
	}

	// global function

	static void run_timer_(FunctionCall args, int loop) {
		JS_WORKER(args);
		if (args.Length() == 0 || !args[0]->IsFunction(worker)) {
			JS_THROW_ERR("Bad argument");
		}
		uint64_t timeout = 0;
		if (args.Length() > 1 && args[1]->IsNumber(worker)) {
			timeout = FX_MAX(0, args[1]->ToNumberValue(worker));
		}
		
		Local<JSValue> cb = args[0];
		Local<JSObject> o = worker->NewInstance(JS_TYPEID(Timer), 1, &cb);
		
		if ( !o.IsEmpty() ) {
			Wrap<Timer>* wrap = Wrap<Timer>::unpack(o);
			wrap->self()->run(timeout, loop);
			JS_RETURN( wrap->that() );
		}
	}

	static void setTimeout(FunctionCall args) {
		run_timer_(args, 1);
	}

	static void setInterval(FunctionCall args) {
		run_timer_(args, -1);
	}

	static void clearTimeout(FunctionCall args) {
		JS_WORKER(args);
		if ( args.Length() == 0 || ! worker->hasInstance<Timer>(args[0]) ) {
			JS_THROW_ERR("Bad argument");
		}
		Wrap<Timer>::unpack(args[0].To<JSObject>())->self()->stop();
	}

	static void clearInterval(FunctionCall args) {
		clearTimeout(args);
	}

	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(Timer, constructor, {
			JS_SET_CLASS_ACCESSOR(loop, Loop, SetLoop);
			JS_SET_CLASS_METHOD(run, Run);
			JS_SET_CLASS_METHOD(stop, Stop);
		}, nullptr);
		JS_SET_METHOD(setTimeout, setTimeout);
		JS_SET_METHOD(setInterval, setInterval);
		JS_SET_METHOD(clearTimeout, clearTimeout);
		JS_SET_METHOD(clearInterval, clearInterval);
	}

};

JS_REG_MODULE(_timer, WrapTimer)
JS_END
