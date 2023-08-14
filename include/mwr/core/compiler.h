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

#define MWR_CPLUSPLUS_97 199711L
#define MWR_CPLUSPLUS_11 201103L
#define MWR_CPLUSPLUS_14 201402L
#define MWR_CPLUSPLUS_17 201703L
#define MWR_CPLUSPLUS_20 202002L

#ifndef _MSC_VER
#define MWR_DECL_ALIGN(n)     __attribute__((aligned(n)))
#define MWR_DECL_PRINTF(s, a) __attribute__((format(printf, s, a)))
#define MWR_DECL_INLINE       __attribute__((always_inline))
#define MWR_DECL_NOINLINE     __attribute__((noinline))
#define MWR_DECL_NORETURN     __attribute__((noreturn))
#define MWR_DECL_DEPRECATED   __attribute__((deprecated))
#else
#define MWR_DECL_ALIGN(n)     __declspec(align(n))
#define MWR_DECL_PRINTF(s, a) /* MSVC does not support this */
#define MWR_DECL_INLINE       __declspec(inline)
#define MWR_DECL_NOINLINE     __declspec(noinline)
#define MWR_DECL_NORETURN     __declspec(noreturn)
#define MWR_DECL_DEPRECATED   __declspec(deprecated)
#endif

#define MWR_CONSTRUCTOR(fn) \
    static void fn();       \
    struct fn##_t {         \
        fn##_t() { fn(); }  \
    };                      \
    static fn##_t g_##fn;   \
    static void fn()

#define MWR_DESTRUCTOR(fn)  \
    static void fn();       \
    struct fn##_t {         \
        ~fn##_t() { fn(); } \
    };                      \
    static fn##_t g_##fn;   \
    static void fn()

#ifndef _MSC_VER
#define MWR_UNREACHABLE __builtin_unreachable()
#else
#define MWR_UNREACHABLE __assume(0)
#endif

#define MWR_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define MWR_NOP(val)   val
#define MWR_XCAT(a, b) a##b
#define MWR_CAT(a, b)  MWR_XCAT(a, b)
#define MWR_XSTR(str)  #str
#define MWR_STR(str)   MWR_XSTR(str)

namespace mwr {

template <typename T>
constexpr int likely(const T& x) {
#ifndef _MSC_VER
    return __builtin_expect(!!(x), 1);
#else
    return !!(x);
#endif
}

template <typename T>
constexpr int unlikely(const T& x) {
#ifndef _MSC_VER
    return __builtin_expect(!!(x), 0);
#else
    return !!(x);
#endif
}

} // namespace mwr

#endif
