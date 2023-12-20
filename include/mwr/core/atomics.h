/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_ATOMICS_H
#define MWR_ATOMICS_H

#include <algorithm>

#include "mwr/core/compiler.h"
#include "mwr/core/types.h"
#include "mwr/core/report.h"

#ifdef GCC_MSVC
#include <intrin.h>
#endif

namespace mwr {

inline void barrier() {
#ifdef MWR_MSVC
    _ReadWriteBarrier();
#else
    asm volatile("" : : : "memory");
#endif
}

inline void read_once(void* dest, const void* src, size_t n) {
    switch (n) {
    case 1:
        *(u8*)dest = *(volatile u8*)src;
        break;
    case 2:
        *(u16*)dest = *(volatile u16*)src;
        break;
    case 4:
        *(u32*)dest = *(volatile u32*)src;
        break;
    case 8:
        *(u64*)dest = *(volatile u64*)src;
        break;
    default:
        barrier();
        memcpy(dest, src, n);
        barrier();
    }
}

inline void write_once(void* dest, const void* src, size_t n) {
    switch (n) {
    case 1:
        *(volatile u8*)dest = *(u8*)src;
        break;
    case 2:
        *(volatile u16*)dest = *(u16*)src;
        break;
    case 4:
        *(volatile u32*)dest = *(u32*)src;
        break;
    case 8:
        *(volatile u64*)dest = *(u64*)src;
        break;
    default:
        barrier();
        memcpy(dest, src, n);
        barrier();
    }
}

template <typename T>
inline T read_once(const void* src) {
    T val = 0;
    read_once(&val, src, sizeof(T));
    return val;
}

template <typename T>
inline void write_once(void* dest, const T& val) {
    write_once(dest, &val, sizeof(val));
}

template <typename T, typename T2 = T, typename T3 = T>
inline bool atomic_cmpxchg(T* ptr, T2 expected, T3 desired) {
#ifndef MWR_MSVC
    return __atomic_compare_exchange(ptr, (T*)&expected, (T*)&desired, false,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#else
    if constexpr (sizeof(T) == sizeof(char))
        return _InterlockedCompareExchange8((volatile char*)ptr, (char)desired,
                                            (char)expected);
    if constexpr (sizeof(T) == sizeof(short))
        return _InterlockedCompareExchange16((volatile short*)ptr,
                                             (short)desired, (short)expected);
    if constexpr (sizeof(T) == sizeof(long))
        return _InterlockedCompareExchange((volatile long*)ptr, (long)desired,
                                           (long)expected);
    if constexpr (sizeof(T) == sizeof(long long))
        return _InterlockedCompareExchange64(
            (volatile long long*)ptr, (long long)desired, (long long)expected);
    MWR_UNREACHABLE;
#endif
}

inline bool atomic_cas8(volatile void* ptr, void* cmp, void* val) {
#ifndef MWR_MSVC
    return __atomic_compare_exchange((volatile u8*)ptr, (u8*)cmp, (u8*)val,
                                     false, __ATOMIC_SEQ_CST,
                                     __ATOMIC_SEQ_CST);
#else
    return _InterlockedCompareExchange8((volatile u8*)ptr, read_once<u8>(val),
                                        read_once<u8>(cmp));
#endif
}

inline bool atomic_cas16(volatile void* ptr, void* cmp, void* val) {
#ifndef MWR_MSVC
    return __atomic_compare_exchange((volatile u16*)ptr, (u16*)cmp, (u16*)val,
                                     false, __ATOMIC_SEQ_CST,
                                     __ATOMIC_SEQ_CST);
#else
    return _InterlockedCompareExchange16(
        (volatile u16*)ptr, read_once<u16>(val), read_once<u16>(cmp));
#endif
}

inline bool atomic_cas32(volatile void* ptr, void* cmp, void* val) {
#ifndef MWR_MSVC
    return __atomic_compare_exchange((volatile u32*)ptr, (u32*)cmp, (u32*)val,
                                     false, __ATOMIC_SEQ_CST,
                                     __ATOMIC_SEQ_CST);
#else
    return _InterlockedCompareExchange((volatile u32*)ptr, read_once<u32>(val),
                                       read_once<u32>(cmp));
#endif
}

inline bool atomic_cas64(volatile void* ptr, void* cmp, void* val) {
#ifndef MWR_MSVC
    return __atomic_compare_exchange((volatile u64*)ptr, (u64*)cmp, (u64*)val,
                                     false, __ATOMIC_SEQ_CST,
                                     __ATOMIC_SEQ_CST);
#else
    return _InterlockedCompareExchange64(
        (volatile u64*)ptr, read_once<u64>(val), read_once<u64>(cmp));
#endif
}

inline bool atomic_cas128(volatile void* ptr, void* cmp, void* val) {
#ifdef MWR_MSVC
    return InterlockedCompareExchange128((volatile LONG64*)ptr, (u64*)cmp[1],
                                         (u64*)cmp[0], (LONG64*)val);
#else
    u64 cmpl = ((u64*)cmp)[0];
    u64 cmph = ((u64*)cmp)[1];
    u64 vall = ((u64*)val)[0];
    u64 valh = ((u64*)val)[1];
#if defined(__x86_64)
    u8 res = 0;
    asm volatile(
        "lock cmpxchg16b %[dst]\n"
        "sete %[res]\n"
        : [dst] "+m"(((u64*)ptr)[0]), [res] "=r"(res)
        : "d"(cmph), "a"(cmpl), "c"(valh), "b"(vall)
        : "memory");
    return res;
#elif defined(aarch64)
    u64 oldl, oldh;
    u32 temp;
    asm volatile(
        "0: ldaxp %[ol], %[oh], %[mem]\n"
        "   cmp   %[ol], %[cl]\n"
        "   ccmp  %[oh], %[ch], #0, eq\n"
        "   b.ne 1f\n"
        "   stlxp %w[temp], %[vl], %[vh], %[mem]\n"
        "   cbnz %w[temp], 0b\n"
        "1:"
        : [mem] "+m"(((u64*)ptr)[0]), [temp] "=&r"(temp), [ol] "=&r"(oldl),
          [oh] "=&r"(oldh)
        : [cl] "r"(cmpl), [ch] "r"(cmph), [vl] "r"(vall), [vh] "r"(valh)
        : "memory", "cc");
    return temp == 0;
#else
#error "atomic cas128 not supported"
#endif
#endif
}

template <typename T, typename T2 = T>
inline T atomic_or(T* mem, T2 val) {
#ifndef MWR_MSVC
    return __atomic_fetch_or(mem, (T)val, __ATOMIC_SEQ_CST);
#else
    if constexpr (sizeof(T) == sizeof(char))
        return _InterlockedOr8((volatile char*)mem, (char)val);
    if constexpr (sizeof(T) == sizeof(short))
        return _InterlockedOr16((volatile short*)mem, (short)val);
    if constexpr (sizeof(T) == sizeof(long))
        return _InterlockedOr((volatile long*)mem, (long)val);
    if constexpr (sizeof(T) == sizeof(long long))
        return _InterlockedOr64((volatile long long*)mem, (long long)val);
    MWR_UNREACHABLE;
#endif
}

template <typename T, typename T2 = T>
inline T atomic_xor(T* mem, T2 val) {
#ifndef MWR_MSVC
    return __atomic_fetch_xor(mem, (T)val, __ATOMIC_SEQ_CST);
#else
    if constexpr (sizeof(T) == sizeof(char))
        return _InterlockedXor8((volatile char*)mem, (char)val);
    if constexpr (sizeof(T) == sizeof(short))
        return _InterlockedXor16((volatile short*)mem, (short)val);
    if constexpr (sizeof(T) == sizeof(long))
        return _InterlockedXor((volatile long*)mem, (long)val);
    if constexpr (sizeof(T) == sizeof(long long))
        return _InterlockedXor64((volatile long long*)mem, (long long)val);
    MWR_UNREACHABLE;
#endif
}

template <typename T, typename T2 = T>
inline T atomic_and(T* mem, T2 val) {
#ifndef MWR_MSVC
    return __atomic_fetch_and(mem, (T)val, __ATOMIC_SEQ_CST);
#else
    if constexpr (sizeof(T) == sizeof(char))
        return _InterlockedAnd8((volatile char*)mem, (char)val);
    if constexpr (sizeof(T) == sizeof(short))
        return _InterlockedAnd16((volatile short*)mem, (short)val);
    if constexpr (sizeof(T) == sizeof(long))
        return _InterlockedAnd((volatile long*)mem, (long)val);
    if constexpr (sizeof(T) == sizeof(long long))
        return _InterlockedAnd64((volatile long long*)mem, (long long)val);
    MWR_UNREACHABLE;
#endif
}

template <typename T, typename T2 = T>
static T atomic_add(T* mem, T2 val) {
#ifndef MWR_MSVC
    return __atomic_fetch_add(mem, (T)val, __ATOMIC_SEQ_CST);
#else
    if constexpr (sizeof(T) == sizeof(char))
        return _InterlockedExchangeAdd8((volatile char*)mem, (char)val);
    if constexpr (sizeof(T) == sizeof(short))
        return _InterlockedExchangeAdd16((volatile short*)mem, (short)val);
    if constexpr (sizeof(T) == sizeof(long))
        return _InterlockedExchangeAdd((volatile long*)mem, (long)val);
    if constexpr (sizeof(T) == sizeof(long long))
        return _InterlockedExchangeAdd64((volatile long long*)mem,
                                         (long long)val);
    MWR_UNREACHABLE;
#endif
}

template <typename T, typename T2 = T>
inline T atomic_min(T* mem, T2 val) {
    T curr, next;
    while (true) {
        curr = *mem;
        next = std::min<T>(curr, val);
        if (atomic_cmpxchg(mem, curr, next))
            return curr;
    };
}

template <typename T, typename T2 = T>
inline T atomic_max(T* mem, T2 val) {
    T curr, next;
    while (true) {
        curr = *mem;
        next = std::max<T>(curr, val);
        if (atomic_cmpxchg(mem, curr, next))
            return curr;
    };
}

template <typename T, typename T2 = T>
inline T atomic_swap(T* mem, T2 val) {
#ifndef MWR_MSVC
    return __atomic_exchange_n(mem, (T2)val, __ATOMIC_SEQ_CST);
#else
    if constexpr (sizeof(T) == sizeof(char))
        return _InterlockedExchange8((volatile char*)mem, (char)val);
    if constexpr (sizeof(T) == sizeof(short))
        return _InterlockedExchange16((volatile short*)mem, (short)val);
    if constexpr (sizeof(T) == sizeof(long))
        return _InterlockedExchange((volatile long*)mem, (long)val);
    if constexpr (sizeof(T) == sizeof(long long))
        return _InterlockedExchange64((volatile long long*)mem,
                                      (long long)val);
    MWR_UNREACHABLE;
#endif
}

template <typename FN>
inline u64 atomic_dispatch(void* ptr, const void* val, size_t size, FN&& fn) {
    switch (size) {
    case 1:
        return fn((u8*)ptr, *(const u8*)val);

    case 2:
        return fn((u16*)ptr, *(const u16*)val);

    case 4:
        return fn((u32*)ptr, *(const u32*)val);

    case 8:
        return fn((u64*)ptr, *(const u64*)val);

    default:
        MWR_ERROR("unsupported operand size: %zu bytes", size);
    }
}

inline u64 atomic_or_ptr(void* ptr, const void* val, size_t size) {
    return atomic_dispatch(ptr, val, size, [](auto* ptr, auto val) -> u64 {
        return atomic_or(ptr, val);
    });
}

inline u64 atomic_xor_ptr(void* ptr, const void* val, size_t size) {
    return atomic_dispatch(ptr, val, size, [](auto* ptr, auto val) -> u64 {
        return atomic_xor(ptr, val);
    });
}

inline u64 atomic_and_ptr(void* ptr, const void* val, size_t size) {
    return atomic_dispatch(ptr, val, size, [](auto* ptr, auto val) -> u64 {
        return atomic_and(ptr, val);
    });
}

inline u64 atomic_add_ptr(void* ptr, const void* val, size_t size) {
    return atomic_dispatch(ptr, val, size, [](auto* ptr, auto val) -> u64 {
        return atomic_add(ptr, val);
    });
}

inline i64 atomic_min_ptr(void* ptr, const void* val, size_t size) {
    return atomic_dispatch(ptr, val, size, [](auto* ptr, auto val) -> u64 {
        typedef typename std::make_signed<decltype(val)>::type type;
        return atomic_min((type*)ptr, (type)val);
    });
}

inline i64 atomic_max_ptr(void* ptr, const void* val, size_t size) {
    return atomic_dispatch(ptr, val, size, [](auto* ptr, auto val) -> u64 {
        typedef typename std::make_signed<decltype(val)>::type type;
        return atomic_max((type*)ptr, (type)val);
    });
}

inline u64 atomic_umin_ptr(void* ptr, const void* val, size_t size) {
    return atomic_dispatch(ptr, val, size, [](auto* ptr, auto val) -> u64 {
        return atomic_min(ptr, val);
    });
}

inline u64 atomic_umax_ptr(void* ptr, const void* val, size_t size) {
    return atomic_dispatch(ptr, val, size, [](auto* ptr, auto val) -> u64 {
        return atomic_max(ptr, val);
    });
}

inline u64 atomic_swap_ptr(void* ptr, const void* val, size_t size) {
    return atomic_dispatch(ptr, val, size, [](auto* ptr, auto val) -> u64 {
        return atomic_swap(ptr, val);
    });
}

inline bool atomic_cmpxchg(volatile void* ptr, u64 cmp, const void* val,
                           size_t size) {
    switch (size) {
    case 1:
        return atomic_cmpxchg((u8*)ptr, (u8)cmp, *(const u8*)val);

    case 2:
        return atomic_cmpxchg((u16*)ptr, (u16)cmp, *(const u16*)val);

    case 4:
        return atomic_cmpxchg((u32*)ptr, (u32)cmp, *(const u32*)val);

    case 8:
        return atomic_cmpxchg((u64*)ptr, (u64)cmp, *(const u64*)val);

    default:
        MWR_ERROR("unsupported operand size: %zu bytes", size);
    }
}

} // namespace mwr

#endif
