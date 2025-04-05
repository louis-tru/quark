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
#include "../../errno.h"
#include <sstream>
#include <iostream>

namespace qk { namespace js {

	static JSStringRef anonymous_s = JsStringWithUTF8("<anonymous>").collapse();
	static JSStringRef eval_s = JsStringWithUTF8("<eval>").collapse();

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
				JSObjectSetPropertyAtIndex(jscc(), _handle, id++, val, &ex);
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
				if (ch[-1] == '@' && ch[-2] != '\0') {
					ch[-1] = '\0';
					_function_name = JSValueMakeString(ctx, *JsStringWithUTF8(value));
				} else {
					_function_name = JSValueMakeString(ctx, anonymous_s);
				}
				ref(_function_name);

				value = ch;

				// script_name
				if (value[0] == '[' && (ch = strchr(value, ']'))) {
					*ch = '\0';
					_script_name = JSValueMakeString(ctx, *JsStringWithUTF8(value + 1));
					ref(_script_name);
					value = ch + 1;
				} else {
					_script_name = worker->_data.EmptyString;
				}
				// line / column
				if (value[0] == ':' && (ch = strchr(value + 1, ':'))) {
					*ch = '\0';
					sscanf(value + 1, "%d", &_lineNumber);
					value = ch + 1;
					sscanf(value, "%d", &_column);
				}
			} else {
				// B
				// run@[test.js]:25:38
				// global code@[test.js]:45:4
				_is_eval = true;
				_function_name = JSValueMakeString(ctx, *JsStringWithUTF8(value));
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
						<< *script_name << ":" << _lineNumber << ":" << _column << ")\n";
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
		int _lineNumber = 0;
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
			if (!JSValueIsUndefined(jscc(), stack)) {
				auto stackStr = jsToString(jscc(), stack).collapse();
				char* value = *stackStr;
				char* end = value + stackStr.length();
				while (value < end) {
					char* ch = strchr(value, '\n');
					if (ch) {
						*ch = '\0';
					} else {
						ch = end;
					}
					_frames.push(new StackFrame(worker, value));
					value = ch + 1;
				}
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
			auto end = Qk_Minus(_frames.length(), 1);
			for (int i = 0; i < _frames.length(); i++) {
				_frames[i]->Print(stream, space, i == end);
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
															JSValueMakeNumber(ctx, item->_lineNumber), 0, OK(nullptr));
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
	};

	class Message: public JscRef {
	public:
		Message(JscWorker* worker,
						JSObjectRef exception,
						JSValueRef message,
						JSValueRef sourceLine,
						JSValueRef sourceURL,
						StackTrace* stack)
		: JscRef(worker)
		, _exception(exception)
		, _message(message)
		, _sourceLine(sourceLine ? sourceLine: worker->_data.EmptyString)
		, _sourceURL(sourceURL ? sourceURL: worker->_data.EmptyString)
		, _startPosition(0)
		, _endPosition(0)
		, _lineNumber(0)
		, _startColumn(0)
		, _endColumn(0)
		, _stack_trace(stack)
		{
			ref(_exception);
			ref(_message);
			ref(_sourceLine);
			ref(_sourceURL);
		}

		~Message() {
			Releasep(_stack_trace);
		}

		void Print(std::stringstream& stream, const std::string& space = "  ") {
			auto ctx = jscc();

			// Print (filename):(line number): (message).
			stream << *jsToString(ctx, _sourceURL) << ":" << _lineNumber << ": " << *jsToString(ctx, _message);

			auto sourceline = jsToString(ctx, _sourceLine).trim();
			if (!sourceline.isEmpty()) {
				stream << "\n";
				// Print line of source code.
				stream << *sourceline << "\n";

				// Print line of source code position.
				for (int i = 0; i < _startColumn; i++) {
					stream << (sourceline[i] == 9 ? "\t": " ");
				}
				for (int i = _startColumn; i < _endColumn; i++) {
					stream << "^";
				}
			}
			// Print stack.
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
									_stack_trace->GetFrame(0)->ScriptName(): _worker->_data.EmptyString;
		}

		static Sp<Message> Create(JscWorker* w, JSValueRef exception_) {
			DCHECK(exception_);
			ENV(w);
			auto exception = (JSObjectRef)exception_;
			if (!JSValueIsObject(ctx, exception)) {
				ex = JSObjectCallAsConstructor(ctx, worker->_data.global_Error, 1, &exception, 0);
				DCHECK(ex);
			}
			auto line = JSObjectGetProperty(ctx, exception, line_s, JsFatal(nullptr));
			auto column = JSObjectGetProperty(ctx, exception, column_s, JsFatal(nullptr));
			auto sourceURL = JSObjectGetProperty(ctx, exception, sourceURL_s, JsFatal(nullptr));
			// column: {value: 34, writable: true, enumerable: false, configurable: true}
			// line: {value: 3, writable: true, enumerable: false, configurable: true}
			// message: {value: "ABCDEFG", writable: true, enumerable: false, configurable: true}
			// stack: {value: "AA@↵BB@↵global code@", writable: true, enumerable: false, configurable: true}

			auto lineNumber = JSValueToNumber(ctx, line, JsFatal(nullptr));
			auto startColumn = JSValueToNumber(ctx, column, JsFatal(nullptr));
			auto message_str = JsValueToStringCopy(ctx, exception, JsFatal(nullptr));
			auto message = JSValueMakeString(ctx, *message_str);
			auto sourceURL_IsString = JSValueIsString(ctx, sourceURL);

			bool is__stack = JSObjectHasProperty(ctx, exception, _stack_s);
			auto stack = JSObjectGetProperty(ctx, exception, is__stack ? _stack_s: stack_s, JsFatal(nullptr));
			DCHECK(stack);
			auto stackTrace = new StackTrace(worker, stack);
			auto msg = new Message(worker, exception, message, nullptr,
				sourceURL_IsString ? sourceURL: nullptr, stackTrace
			);
			msg->_lineNumber = lineNumber;
			msg->_startColumn = startColumn;
			msg->_endColumn = startColumn;

			if (!is__stack) { // no set
				JSPropertyAttributes attrs = kJSPropertyAttributeReadOnly |
					kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete;
				JSObjectSetProperty(ctx, exception, _stack_s, stack, attrs, JsFatal(nullptr));
				std::stringstream stream;
				msg->Print(stream);
				std::string r = stream.str();
				auto newStack = JSValueMakeString(ctx, *JsStringWithUTF8(r.c_str()));
				JSObjectSetProperty(ctx, exception, stack_s, newStack, attrs, JsFatal(nullptr));
			}

			return msg;
		}

	private:
		JSObjectRef _exception;
		JSValueRef _message;
		JSValueRef _sourceLine;
		JSValueRef _sourceURL;
		int _startPosition;
		int _endPosition;
		int _lineNumber;
		int _startColumn;
		int _endColumn;
		StackTrace* _stack_trace;
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
				JSValueUnprotect(_worker->_ctx, _ex);
			}
		}

		JSValueRef exception() const {
			return _ex;
		}

		Message* message() {
			if (!_msg) {
				_msg = Message::Create(_worker, _ex);
			}
			return *_msg;
		}

		void reThrow() {
			_rethrow = true;
		}

		void caught(JSValueRef ex) {
			CHECK(_ex == nullptr, "Throw too many exceptions");
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

	void defalutMessageListener(Worker* w, JSValue* ex) {
		DCHECK(ex);
		if (!triggerUncaughtException(WORKER(w), ex)) {
			Message::Create(WORKER(w), Back(ex))->PrintLog();
			qk::thread_exit(ERR_UNCAUGHT_EXCEPTION);
		}
	}

	void defalutPromiseRejectListener(Worker* w, JSValue* reason, JSValue* promise) {
		if (!reason)
			reason = w->newUndefined();
		if (!triggerUnhandledRejection(WORKER(w), reason, promise)) {
			Message::Create(WORKER(w), Back(reason))->PrintLog();
			qk::thread_exit(ERR_UNHANDLED_REJECTION);
		}
	}

	void JscWorker::throwException(JSValueRef ex) {
		CHECK(_ex == nullptr, "Throw too many exceptions");
		if (_try) {
			_try->caught(ex);
		} else if (_callStack == 0) {
			defalutMessageListener(this, Cast(ex));
		} else {
			Qk_DLog(jsToString(_ctx, ex));
			_ex = ex;
		}
	}

} }
