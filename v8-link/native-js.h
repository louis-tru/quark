#ifndef __native__js__native_js_h__
#define __native__js__native_js_h__
namespace native_js {
struct JSC_NativeJSCode {
 int count;
 const char* code;
 const char* name;
};
extern const int JSC_native_js_code_jsc_v8_isolate_count_;
extern const unsigned char JSC_native_js_code_jsc_v8_isolate_[];
extern const int JSC_native_js_code_jsc_v8_context_count_;
extern const unsigned char JSC_native_js_code_jsc_v8_context_[];
extern const int JSC_native_js_count_;
extern const JSC_NativeJSCode JSC_native_js_[];
}
#endif
