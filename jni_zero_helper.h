// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JNI_ZERO_JNI_ZERO_HELPER_H_
#define JNI_ZERO_JNI_ZERO_HELPER_H_

#include <jni.h>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"
#include "base/compiler_specific.h"
#include "base/memory/raw_ptr.h"
#if defined(USE_CHROMIUM_BASE)
// Used for ARCH_CPU_X86 - embedder must define this correctly if they want
// 16-byte stack alignment on x86.
#include "build/build_config.h"
#endif                     // defined(USE_CHROMIUM_BASE)
#include "third_party/jni_zero/core.h"
#include "third_party/jni_zero/jni_export.h"
#include "third_party/jni_zero/jni_int_wrapper.h"
#include "third_party/jni_zero/logging.h"

// Project-specific macros used by the header files generated by
// jni_generator.py. Different projects can then specify their own
// implementation for this file.
#define CHECK_NATIVE_PTR(env, jcaller, native_ptr, method_name, ...) \
  JNI_ZERO_DCHECK(native_ptr);

#define CHECK_CLAZZ(env, jcaller, clazz, ...) JNI_ZERO_DCHECK(clazz);

namespace jni_zero {

inline void HandleRegistrationError(JNIEnv* env,
                                    jclass clazz,
                                    const char* filename) {
  JNI_ZERO_ELOG("RegisterNatives failed in %s", filename);
}

// A 32 bit number could be an address on stack. Random 64 bit marker on the
// stack is much less likely to be present on stack.
constexpr uint64_t kJniStackMarkerValue = 0xbdbdef1bebcade1b;

// Context about the JNI call with exception checked to be stored in stack.
struct JNI_ZERO_COMPONENT_BUILD_EXPORT JniJavaCallContextUnchecked {
  ALWAYS_INLINE JniJavaCallContextUnchecked() {
// TODO(ssid): Implement for other architectures.
#if defined(__arm__) || defined(__aarch64__)
    // This assumes that this method does not increment the stack pointer.
    asm volatile("mov %0, sp" : "=r"(sp));
#else
    sp = 0;
#endif
  }

  // Force no inline to reduce code size.
  template <base::android::MethodID::Type type>
  NOINLINE void Init(JNIEnv* env,
                     jclass clazz,
                     const char* method_name,
                     const char* jni_signature,
                     std::atomic<jmethodID>* atomic_method_id) {
    env1 = env;

    // Make sure compiler doesn't optimize out the assignment.
    memcpy(&marker, &kJniStackMarkerValue, sizeof(kJniStackMarkerValue));
    // Gets PC of the calling function.
    pc = reinterpret_cast<uintptr_t>(__builtin_return_address(0));

    method_id = base::android::MethodID::LazyGet<type>(
        env, clazz, method_name, jni_signature, atomic_method_id);
  }

  NOINLINE ~JniJavaCallContextUnchecked() {
    // Reset so that spurious marker finds are avoided.
    memset(&marker, 0, sizeof(marker));
  }

  uint64_t marker;
  uintptr_t sp;
  uintptr_t pc;

  raw_ptr<JNIEnv> env1;
  jmethodID method_id;
};

// Context about the JNI call with exception unchecked to be stored in stack.
struct JNI_ZERO_COMPONENT_BUILD_EXPORT JniJavaCallContextChecked {
  // Force no inline to reduce code size.
  template <base::android::MethodID::Type type>
  NOINLINE void Init(JNIEnv* env,
                     jclass clazz,
                     const char* method_name,
                     const char* jni_signature,
                     std::atomic<jmethodID>* atomic_method_id) {
    base.Init<type>(env, clazz, method_name, jni_signature, atomic_method_id);
    // Reset |pc| to correct caller.
    base.pc = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
  }

  NOINLINE ~JniJavaCallContextChecked() { CheckException(base.env1); }

  JniJavaCallContextUnchecked base;
};

static_assert(sizeof(JniJavaCallContextChecked) ==
                  sizeof(JniJavaCallContextUnchecked),
              "Stack unwinder cannot work with structs of different sizes.");

}  // namespace jni_zero

#endif  // JNI_ZERO_JNI_ZERO_HELPER_H_
