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

#include "v8-profiler.h"

namespace v8 {
	
	std::unique_ptr<TracingCpuProfiler> TracingCpuProfiler::Create(Isolate* iso) {
		return std::unique_ptr<TracingCpuProfiler>();
	}

	void TickSample::Init(Isolate* isolate, const v8::RegisterState& state,
						RecordCEntryFrame record_c_entry_frame, bool update_stats,
						bool use_simulator_reg_state) {
	}

	bool TickSample::GetStackSample(Isolate* isolate, v8::RegisterState* state,
														 RecordCEntryFrame record_c_entry_frame,
														 void** frames, size_t frames_limit,
														 v8::SampleInfo* sample_info,
														 bool use_simulator_reg_state) {
		return false;
	}

	Local<String> CpuProfileNode::GetFunctionName() const {
		return Local<String>();
	}

	const char* CpuProfileNode::GetFunctionNameStr() const {
		return nullptr;
	}

	int CpuProfileNode::GetScriptId() const {
		return 0;
	}

	Local<String> CpuProfileNode::GetScriptResourceName() const {
		return Local<String>();
	}

	const char* CpuProfileNode::GetScriptResourceNameStr() const {
		return nullptr;
	}

	int CpuProfileNode::GetLineNumber() const {
		return 0;
	}

	int CpuProfileNode::GetColumnNumber() const {
		return 0;
	}

	unsigned int CpuProfileNode::GetHitLineCount() const {
		return 0;
	}

	bool CpuProfileNode::GetLineTicks(LineTick* entries, unsigned int length) const {
		return false;
	}

	const char* CpuProfileNode::GetBailoutReason() const {
		return nullptr;
	}

	unsigned CpuProfileNode::GetHitCount() const {
		return 0;
	}

	unsigned CpuProfileNode::GetCallUid() const {
		return 0;
	}

	unsigned CpuProfileNode::GetNodeId() const {
		return 0;
	}

	int CpuProfileNode::GetChildrenCount() const {
		return 0;
	}

	const CpuProfileNode* CpuProfileNode::GetChild(int index) const {
		return 0;
	}

	const std::vector<CpuProfileDeoptInfo>& CpuProfileNode::GetDeoptInfos() const {
		static std::vector<CpuProfileDeoptInfo> vec; return vec;
	}
	
	Local<String> CpuProfile::GetTitle() const {
		return Local<String>();
	}

	const CpuProfileNode* CpuProfile::GetTopDownRoot() const {
		return nullptr;
	}

	int CpuProfile::GetSamplesCount() const { 
		return 0;
	}

	const CpuProfileNode* CpuProfile::GetSample(int index) const {
		return nullptr;
	}

	int64_t CpuProfile::GetSampleTimestamp(int index) const {
		return 0;
	}

	int64_t CpuProfile::GetStartTime() const {
		return 0;
	}

	int64_t CpuProfile::GetEndTime() const {
		return 0;
	}

	void CpuProfile::Delete() {}

	CpuProfiler* CpuProfiler::New(Isolate* isolate) {
		return nullptr;
	}

	void CpuProfiler::Dispose() {}

	void CpuProfiler::SetSamplingInterval(int us) {}

	void CpuProfiler::StartProfiling(Local<String> title, bool record_samples) {}

	CpuProfile* CpuProfiler::StopProfiling(Local<String> title) {
		return nullptr;
	}

	void CpuProfiler::CollectSample() {}

	void CpuProfiler::SetIdle(bool is_idle) {}

	HeapGraphEdge::Type HeapGraphEdge::GetType() const {
		return kContextVariable;
	}

	Local<Value> HeapGraphEdge::GetName() const {
		return Local<Value>();
	}

	const HeapGraphNode* HeapGraphEdge::GetFromNode() const {
		return nullptr;
	}

	const HeapGraphNode* HeapGraphEdge::GetToNode() const {
		return nullptr;
	}

	HeapGraphNode::Type HeapGraphNode::GetType() const {
		return kHidden;
	}

	Local<String> HeapGraphNode::GetName() const {
		return Local<String>();
	}

	SnapshotObjectId HeapGraphNode::GetId() const {
		return 0;
	}

	size_t HeapGraphNode::GetShallowSize() const {
		return 0;
	}

	int HeapGraphNode::GetChildrenCount() const {
		return 0;
	}

	const HeapGraphEdge* HeapGraphNode::GetChild(int index) const {
		return nullptr;
	}

	const HeapGraphNode* HeapSnapshot::GetRoot() const {
		return nullptr;
	}

	const HeapGraphNode* HeapSnapshot::GetNodeById(SnapshotObjectId id) const {
		return nullptr;
	}

	int HeapSnapshot::GetNodesCount() const {
		return 0;
	}

	const HeapGraphNode* HeapSnapshot::GetNode(int index) const {
		return nullptr;
	}

	SnapshotObjectId HeapSnapshot::GetMaxSnapshotJSObjectId() const {
		return 0;
	}

	void HeapSnapshot::Delete() {}

	void HeapSnapshot::Serialize(OutputStream* stream,
								 SerializationFormat format) const {}

	int HeapProfiler::GetSnapshotCount() {
		return 0;
	}

	const HeapSnapshot* HeapProfiler::GetHeapSnapshot(int index) {
		return nullptr;
	}

	SnapshotObjectId HeapProfiler::GetObjectId(Local<Value> value) {
		return 0;
	}

	Local<Value> HeapProfiler::FindObjectById(SnapshotObjectId id) {
		return Local<Value>();
	}

	void HeapProfiler::ClearObjectIds() {}

	/**
	 * Takes a heap snapshot and returns it.
	 */
	const HeapSnapshot* HeapProfiler::TakeHeapSnapshot(
			ActivityControl* control,
			ObjectNameResolver* global_object_name_resolver) {
		return nullptr;
	}

	void HeapProfiler::StartTrackingHeapObjects(bool track_allocations) {}

	SnapshotObjectId HeapProfiler::GetHeapStats(OutputStream* stream,
																int64_t* timestamp_us) {
		return 0;
	}

	void HeapProfiler::StopTrackingHeapObjects() {}

	bool HeapProfiler::StartSamplingHeapProfiler(uint64_t sample_interval,
																 int stack_depth,
																 SamplingFlags flags) {
		return false;
	}

	void HeapProfiler::StopSamplingHeapProfiler() {}

	AllocationProfile* HeapProfiler::GetAllocationProfile() {
		return nullptr;
	}

	void HeapProfiler::DeleteAllHeapSnapshots() {}

	void HeapProfiler::SetWrapperClassInfoProvider(
			uint16_t class_id,
			WrapperInfoCallback callback) {}

	void HeapProfiler::SetGetRetainerInfosCallback(GetRetainerInfosCallback callback) {}

	size_t HeapProfiler::GetProfilerMemorySize() {
		return 0;
	}

}

