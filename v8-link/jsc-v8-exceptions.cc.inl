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

// --- E x c e p t i o n s ---

v8::TryCatch::TryCatch()
: isolate_(i::Isolate::Current()),
next_(isolate_->m_try_catch),
is_verbose_(false),
can_continue_(true),
capture_message_(true),
rethrow_(false),
has_terminated_(false) {
  isolate_->m_try_catch = this;
}

v8::TryCatch::TryCatch(v8::Isolate* isolate)
: isolate_(reinterpret_cast<i::Isolate*>(isolate)),
next_(isolate_->m_try_catch),
exception_(nullptr),
message_obj_(nullptr),
js_stack_comparable_address_(nullptr),
is_verbose_(false),
can_continue_(true),
capture_message_(true),
rethrow_(false),
has_terminated_(false) {
  isolate_->m_try_catch = this;
}

v8::TryCatch::~TryCatch() {
  auto exception = isolate_->m_exception;
  if (rethrow_ && exception) { // rethrow
    if (next_) {
      next_->message_obj_ = message_obj_;
      next_->has_terminated_ = has_terminated_;
      next_->can_continue_ = can_continue_;
      isolate_->m_try_catch = next_;
    } else {
      isolate_->m_try_catch = nullptr;
      isolate_->ThrowException(exception, (i::Message*)message_obj_);
    }
  } else {
    isolate_->m_try_catch = next_;
  }
}

void* v8::TryCatch::operator new(size_t) { UNREACHABLE(); }

void v8::TryCatch::operator delete(void*, size_t) { UNREACHABLE(); }

bool v8::TryCatch::HasCaught() const {
  return isolate_->m_exception;
}

bool v8::TryCatch::CanContinue() const {
  return can_continue_;
}

bool v8::TryCatch::HasTerminated() const {
  return has_terminated_;
}

v8::Local<v8::Value> v8::TryCatch::ReThrow() {
  if (!HasCaught())
    return v8::Local<v8::Value>();
  rethrow_ = true;
  return v8::Undefined(reinterpret_cast<v8::Isolate*>(isolate_));
}

v8::Local<Value> v8::TryCatch::Exception() const {
  return i::Cast(isolate_->m_exception);
}

MaybeLocal<Value> v8::TryCatch::StackTrace(Local<Context> context) const {
  ENV(context->GetIsolate());
  if (isolate_->m_exception) {
    DCHECK(JSValueIsObject(ctx, (JSValueRef)exception_));
    auto r = JSObjectGetProperty(ctx, (JSObjectRef)isolate_->m_exception, i::stack_s, 0);
    DCHECK(r);
    return i::Cast(r);
  }
  return Local<Value>();
}

v8::Local<Value> v8::TryCatch::StackTrace() const {
  auto context = isolate_->GetCurrentContext();
  RETURN_TO_LOCAL_UNCHECKED(StackTrace(context), Value);
}

v8::Local<v8::Message> v8::TryCatch::Message() const {
  return *reinterpret_cast<Local<v8::Message>*>(const_cast<void**>(&message_obj_));
}

void v8::TryCatch::Reset() {
  if (!rethrow_) {
    isolate_->m_exception = nullptr;
    message_obj_ = nullptr;
  }
  ResetInternal();
}

void v8::TryCatch::ResetInternal() {
}

void v8::TryCatch::SetVerbose(bool value) {
  is_verbose_ = value;
}

bool v8::TryCatch::IsVerbose() const { return is_verbose_; }

void v8::TryCatch::SetCaptureMessage(bool value) {
  capture_message_ = value;
}

// --- M e s s a g e ---

v8_ns(internal)

class StackFrame: public Wrap {
 public:
  StackFrame(Isolate* isolate, char* value): Wrap(isolate) {
    auto ctx = isolate->jscc();
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
        m_function_name = JSValueMakeString(ctx, JSStringCreateWithUTF8CString(value));
        value = ch;
      } else {
        m_function_name = JSValueMakeString(ctx, anonymous_s);
      }
      Reference(m_function_name);
      
      // script_name
      if (value[0] == '[' && (ch = strchr(value, ']'))) {
        *ch = '\0';
        m_script_name = JSValueMakeString(ctx, JSStringCreateWithUTF8CString(value + 1));
        Reference(m_script_name);
        value = ch + 1;
      } else {
        m_script_name = isolate->Empty();
      }
      // line / column
      if (value[0] == ':' && (ch = strchr(value + 1, ':'))) {
        *ch = '\0';
        sscanf(value + 1, "%d", &m_line_number);
        value = ch + 1;
        sscanf(value, "%d", &m_column);
      }
    } else {
       // B
       // run@[test.js]:25:38
       // global code@[test.js]:45:4
      m_is_eval = true;
      m_function_name = JSValueMakeString(ctx, JSStringCreateWithUTF8CString(value));
      m_script_name = JSValueMakeString(ctx, eval_s);
      Reference(m_function_name);
      Reference(m_script_name);
    }
  }
  
  void Print(std::stringstream& stream, const std::string& space, bool line_feed) {
    // ReferenceError: KK is not defined
    //   at emit (events.js:210:7)
    //   at onLine (readline.js:279:10)
    v8::String::Utf8Value function_name(i::Cast<String>(m_function_name));
    v8::String::Utf8Value script_name(i::Cast<String>(m_script_name));
    stream << space << "at " << *function_name << " ("
           << *script_name << ":" << m_line_number << ":" << m_column << ")";
    if (line_feed) {
      stream << '\n';
    }
  }
  
  inline JSValueRef FunctionName() const {
    return m_function_name;
  }
  
  inline JSValueRef ScriptName() const {
    return m_script_name;
  }

 private:
  int m_line_number = 0;
  int m_column = 0;
  int m_script_id = 0;
  JSValueRef m_function_name = nullptr;
  JSValueRef m_script_name = nullptr;
  bool m_is_eval = false;
  bool m_is_constructor = false;
  bool m_is_wasm = false;
  friend class v8::StackFrame;
  friend class StackTrace;
  friend class Message;
};

class StackTrace: public Wrap {
public:
  StackTrace(Isolate* iso, JSValueRef stack)
  : Wrap(iso), m_as_array(nullptr) {
    ENV(iso);
    v8::String::Utf8Value utf8_value(i::Cast<String>(stack));
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
      auto frame = new StackFrame(iso, value);
      m_frames.push_back(frame);
      value = ch + 1;
      Reference(frame);
    }
  }
  
  inline int GetFrameCount() const {
    return (int)m_frames.size();
  }
  
  StackFrame* GetFrame(uint32_t index) const {
    if (index < m_frames.size()) {
      return m_frames[index];
    }
    return nullptr;
  }
  
  void Print(std::stringstream& stream, const std::string& space) {
    if (m_frames.size()) {
      for (int i = 0; i < m_frames.size() - 1; i++) {
        m_frames[i]->Print(stream, space, true);
      }
      m_frames[m_frames.size() - 1]->Print(stream, space, false);
    }
  }
  
  JSObjectRef AsArray() {
    if (!m_as_array) {
      ENV(GetIsolate());
      JSValueRef argv[1] = { JSValueMakeNumber(ctx, m_frames.size()) };
      JSObjectRef arr = JSObjectMakeArray(ctx, 1, argv, OK(0));
      
      for ( int i = 0 ; i < m_frames.size(); i++) {
        auto item = m_frames[i];
        auto jitem = JSObjectMake(ctx, 0, 0);
        DCHECK(item);
        JSObjectSetProperty(ctx, jitem, line_s,
                            JSValueMakeNumber(ctx, item->m_line_number), 0, OK(0));
        JSObjectSetProperty(ctx, jitem, column_s,
                            JSValueMakeNumber(ctx, item->m_column), 0, OK(0));
        JSObjectSetProperty(ctx, jitem, scriptId_s,
                            JSValueMakeNumber(ctx, item->m_script_id), 0, OK(0));
        JSObjectSetProperty(ctx, jitem, functionName_s, item->m_function_name, 0, OK(0));
        JSObjectSetProperty(ctx, jitem, scriptName_s, item->m_script_name, 0, OK(0));
        JSObjectSetProperty(ctx, jitem, isEval_s,
                            JSValueMakeBoolean(ctx, item->m_is_eval), 0, OK(0));
        JSObjectSetProperty(ctx, jitem, isConstructor_s,
                            JSValueMakeBoolean(ctx, item->m_is_constructor), 0, OK(0));
        JSObjectSetProperty(ctx, jitem, isWasm_s,
                            JSValueMakeBoolean(ctx, item->m_is_wasm), 0, OK(0));
        JSObjectSetPropertyAtIndex(ctx, arr, i, jitem, OK(0));
      }
      m_as_array = arr;
      Reference(arr);
    }
    return m_as_array;
  }
  
  static StackTrace* CurrentStackTrace(Isolate* iso) {
    ENV(iso);
    auto exception = JSObjectCallAsConstructor(ctx, iso->Error(), 0, nullptr, OK(0));
    DCHECK(exception);
    return new StackTrace(iso, exception);
  }
  
private:
  JSObjectRef m_as_array;
  std::vector<StackFrame*> m_frames;
  friend class v8::StackTrace;
};

class Message: public Wrap {
 public:
  Message(Isolate* isolate,
          JSValueRef exception,
          JSValueRef message, StackTrace* stack): Wrap(isolate)
  , m_exception(exception)
  , m_message(message)
  , m_source_line(isolate->Empty())
  , m_start_position(0)
  , m_end_position(0)
  , m_line_number(0)
  , m_start_column(0)
  , m_end_column(0)
  , m_stack_trace(stack)
  {
    Reference(m_exception);
    Reference(m_message);
    Reference(m_source_line);
    Reference(m_stack_trace);
  }
  
  void Print(std::stringstream& stream, const std::string& space = "    ") {
    v8::String::Utf8Value utf8(i::Cast(m_message));
    stream << *utf8;
    if (m_stack_trace->GetFrameCount()) {
      stream << "\n";
      m_stack_trace->Print(stream, space);
    }
  }
  
  static Message* CreateMessage(Isolate* iso, JSObjectRef exception) {
    DCHECK(exception);
    ENV(iso);
    if (!JSValueIsObject(ctx, exception)) {
      exception = JSObjectCallAsConstructor(ctx, iso->Error(), 1, &exception, 0);
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
    m->m_line_number = line2;
    m->m_start_column = column2;
    m->m_end_column = m->m_start_column;
    
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
      i::Cast<v8::Object>(exception)->
        DefineProperty(isolate->GetCurrentContext(), key, desc).FromJust();
    }
    return m;
  }
  
  ScriptOrigin GetScriptOrigin() const {
    auto isolate = reinterpret_cast<v8::Isolate*>(GetIsolate());
    return ScriptOrigin(GetScriptResourceName(),
                        Integer::New(isolate, m_line_number),
                        Integer::New(isolate, m_start_column));
  }
  
  Local<Value> GetScriptResourceName() const {
    return Cast(m_stack_trace->GetFrameCount() ?
                m_stack_trace->GetFrame(0)->ScriptName(): GetIsolate()->Empty());
  }
  
 private:
  JSValueRef m_exception;
  JSValueRef m_message;
  JSValueRef m_source_line;
  int m_start_position;
  int m_end_position;
  int m_line_number;
  int m_start_column;
  int m_end_column;
  StackTrace* m_stack_trace;
  friend class v8::Message;
};

void Isolate::PrintMessage(Message* message) {
  std::stringstream stream;
  message->Print(stream);
  std::string r = stream.str();
  LOG(r);
}

v8_ns_end

Local<String> Message::Get() const {
  return i::Cast<String>(reinterpret_cast<const i::Message*>(this)->m_message);
}

Local<String> Message::GetSourceLine() const {
  return i::Cast<String>(reinterpret_cast<const i::Message*>(this)->m_source_line);
}

MaybeLocal<String> Message::GetSourceLine(Local<Context> context) const {
  return i::Cast<String>(reinterpret_cast<const i::Message*>(this)->m_source_line);
}

ScriptOrigin Message::GetScriptOrigin() const {
  return reinterpret_cast<const i::Message*>(this)->GetScriptOrigin();
}

Local<Value> Message::GetScriptResourceName() const {
  return reinterpret_cast<const i::Message*>(this)->GetScriptResourceName();
}

Local<StackTrace> Message::GetStackTrace() const {
  auto r = reinterpret_cast<const i::Message*>(this)->m_stack_trace;
  i::Cast<StackTrace>(r);
}

int Message::GetLineNumber() const {
  return reinterpret_cast<const i::Message*>(this)->m_line_number;
}

Maybe<int> Message::GetLineNumber(Local<Context> context) const {
  return Just(reinterpret_cast<const i::Message*>(this)->m_line_number);
}

int Message::GetStartPosition() const {
  return reinterpret_cast<const i::Message*>(this)->m_start_position;
}

int Message::GetEndPosition() const {
  return reinterpret_cast<const i::Message*>(this)->m_end_position;
}

int Message::ErrorLevel() const {
  return 0;
}

int Message::GetStartColumn() const {
  return reinterpret_cast<const i::Message*>(this)->m_start_column;
}

Maybe<int> Message::GetStartColumn(Local<Context> context) const {
  return Just(reinterpret_cast<const i::Message*>(this)->m_start_column);
}

int Message::GetEndColumn() const {
  return reinterpret_cast<const i::Message*>(this)->m_end_column;
}

Maybe<int> Message::GetEndColumn(Local<Context> context) const {
  return Just(reinterpret_cast<const i::Message*>(this)->m_end_column);
}

bool Message::IsSharedCrossOrigin() const {
  return false;
}

bool Message::IsOpaque() const {
  return false;
}

void Message::PrintCurrentStackTrace(Isolate* isolate, FILE* out) {
  auto r = i::StackTrace::CurrentStackTrace(ISOLATE(isolate));
  if (r) {
    DCHECK(out);
    std::stringstream stream;
    r->Print(stream, "");
    std::string str;
    stream >> str;
    fwrite(str.c_str(), sizeof(char*), str.size(), out);
  }
}

// --- S t a c k T r a c e ---

Local<StackFrame> StackTrace::GetFrame(uint32_t index) const {
  auto f = reinterpret_cast<const i::StackTrace*>(this)->GetFrame(index);
  return i::Cast<StackFrame>(f);
}

int StackTrace::GetFrameCount() const {
  return reinterpret_cast<const i::StackTrace*>(this)->GetFrameCount();
}

Local<Array> StackTrace::AsArray() {
  auto r = reinterpret_cast<i::StackTrace*>(this)->AsArray();
  return i::Cast<Array>(r);
}

Local<StackTrace> StackTrace::CurrentStackTrace(Isolate* isolate,
                                                int frame_limit,
                                                StackTraceOptions options) {
  auto r = i::StackTrace::CurrentStackTrace(ISOLATE(isolate));
  return i::Cast<StackTrace>(r);
}

// --- S t a c k F r a m e ---

int StackFrame::GetLineNumber() const {
  return reinterpret_cast<const i::StackFrame*>(this)->m_line_number;
}

int StackFrame::GetColumn() const {
  return reinterpret_cast<const i::StackFrame*>(this)->m_column;
}

int StackFrame::GetScriptId() const {
  return reinterpret_cast<const i::StackFrame*>(this)->m_script_id;
}

Local<String> StackFrame::GetScriptName() const {
  return i::Cast<String>(reinterpret_cast<const i::StackFrame*>(this)->m_script_name);
}

Local<String> StackFrame::GetScriptNameOrSourceURL() const {
  return i::Cast<String>(reinterpret_cast<const i::StackFrame*>(this)->m_script_name);
}

Local<String> StackFrame::GetFunctionName() const {
  return i::Cast<String>(reinterpret_cast<const i::StackFrame*>(this)->m_function_name);
}

bool StackFrame::IsEval() const {
  return reinterpret_cast<const i::StackFrame*>(this)->m_is_eval;
}

bool StackFrame::IsConstructor() const {
  return reinterpret_cast<const i::StackFrame*>(this)->m_is_constructor;
}

bool StackFrame::IsWasm() const {
  return reinterpret_cast<const i::StackFrame*>(this)->m_is_wasm;
}

// --- E x c e p t i o n ---

#define DEFINE_ERROR(NAME, name)  \
  Local<Value> Exception::NAME(v8::Local<v8::String> raw_message) { \
    DCHECK(!raw_message.IsEmpty()); \
    ENV(); \
    JSValueRef argv[1] = { i::Back(raw_message) }; \
    auto error = JSObjectCallAsConstructor(ctx, isolate->NAME(), 1, argv, 0); \
    DCHECK(error); \
    return i::Cast(error); \
  }

DEFINE_ERROR(RangeError, range_error)
DEFINE_ERROR(ReferenceError, reference_error)
DEFINE_ERROR(SyntaxError, syntax_error)
DEFINE_ERROR(TypeError, type_error)
DEFINE_ERROR(Error, error)

#undef DEFINE_ERROR

Local<Message> Exception::CreateMessage(Isolate* isolate, Local<Value> exception) {
  auto msg = i::Message::CreateMessage(ISOLATE(isolate), i::Back<JSObjectRef>(exception));
  return i::Cast<Message>(msg);
}

Local<Message> Exception::CreateMessage(Local<Value> exception) {
  return CreateMessage(Isolate::GetCurrent(), exception);
}

Local<StackTrace> Exception::GetStackTrace(Local<Value> exception) {
  return Exception::CreateMessage(exception)->GetStackTrace();
}
