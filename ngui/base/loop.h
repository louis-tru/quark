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

#ifndef __ngui__loop__
#define __ngui__loop__

#include "util.h"
#include "list.h"
#include "map.h"
#include "event.h"
#include "cb.h"
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

typedef struct uv_loop_s uv_loop_t;
typedef struct uv_async_s uv_async_t;
typedef struct uv_timer_s uv_timer_t;

/**
 * @ns ngui
 */

XX_NS(ngui)

typedef std::thread::id ThreadID;
typedef std::mutex Mutex;
typedef std::recursive_mutex RecursiveMutex;
typedef std::lock_guard<Mutex> ScopeLock;
typedef std::unique_lock<Mutex> Lock;
typedef std::condition_variable Condition;

template<> uint Compare<ThreadID>::hash(const ThreadID &key);
template<> bool Compare<ThreadID>::equals(const ThreadID& a, const ThreadID& b, uint ha, uint hb);

class RunLoop;
class KeepLoop;

/**
 * @class SimpleThread
 */
class XX_EXPORT SimpleThread {
 public:
  typedef NonObjectTraits Traits;
#define XX_THREAD_LOCK(thread) \
ScopeLock thread##_lock(thread.mutex()); if (!t.is_abort())
  typedef std::function<void(SimpleThread& thread)> Exec;
  inline bool is_abort() const { return m_abort; }
  inline Mutex& mutex() { return m_mutex; }
  inline ThreadID id() const { return m_id; }
  inline String name() const { return m_name; }
  static ThreadID detach(Exec exec, cString& name);
  static void sleep_for(uint64 timeUs = 0);
  static ThreadID current_id();
  static SimpleThread* current();
  static void* get_specific_data(char id);
  static void set_specific_data(char id, void* data);
  static void abort(ThreadID id, bool wait_end = false);
  static void wait_end(ThreadID id);
  static void awaken(ThreadID id);
 private:
  XX_DEFINE_INLINE_CLASS(Inl);
  struct Signal {
    Mutex mutex; Condition cond;
  };
  typedef Array<Signal*> WaitList;
  Mutex m_mutex;
  Condition m_cond;
  WaitList m_external_wait;
  bool  m_abort;
  ThreadID  m_id;
  uint  m_gid;
  String  m_name;
};

/**
 * @class PostMessage
 */
class XX_EXPORT PostMessage {
 public:
  virtual uint post_message(cCb& cb, uint64 delay_us = 0) = 0;
};

/**
 * @class RunLoop
 */
class XX_EXPORT RunLoop: public Object, public PostMessage {
  XX_HIDDEN_ALL_COPY(RunLoop);
 public:
  /**
   * @destructor
   */
  virtual ~RunLoop();
  
  /**
   * @func runing()
   */
  bool runing() const;
  
  /**
   * @func is_alive
   */
  bool is_alive() const;
  
  /**
   * @func post(cb[,delay_us]) post message and setting delay
   */
  uint post(cCb& cb, uint64 delay_us = 0);
  
  /**
   * @overwrite
   */
  virtual uint post_message(cCb& cb, uint64 delay_us = 0);
  
  /**
   * @func abort(id) abort message with id
   */
  void abort(uint id);
  
  /**
   * @func run() 运行消息循环
   * @arg [timeout=0] {int64} 超时时间(微妙us),等于0没有新消息立即结束
   */
  void run(uint64 timeout = 0);
  
  /**
   * @func stop() 停止循环
   */
  void stop();
  
  /**
   * @func work(cb[,done])
   */
  uint work(cCb& cb, cCb& done = 0);
  
  /**
   * @func cancel_work(id)
   */
  void cancel_work(uint id);
  
  /**
   * 保持活动状态,并返回一个代理,只要不删除返回的代理对像,消息队列会一直保持活跃状态
   * @func keep_alive(declear)
   * @arg [declear=true] {bool} KeepLoop 释放时是否清理未完成的`post`消息
   */
  KeepLoop* keep_alive(bool declear = true);
  
  /**
   * @func uv_loop()
   */
  inline uv_loop_t* uv_loop() { return m_uv_loop; }
  
  /**
   * @func current() 获取当前线程消息队列
   */
  static RunLoop* current();
  
  /**
   * @func keep_alive_current(declear) 保持当前循环活跃并返回`KeepLoop`实体
   */
  static KeepLoop* keep_alive_current(bool declear = true);
  
  /**
   * @func loop(id) 通过线程获取,目标线程没有创建过实体返回`nullptr`
   * @ret {RunLoop*}
   */
  static RunLoop* loop(ThreadID id);
  
  /**
   * @func next_tick
   */
  static void next_tick(cCb& cb) throw(Error);
  
  /**
   * @func main_loop()
   */
  static RunLoop* main_loop();
  
  /**
   * @func is_main_loop() 当前线程是为主循环
   */
  static bool is_main_loop();
  
  /**
   * @func is_process_exit
   */
  static bool is_process_exit();
  
 private:
  /**
   * @constructor 私有构造每个线程只能创建一个通过`current()`来获取当前实体
   */
  RunLoop();
  
  XX_DEFINE_INLINE_CLASS(Inl);
  XX_DEFINE_INLINE_CLASS(Inl2);
  friend class KeepLoop;
  struct Queue {
    uint id, group;
    int64 time;
    Callback resolve;
  };
  List<Queue> m_queue;
  struct Work;
  List<Work*> m_work;
  Mutex m_mutex;
  RecursiveMutex* m_independent_mutex;
  ThreadID m_thread_id;
  uint  m_keep_count;
  uv_loop_t* m_uv_loop;
  uv_async_t* m_uv_async;
  uv_timer_t* m_uv_timer;
  int64 m_timeout;
  int64 m_record_timeout;
};

/**
 * @class KeepLoop 这个对像能保持RunLoop的循环不自动终止
 */
class XX_EXPORT KeepLoop: public Object, public PostMessage {
  XX_HIDDEN_ALL_COPY(KeepLoop);
 public:
  xx_default_allocator();
  /**
   * @destructor `destructor_clear=true`时会取消通过它`post`的所有消息
   */
  virtual ~KeepLoop();
  uint post(cCb& cb, uint64 delay_us = 0);
  virtual uint post_message(cCb& cb, uint64 delay_us = 0);
  /**
   * @func clear() 取消之前`post`的所有消息
   */
  void clear();
  inline void abort(uint id) { m_loop->abort(id); }
  inline RunLoop* host() { return m_loop; }
 private:
  /**
   * @constructor `declear=true`时表示析构时会进行清理
   */
  inline KeepLoop(bool destructor_clear)
  : m_group(iid32()), m_declear(destructor_clear) {
  }
  RunLoop* m_loop;
  uint  m_group;
  bool  m_declear;
  friend class RunLoop;
};

XX_END
#endif
