
# depe/v8/src/base/cpu.cc

```cpp
+ #if V8_OS_IOS
+   // apple ios system
+ #else
+ #endif // defined(V8_OS_IOS) 
```

# depe/v8/include/v8config.h

```cpp
+ # if defined(TARGET_OS_IPHONE)
+ #   define V8_OS_IOS 1
+ # endif
```

# node/deps/zlib/zlib.gyp

```py
- 'type': 'static_library',
+ 'type': 'none',
```

# node/node.gypi

```py
- # For tests
- './deps/openssl/openssl.gyp:openssl-cli',
```

# node/node.gyp

```py
- 'src/tracing/trace_event.h'
+ 'src/tracing/trace_event.h',
```

# node/src/inspector_io.cc

```cc
+ #include <qgr/js/node-1.h>

+  int len;
+  auto buff = qgr_api->encoding_to_utf8(source, (uint)view.length(), &len);

+  auto s = std::string(buff, len);
+  free(buff);
+  return s;

// 

+ std::unique_ptr<StringBuffer> Utf8ToStringView(const std::string& message) {
+  int len;
+  auto buff = qgr_api->decoding_utf8_to_uint16(message.c_str(),
                                               (uint)message.length(), &len);
+  StringView view(reinterpret_cast<const uint16_t*>(buff), len);
  
+  auto s = StringBuffer::create(view);
+  free(buff);
+  return s;
+ }

```

# node/src/node.cc

```cc

+ #include "qgr/js/node-2.h"

//

   Local<Value> arg = env->process_object();
   f->Call(Null(env->isolate()), 1, &arg);
+  try_catch.ReThrow();
}

//

int exec_argc, const char* const* exec_argv) {
+  qgr::RunLoop* loop = qgr_api->qgr_main_loop();

+ qgrEnvironment qgr_env(&env, is_inspector, argc, argv);

// 
    do {
-			uv_run(env.event_loop(), UV_RUN_DEFAULT);
+     qgr_api->run_qgr_loop(loop);
+			if (qgr_api->is_process_exit()) break;

//

+	if (!qgr_api->is_process_exit()) {

```

# node/src/node_crypto.cc

```cc
- static X509_STORE* NewRootCertStore()
+ X509_STORE* NewRootCertStore()
```

# node/src/base-object-inl.h

```cc
-   self->persistent().Reset();
+   self->persistent().V8_DEATH_RESET();
```

# node/src/node_api.cc
# node/src/node_object_wrap.h
# node/src/node_buffer.cc

# node/lib/internal/bootstrap_node.js
# node/lib/fs.js

```js
+ const pkg = require('internal/pkg');
+ const getPathFromURL = pkg.getPathFromURL;
```

# node/lib/module.js
# node/lib/pkg.js
# node/lib/internal/pkg.js

# node/lib/buffer.js

```js
class FastBuffer extends Uint8Array {
-  constructor(arg1, arg2, arg3) {
+  constructor(...args) {
-    super(arg1, arg2, arg3);
+    super(...args);
	}
}
```

# node/deps/cares/config/android/ares_config.h

```cc
- #define HAVE_GETSERVBYPORT_R 1
+ /* #define HAVE_GETSERVBYPORT_R 1 */
```

# node/.gitignore

+ !deps/v8/src/base/debug

# deps/v8/src/inspector/inspector.gypi

```py
- '../../include/v8-inspector.h',
- '../../include/v8-inspector-protocol.h',
+ #'../../include/v8-inspector.h',
+ #'../../include/v8-inspector-protocol.h',
```
