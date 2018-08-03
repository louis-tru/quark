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

#include "libplatform/v8-tracing.h"
#include "util.h"
#include <thread>

namespace v8 {
namespace base {
class Mutex {
	Mutex() { }
private:
	std::mutex m_mutex;
};
}  // namespace base

namespace platform {
namespace tracing {

TraceObject::~TraceObject() {}

void TraceObject::Initialize(
		char phase, const uint8_t* category_enabled_flag, const char* name,
		const char* scope, uint64_t id, uint64_t bind_id, int num_args,
		const char** arg_names, const uint8_t* arg_types,
		const uint64_t* arg_values,
		std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
		unsigned int flags) {
	UNIMPLEMENTED();
}

void TraceObject::UpdateDuration() {}

void TraceObject::InitializeForTesting(
		char phase, const uint8_t* category_enabled_flag, const char* name,
		const char* scope, uint64_t id, uint64_t bind_id, int num_args,
		const char** arg_names, const uint8_t* arg_types,
		const uint64_t* arg_values,
		std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
		unsigned int flags, int pid, int tid, int64_t ts, int64_t tts,
		uint64_t duration, uint64_t cpu_duration) {
	UNIMPLEMENTED();
}

TraceWriter* TraceWriter::CreateJSONTraceWriter(std::ostream& stream) {
	return nullptr;
}

TraceBufferChunk::TraceBufferChunk(uint32_t seq) : seq_(seq) {}

void TraceBufferChunk::Reset(uint32_t new_seq) {}

TraceObject* TraceBufferChunk::AddTraceEvent(size_t* event_index) {
	return nullptr;
}

TraceBuffer* TraceBuffer::CreateTraceBufferRingBuffer(
		size_t max_chunks, TraceWriter* trace_writer) {
	return nullptr;
}

TraceConfig* TraceConfig::CreateDefaultTraceConfig() {
	return nullptr;
}

void TraceConfig::AddIncludedCategory(const char* included_category) { }

bool TraceConfig::IsCategoryGroupEnabled(const char* category_group) const {
	return false;
}

TracingController::TracingController() {}

TracingController::~TracingController() {}

void TracingController::Initialize(TraceBuffer* trace_buffer) {
	UNIMPLEMENTED();
}

const uint8_t* TracingController::GetCategoryGroupEnabled(const char* category_group) {
	return nullptr;
}

uint64_t TracingController::AddTraceEvent(
	char phase, const uint8_t* category_enabled_flag, const char* name,
	const char* scope, uint64_t id, uint64_t bind_id, int num_args,
	const char** arg_names, const uint8_t* arg_types,
	const uint64_t* arg_values,
	std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
	unsigned int flags) {
	return 0;
}

void TracingController::UpdateTraceEventDuration(
	const uint8_t* category_enabled_flag, const char* name, uint64_t handle) {
}

const char* TracingController::GetCategoryGroupName(const uint8_t* category_group_enabled) {
	return nullptr;
}

void TracingController::StartTracing(TraceConfig* trace_config) { }

void TracingController::StopTracing() { }

void TracingController::UpdateCategoryGroupEnabledFlag(size_t category_index) { }

void TracingController::UpdateCategoryGroupEnabledFlags() { }

const uint8_t* TracingController::GetCategoryGroupEnabledInternal(const char* category_group) {
	return nullptr;
}

void TracingController::AddTraceStateObserver(
	v8::TracingController::TraceStateObserver* observer) {
}

void TracingController::RemoveTraceStateObserver(
	v8::TracingController::TraceStateObserver* observer) {
}

}
}
}
