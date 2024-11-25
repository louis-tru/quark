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

#include "./jsc.h"

namespace qk { namespace js {

	// --- M e s s a g e ---

	class JscRef {
	public:
		JscRef(JscWorker* worker): _worker(worker) {
			_handle = JSObjectMake(worker->jscc(), nullptr, nullptr);
			DCHECK(_handle);
			JSValueProtect(worker->jscc(), _handle);
		}
		~JscRef() {
			JSValueUnprotect(worker->jscc(), _handle);
		}
		void ref(JSValueRef val) {
			if (val) {
				JSValueRef ex = 0;
				static int id = 0;
				JSObjectSetPropertyAtIndex(_worker->jscc(), Handle(), id++, val, &ex);
				DCHECK(!ex);
			}
		}
	protected:
		JscWorker*  _worker;
		JSObjectRef _handle;
	};

	class StackFrame: public JscRef {
	public:
		StackFrame(JscWorker* worker, char* value): JscRef(worker) {
			auto ctx = worker->jscc();
			//
			//  [test.js]:7:21
			//  func@[test.js]:7:25
			//  run@[test.js]:8:5
			//  global code@[test.js]:35:4
			//
			// function_name
			char* ch = strchr(value, '[');
			if (ch) {
				if (ch[-1] == '@') {
					ch[-1] = '\0';
					_function_name = JSValueMakeString(ctx, JSStringCreateWithUTF8CString(value));
					value = ch;
				} else {
					_function_name = JSValueMakeString(ctx, anonymous_s);
				}
				ref(_function_name);

				// script_name
				if (value[0] == '[' && (ch = strchr(value, ']'))) {
					*ch = '\0';
					_script_name = JSValueMakeString(ctx, JSStringCreateWithUTF8CString(value + 1));
					ref(_script_name);
					value = ch + 1;
				} else {
					_script_name = isolate->Empty();
				}
				// line / column
				if (value[0] == ':' && (ch = strchr(value + 1, ':'))) {
					*ch = '\0';
					sscanf(value + 1, "%d", &_line_number);
					value = ch + 1;
					sscanf(value, "%d", &_column);
				}
			} else {
				// B
				// run@[test.js]:25:38
				// global code@[test.js]:45:4
				_is_eval = true;
				_function_name = JSValueMakeString(ctx, JSStringCreateWithUTF8CString(value));
				_script_name = JSValueMakeString(ctx, eval_s);
				ref(_function_name);
				ref(_script_name);
			}
		}

		void Print(std::stringstream& stream, const std::string& space, bool line_feed) {
			// ReferenceError: KK is not defined
			//   at emit (events.js:210:7)
			//   at onLine (readline.js:279:10)
			v8::String::Utf8Value function_name(Cast<String>(_function_name));
			v8::String::Utf8Value script_name(Cast<String>(_script_name));
			stream << space << "at " << *function_name << " ("
						<< *script_name << ":" << _line_number << ":" << _column << ")";
			if (line_feed) {
				stream << '\n';
			}
		}

		inline JSValueRef FunctionName() const {
			return _function_name;
		}
		
		inline JSValueRef ScriptName() const {
			return _script_name;
		}

	private:
		int _line_number = 0;
		int _column = 0;
		int _script_id = 0;
		JSValueRef _function_name = nullptr;
		JSValueRef _script_name = nullptr;
		bool _is_eval = false;
		bool _is_constructor = false;
		bool _is_wasm = false;
		friend class StackTrace;
		friend class Message;
	};

	class StackTrace: public JscRef {
	public:
		StackTrace(JscWorker* worker, JSValueRef stack): JscRef(worker), _as_array(nullptr) {
			v8::String::Utf8Value utf8_value(Cast<String>(stack));
			DCHECK(*utf8_value);
			char* value = *utf8_value;
			char* ch = nullptr;
			while (*value != '\0') {
				char* ch = strchr(value, '\n');
				if (ch) {
					*ch = '\0';
				} else {
					ch = *utf8_value + utf8_value.length() - 1;
				}
				auto frame = new StackFrame(worker, value);
				_frames.push_back(frame);
				value = ch + 1;
			}
		}

		~StackTrace() {
			for (auto i: _frames) {
				delete i;
			}
		}

		inline int GetFrameCount() const {
			return (int)_frames.size();
		}

		StackFrame* GetFrame(uint32_t index) const {
			if (index < _frames.size()) {
				return _frames[index];
			}
			return nullptr;
		}

		void Print(std::stringstream& stream, const std::string& space) {
			if (_frames.size()) {
				for (int i = 0; i < _frames.size() - 1; i++) {
					_frames[i]->Print(stream, space, true);
				}
				_frames[m_frames.size() - 1]->Print(stream, space, false);
			}
		}

		JSObjectRef AsArray() {
			if (!_as_array) {
				ENV(_worker);
				JSValueRef argv[1] = { JSValueMakeNumber(ctx, _frames.size()) };
				JSObjectRef arr = JSObjectMakeArray(ctx, 1, argv, OK(0));
				
				for ( int i = 0 ; i < m_frames.size(); i++) {
					auto item = m_frames[i];
					auto jitem = JSObjectMake(ctx, 0, 0);
					DCHECK(item);
					JSObjectSetProperty(ctx, jitem, line_s,
															JSValueMakeNumber(ctx, item->_line_number), 0, OK(0));
					JSObjectSetProperty(ctx, jitem, column_s,
															JSValueMakeNumber(ctx, item->_column), 0, OK(0));
					JSObjectSetProperty(ctx, jitem, scriptId_s,
															JSValueMakeNumber(ctx, item->_script_id), 0, OK(0));
					JSObjectSetProperty(ctx, jitem, functionName_s, item->_function_name, 0, OK(0));
					JSObjectSetProperty(ctx, jitem, scriptName_s, item->_script_name, 0, OK(0));
					JSObjectSetProperty(ctx, jitem, isEval_s,
															JSValueMakeBoolean(ctx, item->_is_eval), 0, OK(0));
					JSObjectSetProperty(ctx, jitem, isConstructor_s,
															JSValueMakeBoolean(ctx, item->_is_constructor), 0, OK(0));
					JSObjectSetProperty(ctx, jitem, isWasm_s,
															JSValueMakeBoolean(ctx, item->_is_wasm), 0, OK(0));
					JSObjectSetPropertyAtIndex(ctx, arr, i, jitem, OK(0));
				}
				_as_array = arr;
				ref(arr);
			}
			return _as_array;
		}

		static StackTrace* CurrentStackTrace(JscWorker* worker) {
			auto exception = JSObjectCallAsConstructor(worker->jscc(), worker->_globalData.Error, 0, nullptr, OK(0));
			DCHECK(exception);
			return new StackTrace(worker, exception);
		}

	private:
		JSObjectRef _as_array;
		std::vector<StackFrame*> _frames;
		friend class v8::StackTrace;
	};

	class Message: public JscRef {
	public:
		Message(JscWorker* worker,
						JSObjectRef exception,
						JSValueRef message, StackTrace* stack)
		: JscRef(worker)
		, _exception(exception)
		, _message(message)
		, _source_line(worker->_globalData.Empty)
		, _start_position(0)
		, _end_position(0)
		, _line_number(0)
		, _start_column(0)
		, _end_column(0)
		, _stack_trace(stack)
		{
			ref(_exception);
			ref(_message);
			ref(_source_line);
		}

		~Message() {
			delete _stack_trace;
		}

		void Print(std::stringstream& stream, const std::string& space = "    ") {
			v8::String::Utf8Value utf8(Cast(m_message));
			stream << *utf8;
			if (_stack_trace->GetFrameCount()) {
				stream << "\n";
				_stack_trace->Print(stream, space);
			}
		}

		void PrintLog() {
			std::stringstream stream;
			Print(stream);
			std::string r = stream.str();
			LOG(r);
		}

		ScriptOrigin GetScriptOrigin() const {
			// auto isolate = reinterpret_cast<v8::Isolate*>(_worker);
			// return ScriptOrigin(GetScriptResourceName(),
			// 										Integer::New(isolate, m_line_number),
			// 										Integer::New(isolate, m_start_column));
		}

		Local<Value> GetScriptResourceName() const {
			return Cast(_stack_trace->GetFrameCount() ?
									_stack_trace->GetFrame(0)->ScriptName(): GetIsolate()->Empty());
		}

		static Message* Create(JscWorker* worker, JSObjectRef exception) {
			DCHECK(exception);
			ENV(worker);
			if (!JSValueIsObject(ctx, exception)) {
				exception = JSObjectCallAsConstructor(ctx, worker->_globalData.Error, 1, &exception, 0);
				DCHECK(exception);
			}
			auto line = JSObjectGetProperty(ctx, exception, line_s, OK(nullptr));
			auto column = JSObjectGetProperty(ctx, exception, column_s, OK(nullptr));
			auto line2 = JSValueToNumber(ctx, line, OK(nullptr));
			auto column2 = JSValueToNumber(ctx, column, OK(nullptr));
			JSStringRef message_str = JSValueToStringCopy(ctx, exception, OK(nullptr));
			JSValueRef message = JSValueMakeString(ctx, message_str);
			JSValueRef stack = nullptr;
			bool has_old_stack = JSObjectHasProperty(ctx, exception, _stack_s);
			if (has_old_stack) {
				stack = JSObjectGetProperty(ctx, exception, _stack_s, 0);
			} else {
				stack = JSObjectGetProperty(ctx, exception, stack_s, 0);
			}
			DCHECK(stack);
			auto stack_trace = new StackTrace(isolate, stack);
			auto m = new Message(iso, exception, message, stack_trace);
			m->_line_number = line2;
			m->_start_column = column2;
			m->_end_column = m->_start_column;

			if (!has_old_stack) {
				JSPropertyAttributes attrs = kJSPropertyAttributeReadOnly |
					kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete;
				JSObjectSetProperty(ctx, exception, _stack_s, stack, attrs, OK(nullptr));
				// set
				std::stringstream stream;
				m->Print(stream);
				std::string r = stream.str();
				auto v8_isolate = reinterpret_cast<v8::Isolate*>(isolate);
				auto key = v8::String::NewFromUtf8(v8_isolate, "stack");
				auto value = v8::String::NewFromUtf8(v8_isolate, r.c_str());
				PropertyDescriptor desc(value);
				Cast<v8::Object>(exception)->
					DefineProperty(isolate->GetCurrentContext(), key, desc).FromJust();
			}
			return m;
		}

	private:
		JSObjectRef _exception;
		JSValueRef _message;
		JSValueRef _source_line;
		int _start_position;
		int _end_position;
		int _line_number;
		int _start_column;
		int _end_column;
		StackTrace* _stack_trace;
		friend class v8::Message;
	};

	// --- E x c e p t i o n s ---

	class JscTryCatch {
	public:
		JscTryCatch(JscWorker* worker)
			: _worker(worker)
			, _prev(_worker->_try)
			, _ex(nullptr)
			, _msg(nullptr)
			, _rethrow(false) {
			_worker->_try = this;
		}

		~JscTryCatch() {
			_worker->_try = _prev;
			if (_rethrow && _ex) { // rethrow to parent try catch
				if (_prev) {
					_prev->_ex = _ex;
					_prev->_msg = _msg;
					_ex = nullptr;
				} else {
					_worker->reportException(_ex);
				}
			}
			if (_ex) {
				JSValueProtect(_worker->_ctx, _ex);
			}
		}

		JSValueRef exception() const {
			return _ex;
		}

		Message* message() const {
			if (!_msg) {
				_msg = Message::Create(_worker, ex);
			}
			return _msg;
		}

		void reThrow() {
			_rethrow = true;
		}

		void caught(JSObjectRef ex) {
			DCHECK(_try->_ex == nullptr, "Throw too many exceptions");
			_try->_ex = ex;
			JSValueProtect(_worker->_ctx, _ex);
		}

		void reset() {
			if (!_rethrow) {
				if (_ex) {
					JSValueUnprotect(_worker->_ctx, _ex);
					_ex = nullptr;
				}
				_msg = nullptr;
			}
		}

	private:
		JscWorker* _worker;
		JscTryCatch* _prev;
		JSObjectRef _ex;
		Sp<Message> _msg;
		bool _rethrow;
		friend class JscWorker;
	};

	TryCatch::TryCatch(Worker *worker): _val(new JscTryCatch(WORKER(worker))) {
	}

	TryCatch::~TryCatch() {
		delete static_cast<JscTryCatch*>(_val);
	}

	bool TryCatch::hasCaught() const {
		return static_cast<JscTryCatch*>(_val)->exception(); 
	}

	JSValue* TryCatch::exception() const {
		return Cast(static_cast<JscTryCatch*>(_val)->exception());
	}

	void TryCatch::reThrow() {
		static_cast<JscTryCatch*>(_val)->reThrow(); 
	}

	void TryCatch::print() const {
		//wrap->_worker->print_exception(wrap->_try.Message(), wrap->_try.Exception());
		// ((JscTryCatch*)_val)->print_exception();
	}

	void JscWorker::throwException(JSValueRef ex) {
		JSObjectRef _ex = const_cast<JSObjectRef>(ex);
		if (!JSValueIsObject(_ctx, ex)) {
			_ex = JSObjectCallAsConstructor(_ctx, _ctxData.Error, 1, &ex, nullptr);
			DCHECK(_ex);
		}
		if (_try) {
			_try->caught(_ex);
		} else {
			reportException(_ex);
		}
	}

	void JscWorker::reportException(JSObjectRef ex) {
		Sp<Message> msg = Message::Create(this, ex);
		if (_worker->_messageListener) {
			_worker->_messageListener(*msg, Cast(ex));
		} else {
			msg->PrintLog(); // print exception message
		}
	}

} }