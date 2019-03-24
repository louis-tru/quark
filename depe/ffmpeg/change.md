# libavutil/x86/asm.h

+ #if defined(__clang__) && defined(__ANDROID__) && ARCH_X86_32
+ #undef HAVE_EBX_AVAILABLE
+ #define HAVE_EBX_AVAILABLE 0
+ #endif

# libswresample/arm/audio_convert_neon.S

	blt             X(swri_oldapi_conv_flt_to_s16_neon)
+	it              eq
	beq             X(swri_oldapi_conv_fltp_to_s16_2ch_neon)