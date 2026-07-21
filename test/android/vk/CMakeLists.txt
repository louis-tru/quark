cmake_minimum_required(VERSION 3.22.1)
project(test_vk LANGUAGES C CXX)

add_library(native_app_glue STATIC
	${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC
	${ANDROID_NDK}/sources/android/native_app_glue)

add_library(test_vk SHARED main.cc)
target_compile_features(test_vk PRIVATE cxx_std_14)
target_link_options(test_vk PRIVATE "-uANativeActivity_onCreate")
target_link_libraries(test_vk PRIVATE native_app_glue android log vulkan)
