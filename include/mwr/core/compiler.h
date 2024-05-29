/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_COMPILER_H
#define MWR_COMPILER_H

#if defined(__GNUC__)
#define MWR_GCC
#elif defined(__clang__)
#define MWR_CLANG
#elif defined(_MSC_VER)
#define MWR_MSVC
#else
#error unsupported compiler
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
#else
#error unsupported cpu architecture
#endif

#ifdef MWR_MSVC
#include <intrin.h>
#endif

#define MWR_CPLUSPLUS_97 199711L
#define MWR_CPLUSPLUS_11 201103L
#define MWR_CPLUSPLUS_14 201402L
#define MWR_CPLUSPLUS_17 201703L
#define MWR_CPLUSPLUS_20 202002L

#if defined(MWR_GCC) || defined(MWR_CLANG)
#define MWR_DECL_ALIGN(n)     __attribute__((aligned(n)))
#define MWR_DECL_WEAK         __attribute__((weak))
#define MWR_DECL_PRINTF(s, a) __attribute__((format(printf, s, a)))
#define MWR_DECL_INLINE       __attribute__((always_inline))
#define MWR_DECL_NOINLINE     __attribute__((noinline))
#define MWR_DECL_NORETURN     __attribute__((noreturn))
#define MWR_DECL_DEPRECATED   __attribute__((deprecated))
#elif defined(MWR_MSVC)
#define MWR_DECL_ALIGN(n)     __declspec(align(n))
#define MWR_DECL_WEAK         __declspec(selectany)
#define MWR_DECL_PRINTF(s, a) /* MSVC does not support this */
#define MWR_DECL_INLINE       __declspec(inline)
#define MWR_DECL_NOINLINE     __declspec(noinline)
#define MWR_DECL_NORETURN     __declspec(noreturn)
#define MWR_DECL_DEPRECATED   __declspec(deprecated)
#endif

#if defined(MWR_GCC) || defined(MWR_CLANG)
#define MWR_UNREACHABLE __builtin_unreachable()
#elif defined(MWR_MSVC)
#define MWR_UNREACHABLE __assume(0)
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
        MWR_CAT(fn, _t)() {                 \
            fn();                           \
        }                                   \
    };                                      \
    static MWR_CAT(fn, _t) MWR_CAT(g_, fn); \
    }                                       \
    static void fn()

#define MWR_DESTRUCTOR(fn)                  \
    static void fn();                       \
    namespace {                             \
    struct MWR_CAT(fn, _t) {                \
        ~MWR_CAT(fn, _t)() {                \
            fn();                           \
        }                                   \
    };                                      \
    static MWR_CAT(fn, _t) MWR_CAT(g_, fn); \
    }                                       \
    static void fn()

namespace mwr {

template <typename T>
constexpr int likely(const T& x) {
#if defined(MWR_GCC) || defined(MWR_CLANG)
    return __builtin_expect(!!(x), 1);
#else
    return !!(x);
#endif
}

template <typename T>
constexpr int unlikely(const T& x) {
#if defined(MWR_GCC) || defined(MWR_CLANG)
    return __builtin_expect(!!(x), 0);
#else
    return !!(x);
#endif
}

} // namespace mwr

#endif
