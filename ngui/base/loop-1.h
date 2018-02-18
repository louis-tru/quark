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

#ifndef __ngui__base__loop_1__
#define __ngui__base__loop_1__

#include "loop.h"

/**
 * @ns ngui
 */

XX_NS(ngui)

/**
 * @class ParallelWorking
 */
class ParallelWorking: public Object {
  XX_HIDDEN_ALL_COPY(ParallelWorking);
 public:
  typedef SimpleThread::Exec Exec;
  ParallelWorking();
  ParallelWorking(RunLoop* loop);
  virtual ~ParallelWorking();
  ThreadID detach_child(Exec exec, cString& name);
  void abort_child(ThreadID id = ThreadID());   // default abort all child
  void awaken_child(ThreadID id = ThreadID());  // default awaken all child
  uint post(cCb& cb); // post message to main thread
  uint post(cCb& cb, uint64 delay_us);
  void abort(uint id = 0);
  inline Mutex& mutex() { return m_mutex; }
 private:
  KeepLoop*  m_proxy;
  Mutex m_mutex;
  uint  m_gid;
};

XX_DEFINE_INLINE_MEMBERS(RunLoop, Inl2) {
 public:
  inline void set_independent_mutex(RecursiveMutex* mutex) {
    m_independent_mutex = mutex;
  }
  inline void independent_mutex_lock() {
    if (m_independent_mutex) {
      m_independent_mutex->lock();
    }
  }
  inline void independent_mutex_unlock() {
    if (m_independent_mutex) {
      m_independent_mutex->unlock();
    }
  }
};

XX_END
#endif
