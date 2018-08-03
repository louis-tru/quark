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


// --- M o d u l e ---

Module::Status Module::GetStatus() const { UNIMPLEMENTED(); }
Local<Value> Module::GetException() const { UNIMPLEMENTED(); }
Location Module::GetModuleRequestLocation(int i) const { UNIMPLEMENTED(); }
int Module::GetModuleRequestsLength() const { UNIMPLEMENTED(); }
Local<String> Module::GetModuleRequest(int i) const { UNIMPLEMENTED(); }
int Module::GetIdentityHash() const { UNIMPLEMENTED(); }
bool Module::Instantiate(Local<Context> context,
	Module::ResolveCallback callback) { UNIMPLEMENTED(); }
Maybe<bool> Module::InstantiateModule(Local<Context> context,
	Module::ResolveCallback callback) { UNIMPLEMENTED(); }
MaybeLocal<Value> Module::Evaluate(Local<Context> context) { UNIMPLEMENTED(); }
Local<Value> Module::GetModuleNamespace() { UNIMPLEMENTED(); }

// --- W a s m   M o d u l e ---

Local<String> WasmCompiledModule::GetWasmWireBytes() {
	UNIMPLEMENTED();
}
WasmCompiledModule::TransferrableModule&
WasmCompiledModule::TransferrableModule::operator=(TransferrableModule&& src) {
	UNIMPLEMENTED();
}
WasmCompiledModule::TransferrableModule
WasmCompiledModule::GetTransferrableModule() {
	UNIMPLEMENTED();
}
MaybeLocal<WasmCompiledModule>
WasmCompiledModule::FromTransferrableModule(
	Isolate* isolate,
	const WasmCompiledModule::TransferrableModule& transferrable_module) {
	UNIMPLEMENTED();
}
WasmCompiledModule::SerializedModule WasmCompiledModule::Serialize() {
	UNIMPLEMENTED();
}
MaybeLocal<WasmCompiledModule> WasmCompiledModule::Deserialize(
	Isolate* isolate,
	const WasmCompiledModule::CallerOwnedBuffer& serialized_module,
	const WasmCompiledModule::CallerOwnedBuffer& wire_bytes) {
	UNIMPLEMENTED();
}
MaybeLocal<WasmCompiledModule> WasmCompiledModule::DeserializeOrCompile(
	Isolate* isolate,
	const WasmCompiledModule::CallerOwnedBuffer& serialized_module,
	const WasmCompiledModule::CallerOwnedBuffer& wire_bytes) {
	UNIMPLEMENTED();
}
MaybeLocal<WasmCompiledModule> WasmCompiledModule::Compile(
	Isolate* isolate, const uint8_t* start, size_t length) {
	UNIMPLEMENTED();
}
void WasmModuleObjectBuilder::OnBytesReceived(const uint8_t* bytes, size_t size) { }
MaybeLocal<WasmCompiledModule> WasmModuleObjectBuilder::Finish() { UNIMPLEMENTED(); }
WasmModuleObjectBuilder&
WasmModuleObjectBuilder::operator=(WasmModuleObjectBuilder&& src) { UNIMPLEMENTED(); }

// --- L o c k e r ---

void Locker::Initialize(v8::Isolate* isolate) {
	DCHECK(isolate);
	has_lock_ = true;
	top_level_ = true;
	isolate_ = reinterpret_cast<i::Isolate*>(isolate);
}
bool Locker::IsLocked(v8::Isolate* isolate) { return true; }
bool Locker::IsActive() { return true; }
Locker::~Locker() {}
void Unlocker::Initialize(v8::Isolate* isolate) {
	isolate_ = reinterpret_cast<i::Isolate*>(isolate);
}
Unlocker::~Unlocker() {}

// --- S n a p s h o t   C r e a t o r ---

void V8::SetNativesDataBlob(StartupData* natives_blob) {}
void V8::SetSnapshotDataBlob(StartupData* snapshot_blob) {}
SnapshotCreator::SnapshotCreator(intptr_t* external_references,
	StartupData* existing_snapshot) { UNIMPLEMENTED(); }
SnapshotCreator::~SnapshotCreator() {}
Isolate* SnapshotCreator::GetIsolate() { return nullptr; }
void SnapshotCreator::SetDefaultContext(Local<Context> context) {}
size_t SnapshotCreator::AddContext(Local<Context> context,
	SerializeInternalFieldsCallback callback) { return 0; }
size_t SnapshotCreator::AddTemplate(Local<Template> template_obj) { return 0; }
StartupData SnapshotCreator::CreateBlob(SnapshotCreator::FunctionCodeHandling
	function_code_handling) { return StartupData(); }
StartupData V8::CreateSnapshotDataBlob(const char* embedded_source) { return StartupData(); }
StartupData V8::WarmUpSnapshotDataBlob(StartupData cold_snapshot_blob,
	const char* warmup_source) { return StartupData(); }
void V8::SetFlagsFromString(const char* str, int length) {}
void V8::SetFlagsFromCommandLine(int* argc, char** argv, bool remove_flags) {}
Extension::Extension(
	const char* name, const char* source,
	int dep_count, const char** deps, int source_length)
: name_(name)
, source_length_(source_length >= 0 ? source_length :
								 (source ? static_cast<int>(strlen(source)) : 0))
, source_(source, source_length_)
, dep_count_(dep_count)
, deps_(deps)
, auto_enable_(false) {
	CHECK(source != NULL || source_length_ == 0);
}

// --- R e s o u r c e   C o n s t r a i n t s ---

ResourceConstraints::ResourceConstraints()
: max_semi_space_size_(0)
, max_old_space_size_(0)
, stack_limit_(NULL)
, code_range_size_(0)
, max_zone_pool_size_(0) {
}
void ResourceConstraints::ConfigureDefaults(
	uint64_t physical_memory,
	uint64_t virtual_memory_limit) {
	memset(this, 0, sizeof(ResourceConstraints));
}

// --- N a t i v e W e a k M a p ---

Local<NativeWeakMap> NativeWeakMap::New(Isolate* v8_isolate) { UNIMPLEMENTED(); }
void NativeWeakMap::Set(Local<Value> v8_key, Local<Value> v8_value) {}
Local<Value> NativeWeakMap::Get(Local<Value> v8_key) const { return Local<Value>(); }
bool NativeWeakMap::Has(Local<Value> v8_key) { return false; }
bool NativeWeakMap::Delete(Local<Value> v8_key) { return false; }

// --- E n v i r o n m e n t ---

HeapStatistics::HeapStatistics()
: total_heap_size_(0)
, total_heap_size_executable_(0)
, total_physical_size_(0)
, total_available_size_(0)
, used_heap_size_(0)
, heap_size_limit_(0)
, malloced_memory_(0)
, peak_malloced_memory_(0)
, does_zap_garbage_(0) {
}
HeapSpaceStatistics::HeapSpaceStatistics()
: space_name_(0)
, space_size_(0)
, space_used_size_(0)
, space_available_size_(0)
, physical_space_size_(0) {

}
HeapObjectStatistics::HeapObjectStatistics()
: object_type_(nullptr)
, object_sub_type_(nullptr)
, object_count_(0)
, object_size_(0) {
}
HeapCodeStatistics::HeapCodeStatistics()
: code_and_metadata_size_(0), bytecode_and_metadata_size_(0) {
}

// --- Microtasks Scope ---

MicrotasksScope::MicrotasksScope(Isolate* isolate, MicrotasksScope::Type type)
: isolate_(reinterpret_cast<i::Isolate*>(isolate)),
run_(type == MicrotasksScope::kRunMicrotasks) {}
MicrotasksScope::~MicrotasksScope() {}
void MicrotasksScope::PerformCheckpoint(Isolate* v8Isolate) {}
int MicrotasksScope::GetCurrentDepth(Isolate* v8Isolate) { return 0; }
bool MicrotasksScope::IsRunningMicrotasks(Isolate* v8Isolate) { return false; }

// --- C o n t e x t ---

void v8::Context::SetSecurityToken(Local<Value> token) {}
void v8::Context::UseDefaultSecurityToken() {}
Local<Value> v8::Context::GetSecurityToken() { return Local<Value>(); }
void Context::DetachGlobal() {}
Local<v8::Object> Context::GetExtrasBindingObject() { return Local<Object>(); }
void Context::AllowCodeGenerationFromStrings(bool allow) {}
bool Context::IsCodeGenerationFromStringsAllowed() {return false;}
void Context::SetErrorMessageForCodeGenerationFromStrings(Local<String> error) {}
size_t Context::EstimatedSize() { return 0; }
MaybeLocal<Context> v8::Context::FromSnapshot(
v8::Isolate* external_isolate, size_t context_snapshot_index,
v8::DeserializeInternalFieldsCallback embedder_fields_deserializer,
v8::ExtensionConfiguration* extensions, MaybeLocal<Value> global_object) {
	UNIMPLEMENTED();
}
MaybeLocal<Object> v8::Context::NewRemoteContext(
	v8::Isolate* external_isolate,
	v8::Local<ObjectTemplate> global_template,
	v8::MaybeLocal<v8::Value> global_object) {
	UNIMPLEMENTED();
}

// --- I s o l a t e ---

void Isolate::SetHostImportModuleDynamicallyCallback(
	HostImportModuleDynamicallyCallback callback) { UNIMPLEMENTED(); }
Local<Context> Isolate::GetIncumbentContext() { UNIMPLEMENTED(); }
v8::Local<v8::Context> Isolate::GetCallingContext() { return Local<Context>(); }
v8::Local<v8::Context> Isolate::GetEnteredContext() { return GetCurrentContext(); }
v8::Local<v8::Context> Isolate::GetEnteredOrMicrotaskContext() { return GetCurrentContext(); }
void Isolate::AddGCPrologueCallback(GCCallbackWithData callback, void* data,
																		GCType gc_type_filter) {}
void Isolate::AddGCPrologueCallback(GCCallback callback, GCType gc_type) {}
void Isolate::RemoveGCPrologueCallback(GCCallbackWithData, void* data) {}
void Isolate::RemoveGCPrologueCallback(GCCallback callback) {}
void Isolate::AddGCEpilogueCallback(GCCallbackWithData callback, void* data,
																		GCType gc_type_filter){}
void Isolate::AddGCEpilogueCallback(GCCallback callback, GCType gc_type) {}
void Isolate::RemoveGCEpilogueCallback(GCCallbackWithData callback, void* data) {}
void Isolate::RemoveGCEpilogueCallback(GCCallback callback) {}
void Isolate::SetGetExternallyAllocatedMemoryInBytesCallback(
	GetExternallyAllocatedMemoryInBytesCallback callback) {}
void Isolate::SetEmbedderHeapTracer(EmbedderHeapTracer* tracer) {}
void Isolate::RequestInterrupt(InterruptCallback callback, void* data) {}
void Isolate::RequestGarbageCollectionForTesting(GarbageCollectionType type) {}
void Isolate::DumpAndResetStats() {}
void Isolate::DiscardThreadSpecificMetadata() {}
void Isolate::Enter() {}
void Isolate::Exit() {}
Isolate::DisallowJavascriptExecutionScope::DisallowJavascriptExecutionScope(Isolate* isolate,
	Isolate::DisallowJavascriptExecutionScope::OnFailure on_failure)
: on_failure_(on_failure) {}
Isolate::DisallowJavascriptExecutionScope::~DisallowJavascriptExecutionScope() {}
Isolate::AllowJavascriptExecutionScope::AllowJavascriptExecutionScope(Isolate* isolate) {}
Isolate::AllowJavascriptExecutionScope::~AllowJavascriptExecutionScope() {}
Isolate::SuppressMicrotaskExecutionScope::SuppressMicrotaskExecutionScope(Isolate* isolate)
: isolate_(reinterpret_cast<i::Isolate*>(isolate)) {}
Isolate::SuppressMicrotaskExecutionScope::~SuppressMicrotaskExecutionScope() {}
void Isolate::GetHeapStatistics(HeapStatistics* heap_statistics) {
	memset(heap_statistics, 0, sizeof(HeapStatistics));
}
size_t Isolate::NumberOfHeapSpaces() { return 0; }
bool Isolate::GetHeapSpaceStatistics(HeapSpaceStatistics*
	space_statistics, size_t index) { return false; }
size_t Isolate::NumberOfTrackedHeapObjectTypes() { return 0; }
bool Isolate::GetHeapObjectStatisticsAtLastGC(
	HeapObjectStatistics* object_statistics, size_t type_index) { return false; }
bool Isolate::GetHeapCodeAndMetadataStatistics(
	HeapCodeStatistics* code_statistics) { return false; }
void Isolate::GetStackSample(const RegisterState& state,
	void** frames, size_t frames_limit, SampleInfo* sample_info) {}
size_t Isolate::NumberOfPhantomHandleResetsSinceLastCall() { return 0; }
void Isolate::SetEventLogger(LogEventCallback that) {}
void Isolate::AddBeforeCallEnteredCallback(BeforeCallEnteredCallback callback) {}
void Isolate::RemoveBeforeCallEnteredCallback(BeforeCallEnteredCallback callback) {}
void Isolate::AddCallCompletedCallback(CallCompletedCallback callback) {}
void Isolate::RemoveCallCompletedCallback(CallCompletedCallback callback) {}
void Isolate::AddCallCompletedCallback(DeprecatedCallCompletedCallback callback) {}
void Isolate::RemoveCallCompletedCallback(DeprecatedCallCompletedCallback callback) {}
void Isolate::SetPromiseHook(PromiseHook hook) {}
void Isolate::SetPromiseRejectCallback(PromiseRejectCallback callback) {}
void Isolate::SetUseCounterCallback(UseCounterCallback callback) {}
void Isolate::SetCounterFunction(CounterLookupCallback callback) {}
void Isolate::SetCreateHistogramFunction(CreateHistogramCallback callback) {}
void Isolate::SetAddHistogramSampleFunction(AddHistogramSampleCallback callback) {}
bool Isolate::IdleNotification(int idle_time_in_ms) { return false; }
bool Isolate::IdleNotificationDeadline(double deadline_in_seconds) { return false; }
int Isolate::ContextDisposedNotification(bool dependant_context) {}
void Isolate::IsolateInForegroundNotification() {}
void Isolate::IsolateInBackgroundNotification() {}
void Isolate::MemoryPressureNotification(MemoryPressureLevel level) {}
void Isolate::SetRAILMode(RAILMode rail_mode) {}
void Isolate::IncreaseHeapLimitForDebugging() {}
void Isolate::RestoreOriginalHeapLimit() {}
bool Isolate::IsHeapLimitIncreasedForDebugging() { return false; }
void Isolate::SetJitCodeEventHandler(JitCodeEventOptions options,
	JitCodeEventHandler event_handler) {}
void Isolate::SetStackLimit(uintptr_t stack_limit) {}
void Isolate::GetCodeRange(void** start, size_t* length_in_bytes) {}
void Isolate::SetOOMErrorHandler(OOMErrorCallback that) {}
void Isolate::SetAllowCodeGenerationFromStringsCallback(
	DeprecatedAllowCodeGenerationFromStringsCallback callback) {}
void Isolate::SetAllowCodeGenerationFromStringsCallback(
	FreshNewAllowCodeGenerationFromStringsCallback callback) {}
void Isolate::SetWasmCompileStreamingCallback(ApiImplementationCallback callback) {}
bool Isolate::IsDead() { return false; }
void Isolate::SetFailedAccessCheckCallbackFunction(FailedAccessCheckCallback callback) {}
void Isolate::SetCaptureStackTraceForUncaughtExceptions(
	bool capture, int frame_limit, StackTrace::StackTraceOptions options) {}
void Isolate::VisitExternalResources(ExternalResourceVisitor* visitor) {}
bool Isolate::IsInUse() { return true; }
void Isolate::VisitHandlesWithClassIds(PersistentHandleVisitor* visitor) {}
void Isolate::VisitHandlesForPartialDependence(PersistentHandleVisitor* visitor) {}
void Isolate::VisitWeakHandles(PersistentHandleVisitor* visitor) {}
void Isolate::SetAllowAtomicsWait(bool allow) {}
void Isolate::ReportExternalAllocationLimitReached() {}
bool Isolate::AddMessageListenerWithErrorLevel(
	MessageCallback that, int message_levels, Local<Value> data) { return false; }
void Isolate::RemoveMessageListeners(MessageCallback that) {}

// --- Context::BackupIncumbentScope ---
Context::BackupIncumbentScope::
	BackupIncumbentScope(Local<Context> backup_incumbent_context):prev_(nullptr) {}
Context::BackupIncumbentScope::~BackupIncumbentScope() {}

// --- V 8 ---

void V8::AddGCPrologueCallback(GCCallback callback, GCType gc_type) {}
void V8::AddGCEpilogueCallback(GCCallback callback, GCType gc_type) {}
void V8::InitializePlatform(Platform* platform) {}
void V8::ShutdownPlatform() {}
#if V8_OS_LINUX && V8_TARGET_ARCH_X64 && !V8_OS_ANDROID
bool TryHandleSignal(int signum, void* info, void* context) { return false; }
#endif  // V8_OS_LINUX
bool V8::RegisterDefaultSignalHandler() { return false; }
void V8::SetEntropySource(EntropySource entropy_source) {}
void V8::SetReturnAddressLocationResolver(
	ReturnAddressLocationResolver return_address_resolver) {}
bool V8::InitializeICU(const char* icu_data_file) { return false; }
bool V8::InitializeICUDefaultLocation(const char* exec_path,
																					const char* icu_data_file) { return false; }
void V8::InitializeExternalStartupData(const char* directory_path) {}
void V8::InitializeExternalStartupData(const char* natives_blob,
																					 const char* snapshot_blob) {}
