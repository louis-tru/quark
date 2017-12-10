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

#include "ngui/js/js.h"
#include "ngui/base/localstorage.h"
#include "cb-1.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

/**
 * @class NativeStorage
 */
class NativeStorage {
  
  /**
   * @func get(key)
   * @arg key {String}
   * @ret {String}
   */
  static void get(FunctionCall args) {
    JS_WORKER(args);
    if (args.Length() < 1) {
      JS_THROW_ERR(
        "* @func get(key)\n"
        "* @arg key {String}\n"
        "* @ret {String}\n"
      );
    }
    JS_RETURN( localstorage_get( args[0]->ToStringValue(worker)) );
  }
  
  /**
   * @func set(key, value)
   * @arg key {String}
   * @arg value {String}
   */
  static void set(FunctionCall args) {
    JS_WORKER(args);
    if (args.Length() < 2) {
      JS_THROW_ERR(
        "* @func set(key)\n"
        "* @arg key {String}\n"
        "* @arg value {String}\n"
      );
    }
    localstorage_set( args[0]->ToStringValue(worker), args[1]->ToStringValue(worker) );
  }
  
  /**
   * @func del(key)
   * @arg key {String}
   */
  static void del(FunctionCall args) {
    JS_WORKER(args);
    if (args.Length() < 1) {
      JS_THROW_ERR(
        "* @func del(key)\n"
        "* @arg key {String}\n"
      );
    }
    localstorage_delete( args[0]->ToStringValue(worker) );
  }
  
  static void clear(FunctionCall args) {
    localstorage_clear();
  }
  
  static void transaction(FunctionCall args) {
    JS_WORKER(args);
    if (args.Length() < 1 || !args[0]->IsFunction()) {
      JS_THROW_ERR(
                   "* @func transaction(key)\n"
                   "* @arg cb {Function}\n"
                   );
    }
    localstorage_transaction(get_callback_for_none(worker, args[0]));
  }
  
 public:
  
  static void binding(Local<JSObject> exports, Worker* worker) {
    JS_SET_METHOD(get, get);
    JS_SET_METHOD(set, set);
    JS_SET_METHOD(del, del);
    JS_SET_METHOD(clear, clear);
    JS_SET_METHOD(transaction, transaction);
  }
};

JS_REG_MODULE(ngui_storage, NativeStorage);
JS_END
