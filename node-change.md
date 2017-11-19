
# depe/v8/src/base/cpu.cc

```cpp
#if V8_OS_IOS
  // apple ios system
#else
#endif // defined(V8_OS_IOS) 
```

# depe/v8/include/v8config.h

```cpp
# if defined(TARGET_OS_IPHONE)
#   define V8_OS_IOS 1
#   ifndef USE_JSC
#     #define USE_JSC 1
#   endif
# endif
```

# node/deps/zlib/zlib.gyp

```py
- 'type': 'static_library',
+ 'type': 'none',
```

# node/node.gypi

```py
# For tests
#'./deps/openssl/openssl.gyp:openssl-cli',
```

# node/src/inspector_io.cc

```cc
#include <ngui/js/node-1.h>

+  int len;
+  auto buff = ngui_api.encoding_to_utf8(source, (uint)view.length(), &len);

+  auto s = std::string(buff, len);
+  free(buff);
+  return s;

// 

+ std::unique_ptr<StringBuffer> Utf8ToStringView(const std::string& message) {
+  int len;
+  auto buff = ngui_api.decoding_utf8_to_uint16(message.c_str(),
                                               (uint)message.length(), &len);
+  StringView view(reinterpret_cast<const uint16_t*>(buff), len);
  
+  auto s = StringBuffer::create(view);
+  free(buff);
+  return s;
+ }

```

# node/src/node.cc

```cc

+ #include "ngui/js/node-2.h"

//

   Local<Value> arg = env->process_object();
   f->Call(Null(env->isolate()), 1, &arg);
+  try_catch.ReThrow();
}

//

int exec_argc, const char* const* exec_argv) {
+  ngui::RunLoop* loop = ngui_api.ngui_main_loop();

+ NguiEnvironment ngui_env(&env);

// 
    do {
-			uv_run(env.event_loop(), UV_RUN_DEFAULT);
+     ngui_api.run_ngui_loop(loop);

```

# node/lib/internal/bootstrap_node.js

```js
- Module._resolveFilename(process.argv[1]);
+ const filename = process.argv[1];
```

# node/lib/fs.js
# node/lib/module.js
# node/lib/pkg.js
# node/lib/internal/pkg.js
