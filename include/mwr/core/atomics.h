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

namespace mwr {

inline void barrier() {
#ifdef MWR_MSVC
    _ReadWriteBarrier();
#else
    asm volatile("" : : : "memory");
#endif
}

inline void read_once(void* dest, const volatile void* src, size_t n) {
    switch (n) {
#if defined(MWR_GCC) || defined(MWR_CLANG)
    case 1:
        *(u8*)dest = __atomic_load_n((const volatile u8*)src,
                                     __ATOMIC_SEQ_CST);
        break;
    case 2:
        *(u16*)dest = __atomic_load_n((const volatile u16*)src,
                                      __ATOMIC_SEQ_CST);
        break;
    case 4:
        *(u32*)dest = __atomic_load_n((const volatile u32*)src,
                                      __ATOMIC_SEQ_CST);
        break;
    case 8:
        *(u64*)dest = __atomic_load_n((const volatile u64*)src,
                                      __ATOMIC_SEQ_CST);
        break;
#else
    case 1:
        *(u8*)dest = *(const volatile u8*)src;
        break;
    case 2:
        *(u16*)dest = *(const volatile u16*)src;
        break;
    case 4:
        *(u32*)dest = *(const volatile u32*)src;
        break;
    case 8:
        *(u64*)dest = *(const volatile u64*)src;
        break;
#endif
    default:
        barrier();
        memcpy(dest, (const void*)src, n);
        barrier();
    }
}

inline void write_once(volatile void* dest, const void* src, size_t n) {
    switch (n) {
#if defined(MWR_GCC) || defined(MWR_CLANG)
    case 1:
        __atomic_store_n((volatile u8*)dest, *(const u8*)src,
                         __ATOMIC_SEQ_CST);
        break;
    case 2:
        __atomic_store_n((volatile u16*)dest, *(const u16*)src,
                         __ATOMIC_SEQ_CST);
        break;
    case 4:
        __atomic_store_n((volatile u32*)dest, *(const u32*)src,
                         __ATOMIC_SEQ_CST);
        break;
    case 8:
        __atomic_store_n((volatile u64*)dest, *(const u64*)src,
                         __ATOMIC_SEQ_CST);
        break;
#else
    case 1:
        *(volatile u8*)dest = *(const u8*)src;
        break;
    case 2:
        *(volatile u16*)dest = *(const u16*)src;
        break;
    case 4:
        *(volatile u32*)dest = *(const u32*)src;
        break;
    case 8:
        *(volatile u64*)dest = *(const u64*)src;
        break;
#endif
    default:
        barrier();
        memcpy((void*)dest, src, n);
        barrier();
    }
}

template <typename T>
inline T read_once(const volatile void* src) {
    T val = 0;
    read_once(&val, src, sizeof(T));
    return val;
}

template <typename T>
inline void write_once(void* dest, const T& val) {
    write_once(dest, &val, sizeof(val));
}

inline u8 atomic_and8(volatile void* mem, u8 val) {
#ifdef MWR_MSVC
    return _InterlockedAnd8((volatile char*)mem, (char)val);
#else
    return __atomic_fetch_and((volatile u8*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline u16 atomic_and16(volatile void* mem, u16 val) {
#ifdef MWR_MSVC
    return _InterlockedAnd16((volatile short*)mem, (short)val);
#else
    return __atomic_fetch_and((volatile u16*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline u32 atomic_and32(volatile void* mem, u32 val) {
#ifdef MWR_MSVC
    return _InterlockedAnd((volatile long*)mem, (long)val);
#else
    return __atomic_fetch_and((volatile u32*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline u64 atomic_and64(volatile void* mem, u64 val) {
#ifdef MWR_MSVC
    return _InterlockedAnd64((volatile long long*)mem, (long long)val);
#else
    return __atomic_fetch_and((volatile u64*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline u64 atomic_and(volatile void* mem, u64 val, size_t size) {
    switch (size) {
    case 1:
        return atomic_and8(mem, (u8)val);
    case 2:
        return atomic_and16(mem, (u16)val);
    case 4:
        return atomic_and32(mem, (u32)val);
    case 8:
        return atomic_and64(mem, (u64)val);
    default:
        MWR_ERROR("%s: unsupported operand size: %zu bytes", __func__, size);
    }
}

template <typename T>
inline T atomic_and(volatile void* mem, T val) {
    return (T)atomic_and(mem, val, sizeof(T));
}

inline u8 atomic_or8(volatile void* mem, u8 val) {
#ifdef MWR_MSVC
    return _InterlockedOr8((volatile char*)mem, (char)val);
#else
    return __atomic_fetch_or((volatile u8*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline u16 atomic_or16(volatile void* mem, u16 val) {
#ifdef MWR_MSVC
    return _InterlockedOr16((volatile short*)mem, (short)val);
#else
    return __atomic_fetch_or((volatile u16*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline u32 atomic_or32(volatile void* mem, u32 val) {
#ifdef MWR_MSVC
    return _InterlockedOr((volatile long*)mem, (long)val);
#else
    return __atomic_fetch_or((volatile u32*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline u64 atomic_or64(volatile void* mem, u64 val) {
#ifdef MWR_MSVC
    return _InterlockedOr64((volatile long long*)mem, (long long)val);
#else
    return __atomic_fetch_or((volatile u64*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline u64 atomic_or(volatile void* mem, u64 val, size_t size) {
    switch (size) {
    case 1:
        return atomic_or8(mem, (u8)val);
    case 2:
        return atomic_or16(mem, (u16)val);
    case 4:
        return atomic_or32(mem, (u32)val);
    case 8:
        return atomic_or64(mem, (u64)val);
    default:
        MWR_ERROR("%s: unsupported operand size: %zu bytes", __func__, size);
    }
}

template <typename T>
inline T atomic_or(volatile void* mem, T val) {
    return (T)atomic_or(mem, val, sizeof(T));
}

inline u8 atomic_xor8(volatile void* mem, u8 val) {
#ifdef MWR_MSVC
    return _InterlockedXor8((volatile char*)mem, (char)val);
#else
    return __atomic_fetch_xor((volatile u8*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline u16 atomic_xor16(volatile void* mem, u16 val) {
#ifdef MWR_MSVC
    return _InterlockedXor16((volatile short*)mem, (short)val);
#else
    return __atomic_fetch_xor((volatile u16*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline u32 atomic_xor32(volatile void* mem, u32 val) {
#ifdef MWR_MSVC
    return _InterlockedXor((volatile long*)mem, (long)val);
#else
    return __atomic_fetch_xor((volatile u32*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline u64 atomic_xor64(volatile void* mem, u64 val) {
#ifdef MWR_MSVC
    return _InterlockedXor64((volatile long long*)mem, (long long)val);
#else
    return __atomic_fetch_xor((volatile u64*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline u64 atomic_xor(volatile void* mem, u64 val, size_t size) {
    switch (size) {
    case 1:
        return atomic_xor8(mem, (u8)val);
    case 2:
        return atomic_xor16(mem, (u16)val);
    case 4:
        return atomic_xor32(mem, (u32)val);
    case 8:
        return atomic_xor64(mem, (u64)val);
    default:
        MWR_ERROR("%s: unsupported operand size: %zu bytes", __func__, size);
    }
}

template <typename T>
inline T atomic_xor(volatile void* mem, T val) {
    return (T)atomic_xor(mem, val, sizeof(T));
}

inline i8 atomic_add8(volatile void* mem, i8 val) {
#ifdef MWR_MSVC
    return _InterlockedExchangeAdd8((volatile char*)mem, (char)val);
#else
    return __atomic_fetch_add((volatile i8*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline i16 atomic_add16(volatile void* mem, i16 val) {
#ifdef MWR_MSVC
    return _InterlockedExchangeAdd16((volatile short*)mem, (short)val);
#else
    return __atomic_fetch_add((volatile i16*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline i32 atomic_add32(volatile void* mem, i32 val) {
#ifdef MWR_MSVC
    return _InterlockedExchangeAdd((volatile long*)mem, (long)val);
#else
    return __atomic_fetch_add((volatile i32*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline i64 atomic_add64(volatile void* mem, i64 val) {
#ifdef MWR_MSVC
    return _InterlockedExchangeAdd64((volatile long long*)mem, (long long)val);
#else
    return __atomic_fetch_add((volatile i64*)mem, val, __ATOMIC_SEQ_CST);
#endif
}

inline i64 atomic_add(volatile void* mem, i64 val, size_t size) {
    switch (size) {
    case 1:
        return atomic_add8(mem, (i8)val);
    case 2:
        return atomic_add16(mem, (i16)val);
    case 4:
        return atomic_add32(mem, (i32)val);
    case 8:
        return atomic_add64(mem, (i64)val);
    default:
        MWR_ERROR("%s: unsupported operand size: %zu bytes", __func__, size);
    }
}

template <typename T>
inline T atomic_add(volatile void* mem, T val) {
    return (T)atomic_add(mem, val, sizeof(T));
}

inline u8 atomic_swap8(volatile void* mem, u8 val) {
#ifdef MWR_MSVC
    return _InterlockedExchange8((volatile char*)mem, (char)val);
#else
    return __atomic_exchange_n((volatile u8*)mem, (u8)val, __ATOMIC_SEQ_CST);
#endif
}

inline u16 atomic_swap16(volatile void* mem, u16 val) {
#ifdef MWR_MSVC
    return _InterlockedExchange16((volatile short*)mem, (short)val);
#else
    return __atomic_exchange_n((volatile u16*)mem, (u16)val, __ATOMIC_SEQ_CST);
#endif
}

inline u32 atomic_swap32(volatile void* mem, u32 val) {
#ifdef MWR_MSVC
    return _InterlockedExchange((volatile long*)mem, (long)val);
#else
    return __atomic_exchange_n((volatile u32*)mem, (u32)val, __ATOMIC_SEQ_CST);
#endif
}

inline u64 atomic_swap64(volatile void* mem, u64 val) {
#ifdef MWR_MSVC
    return _InterlockedExchange64((volatile long long*)mem, (long long)val);
#else
    return __atomic_exchange_n((volatile u64*)mem, (u64)val, __ATOMIC_SEQ_CST);
#endif
}

inline u64 atomic_swap(volatile void* mem, u64 val, size_t size) {
    switch (size) {
    case 1:
        return atomic_swap8(mem, (u8)val);
    case 2:
        return atomic_swap16(mem, (u16)val);
    case 4:
        return atomic_swap32(mem, (u32)val);
    case 8:
        return atomic_swap64(mem, (u64)val);
    default:
        MWR_ERROR("%s: unsupported operand size: %zu bytes", __func__, size);
    }
}

template <typename T>
inline T atomic_swap(volatile void* mem, T val) {
    return (T)atomic_swap(mem, val, sizeof(T));
}

inline bool atomic_cas8(volatile void* ptr, const void* cmp, const void* val) {
    auto comp = read_once<u8>(cmp);
    auto newv = read_once<u8>(val);
#ifdef MWR_MSVC
    return _InterlockedCompareExchange8((volatile char*)ptr, newv, comp) ==
           comp;
#else
    return __atomic_compare_exchange((volatile u8*)ptr, &comp, &newv, false,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#endif
}

inline bool atomic_cas16(volatile void* ptr, const void* cmp,
                         const void* val) {
    auto comp = read_once<u16>(cmp);
    auto newv = read_once<u16>(val);
#ifdef MWR_MSVC
    return _InterlockedCompareExchange16((volatile short*)ptr, newv, comp) ==
           comp;
#else
    return __atomic_compare_exchange((volatile u16*)ptr, &comp, &newv, false,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#endif
}

inline bool atomic_cas32(volatile void* ptr, const void* cmp,
                         const void* val) {
    auto comp = read_once<u32>(cmp);
    auto newv = read_once<u32>(val);
#ifdef MWR_MSVC
    return _InterlockedCompareExchange((volatile long*)ptr, newv, comp) ==
           comp;
#else
    return __atomic_compare_exchange((volatile u32*)ptr, &comp, &newv, false,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#endif
}

inline bool atomic_cas64(volatile void* ptr, const void* cmp,
                         const void* val) {
    auto comp = read_once<u64>(cmp);
    auto newv = read_once<u64>(val);
#ifdef MWR_MSVC
    return _InterlockedCompareExchange64((volatile long long*)ptr, newv,
                                         comp) == comp;
#else
    return __atomic_compare_exchange((volatile u64*)ptr, &comp, &newv, false,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#endif
}

inline bool atomic_cas128(volatile void* ptr, const void* cmp,
                          const void* val) {
    MWR_DECL_ALIGN(16) u64 comp[2];
    MWR_DECL_ALIGN(16) u64 newv[2];
    memcpy(comp, cmp, sizeof(comp));
    memcpy(newv, val, sizeof(newv));
#ifdef MWR_MSVC
    return _InterlockedCompareExchange128((volatile __int64*)ptr, newv[1],
                                          newv[0], (__int64*)comp);
#else
#if defined(MWR_X86_64)
    u8 res = 0;
    asm volatile(
        "lock cmpxchg16b %[dst]\n"
        "sete %[res]\n"
        : [dst] "+m"(*(u64*)ptr), [res] "=r"(res)
        : "d"(comp[1]), "a"(comp[0]), "c"(newv[1]), "b"(newv[0])
        : "memory", "cc");
    return res;
#elif defined(MWR_ARM64)
    u64 oldl, oldh;
    u32 temp = 1;
    asm volatile(
        "0: ldaxp %[oldl], %[oldh], %[mem]\n"
        "   cmp   %[oldl], %[cmpl]\n"
        "   ccmp  %[oldh], %[cmph], #0, eq\n"
        "   b.ne  1f\n"
        "   stlxp %w[temp], %[vall], %[valh], %[mem]\n"
        "   cbnz  %w[temp], 0b\n"
        "1:\n"
        : [temp] "+r"(temp), [oldl] "=&r"(oldl), [oldh] "=&r"(oldh)
        : [mem] "Q"(*(u64*)ptr), [cmpl] "r"(comp[0]), [cmph] "r"(comp[1]),
          [vall] "r"(newv[0]), [valh] "r"(newv[1])
        : "memory", "cc");
    return temp == 0;
#else
#error "atomic cas128 not supported"
#endif
#endif
}

inline bool atomic_cas(volatile void* ptr, const void* cmp, const void* val,
                       size_t size) {
    switch (size) {
    case 1:
        return atomic_cas8(ptr, cmp, val);
    case 2:
        return atomic_cas16(ptr, cmp, val);
    case 4:
        return atomic_cas32(ptr, cmp, val);
    case 8:
        return atomic_cas64(ptr, cmp, val);
    case 16:
        return atomic_cas128(ptr, cmp, val);
    default:
        MWR_ERROR("atomic_cas: unsupported operand size: %zu bytes", size);
    }
}

template <typename T>
inline bool atomic_cas(volatile void* ptr, T cmp, T data) {
    return atomic_cas(ptr, &cmp, &data, sizeof(T));
}

template <typename T>
inline T atomic_min(volatile void* mem, T val) {
    while (true) {
        T curr = read_once<T>(mem);
        T next = std::min<T>(curr, val);
        if (atomic_cas(mem, curr, next))
            return curr;
    };
}

template <typename T>
inline T atomic_max(volatile void* mem, T val) {
    while (true) {
        T curr = read_once<T>(mem);
        T next = std::max<T>(curr, val);
        if (atomic_cas(mem, curr, next))
            return curr;
    };
}

} // namespace mwr

#endif
