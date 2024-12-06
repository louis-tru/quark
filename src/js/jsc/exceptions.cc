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
#include <sstream>

namespace qk { namespace js {

	static JSStringRef anonymous_s = JsStringWithUTF8("<anonymous>");
	static JSStringRef eval_s = JsStringWithUTF8("<eval>");

	// --- M e s s a g e ---

	class JscRef {
	public:
		JscRef(JscWorker* worker): _worker(worker) {
			_handle = JSObjectMake(jscc(), nullptr, nullptr);
			DCHECK(_handle);
			JSValueProtect(jscc(), _handle);
		}
		~JscRef() {
			JSValueUnprotect(jscc(), _handle);
		}
		void ref(JSValueRef val) {
			if (val) {
				JSValueRef ex = 0;
				static int id = 0;
				JSObjectSetPropertyAtIndex(jscc(), Handle(), id++, val, &ex);
				DCHECK(!ex);
			}
		}
		inline JSGlobalContextRef jscc() { return _worker->jscc(); }
	protected:
		JscWorker*  _worker;
		JSObjectRef _handle;
	};

	class StackFrame: public JscRef {
	public:
		StackFrame(JscWorker* worker, char* value): JscRef(worker) {
			auto ctx = jscc();
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
					_function_name = JSValueMakeString(ctx, JsStringWithUTF8(value));
					value = ch;
				} else {
					_function_name = JSValueMakeString(ctx, anonymous_s);
				}
				ref(_function_name);

				// script_name
				if (value[0] == '[' && (ch = strchr(value, ']'))) {
					*ch = '\0';
					_script_name = JSValueMakeString(ctx, JsStringWithUTF8(value + 1));
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
				_function_name = JSValueMakeString(ctx, JsStringWithUTF8(value));
				_script_name = JSValueMakeString(ctx, eval_s);
				ref(_function_name);
				ref(_script_name);
			}
		}

		void Print(std::stringstream& stream, const std::string& space, bool line_feed) {
			// ReferenceError: KK is not defined
			//   at emit (events.js:210:7)
			//   at onLine (readline.js:279:10)
			String function_name = jsToString(jscc(), _function_name);
			String script_name = jsToString(jscc(), _script_name);

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
		StackTrace(JscWorker* worker, JSValueRef stack): JscRef(worker), _asArray(nullptr) {
			auto stackStr = jsToString(jscc(), stack);
			char* value = *stackStr;
			char* ch = nullptr;
			while (*value != '\0') {
				char* ch = strchr(value, '\n');
				if (ch) {
					*ch = '\0';
				} else {
					ch = *stackStr + stackStr.length() - 1;
				}
				_frames.push(new StackFrame(worker, value));
				value = ch + 1;
			}
		}

		~StackTrace() {
			for (auto i: _frames) {
				delete i;
			}
		}

		inline int GetFrameCount() const {
			return (int)_frames.length();
		}

		StackFrame* GetFrame(uint32_t index) const {
			if (index < _frames.length()) {
				return _frames[index];
			}
			return nullptr;
		}

		void Print(std::stringstream& stream, const std::string& space) {
			if (_frames.length()) {
				for (int i = 0; i < _frames.length() - 1; i++) {
					_frames[i]->Print(stream, space, true);
				}
				_frames.back()->Print(stream, space, false);
			}
		}

		JSObjectRef AsArray() {
			if (!_asArray) {
				ENV(_worker);
				JSValueRef argv[1] = { JSValueMakeNumber(ctx, _frames.length()) };
				JSObjectRef arr = JSObjectMakeArray(ctx, 1, argv, OK(0));

				for ( int i = 0 ; i < _frames.length(); i++) {
					auto item = _frames[i];
					auto jitem = JSObjectMake(ctx, 0, 0);
					DCHECK(item);
					JSObjectSetProperty(ctx, jitem, line_s,
															JSValueMakeNumber(ctx, item->_line_number), 0, OK(nullptr));
					JSObjectSetProperty(ctx, jitem, column_s,
															JSValueMakeNumber(ctx, item->_column), 0, OK(nullptr));
					JSObjectSetProperty(ctx, jitem, scriptId_s,
															JSValueMakeNumber(ctx, item->_script_id), 0, OK(nullptr));
					JSObjectSetProperty(ctx, jitem, functionName_s, item->_function_name, 0, OK(nullptr));
					JSObjectSetProperty(ctx, jitem, scriptName_s, item->_script_name, 0, OK(nullptr));
					JSObjectSetProperty(ctx, jitem, isEval_s,
															JSValueMakeBoolean(ctx, item->_is_eval), 0, OK(nullptr));
					JSObjectSetProperty(ctx, jitem, isConstructor_s,
															JSValueMakeBoolean(ctx, item->_is_constructor), 0, OK(nullptr));
					JSObjectSetProperty(ctx, jitem, isWasm_s,
															JSValueMakeBoolean(ctx, item->_is_wasm), 0, OK(nullptr));
					JSObjectSetPropertyAtIndex(ctx, arr, i, jitem, OK(nullptr));
				}
				_asArray = arr;
				ref(arr);
			}
			return _asArray;
		}

	private:
		JSObjectRef _asArray;
		Array<StackFrame*> _frames;
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
			Releasep(_stack_trace);
		}

		void Print(std::stringstream& stream, const std::string& space = "    ") {
			stream << *jsToString(_worker->jsctx(), _message);
			if (_stack_trace->GetFrameCount()) {
				stream << "\n";
				_stack_trace->Print(stream, space);
			}
		}

		void PrintLog() {
			std::stringstream stream;
			Print(stream);
			std::string r = stream.str();
			Qk_Log("%s", r.c_str());
		}

		JSValueRef GetScriptResourceName() const {
			return _stack_trace->GetFrameCount() ?
									_stack_trace->GetFrame(0)->ScriptName(): _worker->_data.Empty;
		}

		static Message* Create(JscWorker* w, JSValueRef ex_) {
			DCHECK(ex_);
			ENV(w);
			auto ex = (JSObjectRef)ex_;
			if (!JSValueIsObject(ctx, ex)) {
				ex = JSObjectCallAsConstructor(ctx, worker->_globalData.Error, 1, &ex, 0);
				DCHECK(ex);
			}
			auto line = JSObjectGetProperty(ctx, ex, line_s, OK(nullptr));
			auto column = JSObjectGetProperty(ctx, ex, column_s, OK(nullptr));
			JSStringRef message_str = JSValueToStringCopy(ctx, ex, OK(nullptr));
			JSValueRef message = JSValueMakeString(ctx, message_str);
			JSValueRef stack = nullptr;
			bool has_set_stack = JSObjectHasProperty(ctx, ex, _stack_s);
			if (has_set_stack) {
				stack = JSObjectGetProperty(ctx, ex, _stack_s, nullptr);
			} else {
				stack = JSObjectGetProperty(ctx, ex, stack_s, nullptr);
			}
			DCHECK(stack);
			auto stack_trace = new StackTrace(worker, stack);
			auto m = new Message(worker, ex, message, stack_trace);
			m->_line_number = JSValueToNumber(ctx, line, OK(nullptr));
			m->_start_column = JSValueToNumber(ctx, column, OK(nullptr));
			m->_end_column = m->_start_column;

			if (!has_set_stack) { // no set
				JSPropertyAttributes attrs = kJSPropertyAttributeReadOnly |
					kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete;
				JSObjectSetProperty(ctx, ex, _stack_s, stack, attrs, OK(nullptr));
				std::stringstream stream;
				m->Print(stream);
				std::string r = stream.str();
				auto newStack = JSValueMakeString(ctx, JsStringWithUTF8(r.c_str()));
				JSObjectSetProperty(ctx, ex, stack_s, newStack, attrs, OK(nullptr));
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
			if (_ex) {
				if (_rethrow) { // rethrow to parent try catch
					_worker->throwException(_ex);
				}
				JSValueUnpotect(_worker->_ctx, _ex);
			}
		}

		JSValueRef exception() const {
			return _ex;
		}

		Message* message() {
			if (!_msg) {
				_msg = Message::Create(_worker, _ex);
			}
			return _msg;
		}

		void reThrow() {
			_rethrow = true;
		}

		void caught(JSValueRef ex) {
			DCHECK(_ex == nullptr, "Throw too many exceptions");
			_ex = ex;
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
		JSValueRef _ex;
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
		static_cast<JscTryCatch*>(_val)->message()->PrintLog();
	}

	void JscWorker::throwException(JSValueRef ex) {
		if (_try) {
			_try->caught(ex);
		} else if (_callStack == 0) {
			DCHECK(_ex == nullptr);
			reportException(ex);
		} else {
			CHECK(_ex == nullptr, "Throw too many exceptions");
			_ex = ex;
		}
	}

	void JscWorker::reportException(JSValueRef ex) {
		Sp<Message> msg = Message::Create(this, ex);
		if (_worker->_messageListener) {
			_worker->_messageListener(*msg, Cast(ex));
		} else {
			msg->PrintLog(); // print exception message
		}
	}

} }