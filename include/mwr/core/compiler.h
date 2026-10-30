/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file found  *
 * in the root directory of this source tree.                                 *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_COMPILER_H
#define MWR_COMPILER_H

#if defined(__clang__)
#define MWR_CLANG
#elif defined(__GNUC__)
#define MWR_GCC
#elif defined(_MSC_VER)
#define MWR_MSVC
#else
#error unsupported compiler
#endif

#ifdef MWR_GCC
#define MWR_GCC_VERSION __GNUC__
#else
#define MWR_GCC_VERSION 0
#endif

#ifdef MWR_CLANG
#define MWR_CLANG_VERSION __clang_major__
#else
#define MWR_CLANG_VERSION 0
#endif

#ifdef MWR_MSVC
#define MWR_MSVC_VERSION _MSC_VER
#else
#define MWR_MSVC_VERSION 0
#endif

#if defined(__GNUC__) && defined(_WIN64)
#define MWR_MINGW
#endif

#if defined(__linux__)
#define MWR_LINUX
#elif defined(__APPLE__)
#define MWR_MACOS
#elif defined(_WIN64)
#define MWR_WINDOWS
#else
#error unsupported operating system
#endif

#if defined(__x86_64) || defined(_M_X64)
#define MWR_X86_64
#elif defined(__aarch64__)
#define MWR_ARM64
#elif defined(__riscv) && (__riscv_xlen == 64)
#define MWR_RISCV64
#else
#error unsupported cpu architecture
#endif

#ifdef MWR_MSVC
#include <intrin.h>
#endif

#define MWR_CPLUSPLUS_97 199711l
#define MWR_CPLUSPLUS_11 201103l
#define MWR_CPLUSPLUS_14 201402l
#define MWR_CPLUSPLUS_17 201703l
#define MWR_CPLUSPLUS_20 202002l

#if defined(MWR_GCC) || defined(MWR_CLANG)
#define MWR_DECL_ALIGN(n) __attribute__((aligned(n)))
#if defined(MWR_MINGW)
#define MWR_DECL_WEAK __declspec(selectany)
#else
#define MWR_DECL_WEAK __attribute__((weak))
#endif
#define MWR_DECL_PRINTF(s, a) __attribute__((format(printf, s, a)))
#define MWR_DECL_INLINE       inline __attribute__((always_inline))
#define MWR_DECL_NOINLINE     __attribute__((noinline))
#define MWR_DECL_NORETURN     __attribute__((noreturn))
#define MWR_DECL_DEPRECATED   __attribute__((deprecated))
#elif defined(MWR_MSVC)
#define MWR_DECL_ALIGN(n)     __declspec(align(n))
#define MWR_DECL_WEAK         __declspec(selectany)
#define MWR_DECL_PRINTF(s, a) /* MSVC does not support this */
#define MWR_DECL_INLINE       __forceinline
#define MWR_DECL_NOINLINE     __declspec(noinline)
#define MWR_DECL_NORETURN     __declspec(noreturn)
#define MWR_DECL_DEPRECATED   __declspec(deprecated)
#endif

#if defined(__SANITIZE_THREAD__)
#define MWR_TSAN
#elif defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define MWR_TSAN
#endif
#endif

#if defined(MWR_X86_64) && defined(__ELF__) && !defined(MWR_TSAN) && \
    (MWR_GCC_VERSION >= 6 || MWR_CLANG_VERSION >= 14)
#define MWR_DECL_MULTIVERSION(...) __attribute__((target_clones(__VA_ARGS__)))
#else
#define MWR_DECL_MULTIVERSION(...)
#endif

#if defined(MWR_X86_64) && (MWR_GCC_VERSION >= 12 || MWR_CLANG_VERSION >= 14)
#define MWR_DECL_VECTORIZE \
    MWR_DECL_MULTIVERSION("default", "arch=x86-64-v3", "arch=x86-64-v4")
#elif defined(MWR_X86_64)
#define MWR_DECL_VECTORIZE MWR_DECL_MULTIVERSION("default", "avx2", "avx512f")
#else
#define MWR_DECL_VECTORIZE
#endif

#if defined(MWR_GCC) || defined(MWR_CLANG)
#define MWR_UNREACHABLE __builtin_unreachable()
#elif defined(MWR_MSVC)
#define MWR_UNREACHABLE __assume(0)
#endif

#if (defined(MWR_GCC) || defined(MWR_CLANG)) && \
    (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define MWR_HOST_BIG_ENDIAN
#else
#define MWR_HOST_LITTLE_ENDIAN
#endif

#define MWR_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define MWR_NOP(val)   val
#define MWR_XCAT(a, b) a##b
#define MWR_CAT(a, b)  MWR_XCAT(a, b)
#define MWR_XSTR(str)  #str
#define MWR_STR(str)   MWR_XSTR(str)

#define MWR_CONSTRUCTOR(fn)                 \
    static void fn();                       \
    namespace {                             \
    struct MWR_CAT(fn, _t) {                \
        MWR_CAT(fn, _t)() { fn(); }         \
    };                                      \
    static MWR_CAT(fn, _t) MWR_CAT(g_, fn); \
    }                                       \
    static void fn()

#define MWR_DESTRUCTOR(fn)                  \
    static void fn();                       \
    namespace {                             \
    struct MWR_CAT(fn, _t) {                \
        ~MWR_CAT(fn, _t)() { fn(); }        \
    };                                      \
    static MWR_CAT(fn, _t) MWR_CAT(g_, fn); \
    }                                       \
    static void fn()

namespace mwr {

template <typename T>
constexpr int likely(const T& x) {
#if defined(MWR_GCC) || defined(MWR_CLANG)
    return __builtin_expect(!!x, 1);
#else
    return !!(x);
#endif
}

template <typename T>
constexpr int unlikely(const T& x) {
#if defined(MWR_GCC) || defined(MWR_CLANG)
    return __builtin_expect(!!x, 0);
#else
    return !!(x);
#endif
}

#if defined(MWR_LINUX) && defined(MWR_GCC)
static void __gcov_init() __attribute__((weakref("__gcov_init"))); // NOLINT
inline bool gcov_enabled() {
    return &__gcov_init != nullptr;
}
#else
constexpr bool gcov_enabled() {
    return false;
}
#endif

} // namespace mwr

#endif
