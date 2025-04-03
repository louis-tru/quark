#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_org_quark_test_MainActivity1_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}