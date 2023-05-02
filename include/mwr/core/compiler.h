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

#define MWR_DECL_ALIGN(n)     __attribute__((aligned(n)))
#define MWR_DECL_PACKED       __attribute__((packed))
#define MWR_DECL_PRINTF(s, a) __attribute__((format(printf, s, a)))
#define MWR_DECL_CONSTRUCTOR  __attribute__((constructor))
#define MWR_DECL_DESTRUCTOR   __attribute__((destructor))
#define MWR_DECL_WEAK         __attribute__((weak))
#define MWR_DECL_USED         __attribute__((used))
#define MWR_DECL_UNUSED       __attribute__((unused))
#define MWR_DECL_INLINE       __attribute__((always_inline))
#define MWR_DECL_NOINLINE     __attribute__((noinline))
#define MWR_DECL_NORETURN     __attribute__((noreturn))
#define MWR_DECL_DEPRECATED   __attribute__((deprecated))

#define MWR_UNREACHABLE __builtin_unreachable()

#define MWR_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define MWR_NOP(val)   val
#define MWR_XCAT(a, b) a##b
#define MWR_CAT(a, b)  MWR_XCAT(a, b)
#define MWR_XSTR(str)  #str
#define MWR_STR(str)   MWR_XSTR(str)

namespace mwr {

template <typename T>
constexpr int likely(const T& x) {
    return __builtin_expect(!!(x), 1);
}

template <typename T>
constexpr int unlikely(const T& x) {
    return __builtin_expect(!!(x), 0);
}

} // namespace mwr

#endif
