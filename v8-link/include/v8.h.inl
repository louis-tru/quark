// Copyright 2012 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** \mainpage V8 API Reference Guide
 *
 * V8 is Google's open source JavaScript engine.
 *
 * This set of documents provides reference material generated from the
 * V8 header file, include/v8.h.
 *
 * For other documentation see http://code.google.com/apis/v8/
 */

#ifndef V8_DEATH_RESET
#if USE_JSC
# define V8_DEATH_RESET Empty
#else
# define V8_DEATH_RESET Reset
#endif
#endif

#if USE_JSC

// --- Implementation ---

namespace internal {
//
const int kApiPointerSize = sizeof(void*);  // NOLINT
const int kApiIntSize = sizeof(int);  // NOLINT
const int kApiInt64Size = sizeof(int64_t);  // NOLINT

/**
 * This class exports constants and functionality from within v8 that
 * is necessary to implement inline functions in the v8 api.  Don't
 * depend on functions and constants defined here.
 */
class Internals {
 public:
  static const int kUndefinedValueRootIndex = 4;
  static const int kTheHoleValueRootIndex = 5;
  static const int kNullValueRootIndex = 6;
  static const int kTrueValueRootIndex = 7;
  static const int kFalseValueRootIndex = 8;
  static const int kEmptyStringRootIndex = 9;
  static const uint32_t kNumIsolateDataSlots = 4;
  
  V8_EXPORT static void CheckInitializedImpl(v8::Isolate* isolate);
  V8_INLINE static void CheckInitialized(v8::Isolate* isolate) {
#ifdef V8_ENABLE_CHECKS
    CheckInitializedImpl(isolate);
#endif
  }
  static void SetEmbedderData(v8::Isolate* isolate, uint32_t slot, void* data);
  static void* GetEmbedderData(const v8::Isolate* isolate, uint32_t slot);

  V8_INLINE static internal::Object** GetRoot(v8::Isolate* isolate, int index) {
    uint8_t* addr = reinterpret_cast<uint8_t*>(isolate);
    return reinterpret_cast<internal::Object**>(addr + index * kApiPointerSize);
  }
  static bool IsWeak(internal::Object* o);
  static bool IsNearDeath(internal::Object* o);
  static void MakeIndependent(internal::Object* o);
  static void MarkActive(internal::Object* o);
};
//
}  // namespace internal


template <class T>
Local<T> Local<T>::New(Isolate* isolate, Local<T> that) {
  return New(isolate, that.val_);
}

template <class T>
Local<T> Local<T>::New(Isolate* isolate, const PersistentBase<T>& that) {
  return New(isolate, that.val_);
}


template <class T>
Local<T> Local<T>::New(Isolate* isolate, T* that) {
  if (that == NULL) return Local<T>();
  T* that_ptr = that;
  internal::Object* p = reinterpret_cast<internal::Object*>(that_ptr);
  return Local<T>(reinterpret_cast<T*>(HandleScope::CreateHandle(
    reinterpret_cast<internal::Isolate*>(isolate), p)));
}


template<class T>
template<class S>
void Eternal<T>::Set(Isolate* isolate, Local<S> handle) {
  TYPE_CHECK(T, S);
  val_ = reinterpret_cast<T*>(
      V8::Eternalize(isolate, reinterpret_cast<Value*>(*handle)));
}

template <class T>
Local<T> Eternal<T>::Get(Isolate* isolate) const {
  // The eternal handle will never go away, so as with the roots, we don't even
  // need to open a handle.
  return Local<T>(val_);
}


template <class T>
Local<T> MaybeLocal<T>::ToLocalChecked() {
  if (V8_UNLIKELY(val_ == nullptr)) V8::ToLocalEmpty();
  return Local<T>(val_);
}


template <class T>
void* WeakCallbackInfo<T>::GetInternalField(int index) const {
#ifdef V8_ENABLE_CHECKS
  if (index < 0 || index >= kEmbedderFieldsInWeakCallback) {
    V8::InternalFieldOutOfBounds(index);
  }
#endif
  return embedder_fields_[index];
}


template <class T>
T* PersistentBase<T>::New(Isolate* isolate, T* that) {
  if (that == NULL) return NULL;
  internal::Object** p = reinterpret_cast<internal::Object**>(that);
  return reinterpret_cast<T*>(
      V8::GlobalizeReference(reinterpret_cast<internal::Isolate*>(isolate),
                             p));
}


template <class T, class M>
template <class S, class M2>
void Persistent<T, M>::Copy(const Persistent<S, M2>& that) {
  TYPE_CHECK(T, S);
  this->Reset();
  if (that.IsEmpty()) return;
  internal::Object** p = reinterpret_cast<internal::Object**>(that.val_);
  this->val_ = reinterpret_cast<T*>(V8::CopyPersistent(p));
  M::Copy(that, this);
}


template <class T>
bool PersistentBase<T>::IsIndependent() const {
  return false;
}


template <class T>
bool PersistentBase<T>::IsNearDeath() const {
  typedef internal::Internals I;
  if (this->IsEmpty()) return false;
  return I::IsNearDeath(reinterpret_cast<internal::Object*>(this->val_));
}


template <class T>
bool PersistentBase<T>::IsWeak() const {
  typedef internal::Internals I;
  if (this->IsEmpty()) return false;
  return I::IsWeak(reinterpret_cast<internal::Object*>(this->val_));
}


template <class T>
void PersistentBase<T>::Reset() {
  if (this->IsEmpty()) return;
  V8::DisposeGlobal(reinterpret_cast<internal::Object**>(this->val_));
  val_ = 0;
}


template <class T>
template <class S>
void PersistentBase<T>::Reset(Isolate* isolate, const Local<S>& other) {
  TYPE_CHECK(T, S);
  Reset();
  if (other.IsEmpty()) return;
  this->val_ = New(isolate, other.val_);
}


template <class T>
template <class S>
void PersistentBase<T>::Reset(Isolate* isolate,
                              const PersistentBase<S>& other) {
  TYPE_CHECK(T, S);
  Reset();
  if (other.IsEmpty()) return;
  this->val_ = New(isolate, other.val_);
}


template <class T>
template <typename P>
V8_INLINE void PersistentBase<T>::SetWeak(
    P* parameter, typename WeakCallbackInfo<P>::Callback callback,
    WeakCallbackType type) {
  typedef typename WeakCallbackInfo<void>::Callback Callback;
  V8::MakeWeak(reinterpret_cast<internal::Object**>(this->val_), parameter,
               reinterpret_cast<Callback>(callback), type);
}

template <class T>
void PersistentBase<T>::SetWeak() {
  V8::MakeWeak(reinterpret_cast<internal::Object***>(&this->val_));
}

template <class T>
template <typename P>
P* PersistentBase<T>::ClearWeak() {
  return reinterpret_cast<P*>(
    V8::ClearWeak(reinterpret_cast<internal::Object**>(this->val_)));
}

template <class T>
void PersistentBase<T>::RegisterExternalReference(Isolate* isolate) const {
  if (IsEmpty()) return;
  V8::RegisterExternallyReferencedObject(
      reinterpret_cast<internal::Object**>(this->val_),
      reinterpret_cast<internal::Isolate*>(isolate));
}

template <class T>
void PersistentBase<T>::MarkIndependent() {
  if (IsEmpty()) return;
  internal::Internals::MakeIndependent(
     reinterpret_cast<internal::Object*>(this->val_));
}

template <class T>
void PersistentBase<T>::MarkActive() {
  if (IsEmpty()) return;
  internal::Internals::MarkActive(
     reinterpret_cast<internal::Object*>(this->val_));
}


template <class T>
void PersistentBase<T>::SetWrapperClassId(uint16_t class_id) {
}


template <class T>
uint16_t PersistentBase<T>::WrapperClassId() const {
  return 0;
}


template<typename T>
ReturnValue<T>::ReturnValue(internal::Object** slot) : value_(slot) {}

template<typename T>
template<typename S>
void ReturnValue<T>::Set(const Persistent<S>& handle) {
  TYPE_CHECK(T, S);
  if (V8_UNLIKELY(handle.IsEmpty())) {
    *value_ = GetDefaultValue();
  } else {
    *value_ = reinterpret_cast<internal::Object*>(*handle);
  }
}

template <typename T>
template <typename S>
void ReturnValue<T>::Set(const Global<S>& handle) {
  TYPE_CHECK(T, S);
  if (V8_UNLIKELY(handle.IsEmpty())) {
    *value_ = GetDefaultValue();
  } else {
    *value_ = reinterpret_cast<internal::Object*>(*handle);
  }
}

template <typename T>
template <typename S>
void ReturnValue<T>::Set(const Local<S> handle) {
  TYPE_CHECK(T, S);
  if (V8_UNLIKELY(handle.IsEmpty())) {
    *value_ = GetDefaultValue();
  } else {
    *value_ = reinterpret_cast<internal::Object*>(*handle);
  }
}

template<typename T>
void ReturnValue<T>::Set(double i) {
  TYPE_CHECK(T, Number);
  Set(Number::New(GetIsolate(), i));
}

template<typename T>
void ReturnValue<T>::Set(int32_t i) {
  TYPE_CHECK(T, Integer);
  Set(Integer::New(GetIsolate(), i));
}

template<typename T>
void ReturnValue<T>::Set(uint32_t i) {
  TYPE_CHECK(T, Integer);
  // Can't simply use INT32_MAX here for whatever reason.
  bool fits_into_int32_t = (i & (1U << 31)) == 0;
  if (V8_LIKELY(fits_into_int32_t)) {
    Set(static_cast<int32_t>(i));
    return;
  }
  Set(Integer::NewFromUnsigned(GetIsolate(), i));
}

template<typename T>
void ReturnValue<T>::Set(bool value) {
  TYPE_CHECK(T, Boolean);
  typedef internal::Internals I;
  int root_index;
  if (value) {
    root_index = I::kTrueValueRootIndex;
  } else {
    root_index = I::kFalseValueRootIndex;
  }
  *value_ = *I::GetRoot(GetIsolate(), root_index);
}

template<typename T>
void ReturnValue<T>::SetNull() {
  TYPE_CHECK(T, Primitive);
  typedef internal::Internals I;
  *value_ = *I::GetRoot(GetIsolate(), I::kNullValueRootIndex);
}

template<typename T>
void ReturnValue<T>::SetUndefined() {
  TYPE_CHECK(T, Primitive);
  typedef internal::Internals I;
  *value_ = *I::GetRoot(GetIsolate(), I::kUndefinedValueRootIndex);
}

template<typename T>
void ReturnValue<T>::SetEmptyString() {
  TYPE_CHECK(T, String);
  typedef internal::Internals I;
  *value_ = *I::GetRoot(GetIsolate(), I::kEmptyStringRootIndex);
}

template <typename T>
Isolate* ReturnValue<T>::GetIsolate() const {
  // Isolate is always the pointer below the default value on the stack.
  return *reinterpret_cast<Isolate**>(&value_[-2]);
}

template <typename T>
Local<Value> ReturnValue<T>::Get() const {
  typedef internal::Internals I;
  if (*value_ == *I::GetRoot(GetIsolate(), I::kTheHoleValueRootIndex))
    return Local<Value>(*Undefined(GetIsolate()));
  return Local<Value>::New(GetIsolate(), reinterpret_cast<Value*>(value_[0]));
}

template <typename T>
template <typename S>
void ReturnValue<T>::Set(S* whatever) {
  // Uncompilable to prevent inadvertent misuse.
  TYPE_CHECK(S*, Primitive);
}

template<typename T>
internal::Object* ReturnValue<T>::GetDefaultValue() {
  // Default value is always the pointer below value_ on the stack.
  return value_[-1];
}

template <typename T>
FunctionCallbackInfo<T>::FunctionCallbackInfo(internal::Object** implicit_args,
                                              internal::Object** values,
                                              int length)
    : implicit_args_(implicit_args), values_(values), length_(length) {}

template<typename T>
Local<Value> FunctionCallbackInfo<T>::operator[](int i) const {
  if (i < 0 || length_ <= i) return Local<Value>(*Undefined(GetIsolate()));
  return Local<Value>(reinterpret_cast<Value*>(values_[i]));
}


template<typename T>
Local<Function> FunctionCallbackInfo<T>::Callee() const {
  return Local<Function>(reinterpret_cast<Function*>(
      implicit_args_[kCalleeIndex]));
}


template<typename T>
Local<Object> FunctionCallbackInfo<T>::This() const {
  return Local<Object>(reinterpret_cast<Object*>(implicit_args_[kNewTargetIndex+1]));
}


template<typename T>
Local<Object> FunctionCallbackInfo<T>::Holder() const {
  return Local<Object>(reinterpret_cast<Object*>(
      implicit_args_[kHolderIndex]));
}

template <typename T>
Local<Value> FunctionCallbackInfo<T>::NewTarget() const {
  return Local<Value>(
      reinterpret_cast<Value*>(implicit_args_[kNewTargetIndex]));
}

template <typename T>
Local<Value> FunctionCallbackInfo<T>::Data() const {
  return Local<Value>(reinterpret_cast<Value*>(implicit_args_[kDataIndex]));
}


template<typename T>
Isolate* FunctionCallbackInfo<T>::GetIsolate() const {
  return reinterpret_cast<Isolate*>(implicit_args_[kIsolateIndex]);
}


template<typename T>
ReturnValue<T> FunctionCallbackInfo<T>::GetReturnValue() const {
  return ReturnValue<T>(&implicit_args_[kReturnValueIndex]);
}


template<typename T>
bool FunctionCallbackInfo<T>::IsConstructCall() const {
  return !NewTarget()->IsUndefined();
}


template<typename T>
int FunctionCallbackInfo<T>::Length() const {
  return length_;
}

ScriptOrigin::ScriptOrigin(Local<Value> resource_name,
                           Local<Integer> resource_line_offset,
                           Local<Integer> resource_column_offset,
                           Local<Boolean> resource_is_shared_cross_origin,
                           Local<Integer> script_id,
                           Local<Value> source_map_url,
                           Local<Boolean> resource_is_opaque,
                           Local<Boolean> is_wasm, Local<Boolean> is_module)
    : resource_name_(resource_name),
      resource_line_offset_(resource_line_offset),
      resource_column_offset_(resource_column_offset),
      options_(!resource_is_shared_cross_origin.IsEmpty() &&
                   resource_is_shared_cross_origin->IsTrue(),
               !resource_is_opaque.IsEmpty() && resource_is_opaque->IsTrue(),
               !is_wasm.IsEmpty() && is_wasm->IsTrue(),
               !is_module.IsEmpty() && is_module->IsTrue()),
      script_id_(script_id),
      source_map_url_(source_map_url) {}

Local<Value> ScriptOrigin::ResourceName() const { return resource_name_; }


Local<Integer> ScriptOrigin::ResourceLineOffset() const {
  return resource_line_offset_;
}


Local<Integer> ScriptOrigin::ResourceColumnOffset() const {
  return resource_column_offset_;
}


Local<Integer> ScriptOrigin::ScriptID() const { return script_id_; }


Local<Value> ScriptOrigin::SourceMapUrl() const { return source_map_url_; }


ScriptCompiler::Source::Source(Local<String> string, const ScriptOrigin& origin,
                               CachedData* data)
    : source_string(string),
      resource_name(origin.ResourceName()),
      resource_line_offset(origin.ResourceLineOffset()),
      resource_column_offset(origin.ResourceColumnOffset()),
      resource_options(origin.Options()),
      source_map_url(origin.SourceMapUrl()),
      cached_data(data) {}


ScriptCompiler::Source::Source(Local<String> string,
                               CachedData* data)
    : source_string(string), cached_data(data) {}


ScriptCompiler::Source::~Source() {
  delete cached_data;
}


const ScriptCompiler::CachedData* ScriptCompiler::Source::GetCachedData()
    const {
  return cached_data;
}

const ScriptOriginOptions& ScriptCompiler::Source::GetResourceOptions() const {
  return resource_options;
}

Local<Boolean> Boolean::New(Isolate* isolate, bool value) {
  return value ? True(isolate) : False(isolate);
}

void Template::Set(Isolate* isolate, const char* name, Local<Data> value) {
  Set(String::NewFromUtf8(isolate, name, NewStringType::kInternalized)
          .ToLocalChecked(),
      value);
}


Local<Value> Object::GetInternalField(int index) {
  return SlowGetInternalField(index);
}


void* Object::GetAlignedPointerFromInternalField(int index) {
  return SlowGetAlignedPointerFromInternalField(index);
}

String* String::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<String*>(value);
}


Local<String> String::Empty(Isolate* isolate) {
  typedef internal::Object* S;
  typedef internal::Internals I;
  I::CheckInitialized(isolate);
  S* slot = I::GetRoot(isolate, I::kEmptyStringRootIndex);
  return Local<String>(reinterpret_cast<String*>(*slot));
}


String::ExternalStringResource* String::GetExternalStringResource() const {
  return nullptr;
}


String::ExternalStringResourceBase* String::GetExternalStringResourceBase(
    String::Encoding* encoding_out) const {
  return nullptr;
}


bool Value::IsUndefined() const {
  return FullIsUndefined();
}

bool Value::QuickIsUndefined() const {
  return FullIsUndefined();
}


bool Value::IsNull() const {
  return FullIsNull();
}

bool Value::QuickIsNull() const {
  return FullIsNull();
}

bool Value::IsNullOrUndefined() const {
  return FullIsNull() || FullIsUndefined();
}

bool Value::QuickIsNullOrUndefined() const {
  return FullIsNull() || FullIsUndefined();
}

bool Value::IsString() const {
  return FullIsString();
}

bool Value::QuickIsString() const {
  return FullIsString();
}


template <class T> Value* Value::Cast(T* value) {
  return static_cast<Value*>(value);
}


Local<Boolean> Value::ToBoolean() const {
  return ToBoolean(Isolate::GetCurrent()->GetCurrentContext())
      .FromMaybe(Local<Boolean>());
}


Local<Number> Value::ToNumber() const {
  return ToNumber(Isolate::GetCurrent()->GetCurrentContext())
      .FromMaybe(Local<Number>());
}


Local<String> Value::ToString() const {
  return ToString(Isolate::GetCurrent()->GetCurrentContext())
      .FromMaybe(Local<String>());
}


Local<String> Value::ToDetailString() const {
  return ToDetailString(Isolate::GetCurrent()->GetCurrentContext())
      .FromMaybe(Local<String>());
}


Local<Object> Value::ToObject() const {
  return ToObject(Isolate::GetCurrent()->GetCurrentContext())
      .FromMaybe(Local<Object>());
}


Local<Integer> Value::ToInteger() const {
  return ToInteger(Isolate::GetCurrent()->GetCurrentContext())
      .FromMaybe(Local<Integer>());
}


Local<Uint32> Value::ToUint32() const {
  return ToUint32(Isolate::GetCurrent()->GetCurrentContext())
      .FromMaybe(Local<Uint32>());
}


Local<Int32> Value::ToInt32() const {
  return ToInt32(Isolate::GetCurrent()->GetCurrentContext())
      .FromMaybe(Local<Int32>());
}


Boolean* Boolean::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Boolean*>(value);
}


Name* Name::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Name*>(value);
}


Symbol* Symbol::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Symbol*>(value);
}


Number* Number::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Number*>(value);
}


Integer* Integer::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Integer*>(value);
}


Int32* Int32::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Int32*>(value);
}


Uint32* Uint32::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Uint32*>(value);
}


Date* Date::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Date*>(value);
}


StringObject* StringObject::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<StringObject*>(value);
}


SymbolObject* SymbolObject::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<SymbolObject*>(value);
}


NumberObject* NumberObject::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<NumberObject*>(value);
}


BooleanObject* BooleanObject::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<BooleanObject*>(value);
}


RegExp* RegExp::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<RegExp*>(value);
}


Object* Object::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Object*>(value);
}


Array* Array::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Array*>(value);
}


Map* Map::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Map*>(value);
}


Set* Set::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Set*>(value);
}


Promise* Promise::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Promise*>(value);
}


Proxy* Proxy::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Proxy*>(value);
}

WasmCompiledModule* WasmCompiledModule::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<WasmCompiledModule*>(value);
}

Promise::Resolver* Promise::Resolver::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Promise::Resolver*>(value);
}


ArrayBuffer* ArrayBuffer::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<ArrayBuffer*>(value);
}


ArrayBufferView* ArrayBufferView::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<ArrayBufferView*>(value);
}


TypedArray* TypedArray::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<TypedArray*>(value);
}


Uint8Array* Uint8Array::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Uint8Array*>(value);
}


Int8Array* Int8Array::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Int8Array*>(value);
}


Uint16Array* Uint16Array::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Uint16Array*>(value);
}


Int16Array* Int16Array::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Int16Array*>(value);
}


Uint32Array* Uint32Array::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Uint32Array*>(value);
}


Int32Array* Int32Array::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Int32Array*>(value);
}


Float32Array* Float32Array::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Float32Array*>(value);
}


Float64Array* Float64Array::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Float64Array*>(value);
}


Uint8ClampedArray* Uint8ClampedArray::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Uint8ClampedArray*>(value);
}


DataView* DataView::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<DataView*>(value);
}


SharedArrayBuffer* SharedArrayBuffer::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<SharedArrayBuffer*>(value);
}


Function* Function::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<Function*>(value);
}


External* External::Cast(v8::Value* value) {
#ifdef V8_ENABLE_CHECKS
  CheckCast(value);
#endif
  return static_cast<External*>(value);
}


template<typename T>
Isolate* PropertyCallbackInfo<T>::GetIsolate() const {
  return reinterpret_cast<Isolate*>(args_[kIsolateIndex]);
}


template<typename T>
Local<Value> PropertyCallbackInfo<T>::Data() const {
  return Local<Value>(reinterpret_cast<Value*>(args_[kDataIndex]));
}


template<typename T>
Local<Object> PropertyCallbackInfo<T>::This() const {
  return Local<Object>(reinterpret_cast<Object*>(args_[kThisIndex]));
}


template<typename T>
Local<Object> PropertyCallbackInfo<T>::Holder() const {
  return Local<Object>(reinterpret_cast<Object*>(args_[kHolderIndex]));
}


template<typename T>
ReturnValue<T> PropertyCallbackInfo<T>::GetReturnValue() const {
  return ReturnValue<T>(&args_[kReturnValueIndex]);
}

template <typename T>
bool PropertyCallbackInfo<T>::ShouldThrowOnError() const {
  return args_[kShouldThrowOnErrorIndex] != nullptr;
}


Local<Primitive> Undefined(Isolate* isolate) {
  typedef internal::Object* S;
  typedef internal::Internals I;
  I::CheckInitialized(isolate);
  S* slot = I::GetRoot(isolate, I::kUndefinedValueRootIndex);
  return Local<Primitive>(reinterpret_cast<Primitive*>(*slot));
}


Local<Primitive> Null(Isolate* isolate) {
  typedef internal::Object* S;
  typedef internal::Internals I;
  I::CheckInitialized(isolate);
  S* slot = I::GetRoot(isolate, I::kNullValueRootIndex);
  return Local<Primitive>(reinterpret_cast<Primitive*>(*slot));
}


Local<Boolean> True(Isolate* isolate) {
  typedef internal::Object* S;
  typedef internal::Internals I;
  I::CheckInitialized(isolate);
  S* slot = I::GetRoot(isolate, I::kTrueValueRootIndex);
  return Local<Boolean>(reinterpret_cast<Boolean*>(*slot));
}


Local<Boolean> False(Isolate* isolate) {
  typedef internal::Object* S;
  typedef internal::Internals I;
  I::CheckInitialized(isolate);
  S* slot = I::GetRoot(isolate, I::kFalseValueRootIndex);
  return Local<Boolean>(reinterpret_cast<Boolean*>(*slot));
}


void Isolate::SetData(uint32_t slot, void* data) {
  typedef internal::Internals I;
  I::SetEmbedderData(this, slot, data);
}


void* Isolate::GetData(uint32_t slot) {
  typedef internal::Internals I;
  return I::GetEmbedderData(this, slot);
}


uint32_t Isolate::GetNumberOfDataSlots() {
  typedef internal::Internals I;
  return I::kNumIsolateDataSlots;
}


int64_t Isolate::AdjustAmountOfExternalAllocatedMemory(int64_t change_in_bytes) {
  return 0;
}

Local<Value> Context::GetEmbedderData(int index) {
  return SlowGetEmbedderData(index);
}


void* Context::GetAlignedPointerFromEmbedderData(int index) {
  return SlowGetAlignedPointerFromEmbedderData(index);
}


void V8::SetAllowCodeGenerationFromStringsCallback(
    DeprecatedAllowCodeGenerationFromStringsCallback callback) {
  Isolate* isolate = Isolate::GetCurrent();
  isolate->SetAllowCodeGenerationFromStringsCallback(
      reinterpret_cast<FreshNewAllowCodeGenerationFromStringsCallback>(
          callback));
}


bool V8::IsDead() {
  Isolate* isolate = Isolate::GetCurrent();
  return isolate->IsDead();
}


bool V8::AddMessageListener(MessageCallback that, Local<Value> data) {
  Isolate* isolate = Isolate::GetCurrent();
  return isolate->AddMessageListener(that, data);
}


void V8::RemoveMessageListeners(MessageCallback that) {
  Isolate* isolate = Isolate::GetCurrent();
  isolate->RemoveMessageListeners(that);
}


void V8::SetFailedAccessCheckCallbackFunction(
    FailedAccessCheckCallback callback) {
  Isolate* isolate = Isolate::GetCurrent();
  isolate->SetFailedAccessCheckCallbackFunction(callback);
}


void V8::SetCaptureStackTraceForUncaughtExceptions(
    bool capture, int frame_limit, StackTrace::StackTraceOptions options) {
  Isolate* isolate = Isolate::GetCurrent();
  isolate->SetCaptureStackTraceForUncaughtExceptions(capture, frame_limit,
                                                     options);
}


void V8::SetFatalErrorHandler(FatalErrorCallback callback) {
  Isolate* isolate = Isolate::GetCurrent();
  isolate->SetFatalErrorHandler(callback);
}

void V8::RemoveGCPrologueCallback(GCCallback callback) {
  Isolate* isolate = Isolate::GetCurrent();
  isolate->RemoveGCPrologueCallback(
      reinterpret_cast<Isolate::GCCallback>(callback));
}


void V8::RemoveGCEpilogueCallback(GCCallback callback) {
  Isolate* isolate = Isolate::GetCurrent();
  isolate->RemoveGCEpilogueCallback(
      reinterpret_cast<Isolate::GCCallback>(callback));
}

void V8::TerminateExecution(Isolate* isolate) { isolate->TerminateExecution(); }


bool V8::IsExecutionTerminating(Isolate* isolate) {
  if (isolate == NULL) {
    isolate = Isolate::GetCurrent();
  }
  return isolate->IsExecutionTerminating();
}


void V8::CancelTerminateExecution(Isolate* isolate) {
  isolate->CancelTerminateExecution();
}


void V8::VisitExternalResources(ExternalResourceVisitor* visitor) {
  Isolate* isolate = Isolate::GetCurrent();
  isolate->VisitExternalResources(visitor);
}


void V8::VisitHandlesWithClassIds(PersistentHandleVisitor* visitor) {
  Isolate* isolate = Isolate::GetCurrent();
  isolate->VisitHandlesWithClassIds(visitor);
}


void V8::VisitHandlesWithClassIds(Isolate* isolate,
                                  PersistentHandleVisitor* visitor) {
  isolate->VisitHandlesWithClassIds(visitor);
}


void V8::VisitHandlesForPartialDependence(Isolate* isolate,
                                          PersistentHandleVisitor* visitor) {
  isolate->VisitHandlesForPartialDependence(visitor);
}

#endif
