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

#ifdef _MSC_VER
#include <Windows.h>
#endif

#include "mwr/core/types.h"
#include "mwr/core/report.h"

namespace mwr {

inline void barrier() {
#ifndef _MSC_VER
    asm volatile("" : : : "memory");
#else
    _ReadWriteBarrier();
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
    return 
        val;
}

template <typename T>
inline void write_once(void* dest, const T& val) {
    write_once(dest, &val, sizeof(val));
}

template <typename T, typename T2 = T, typename T3 = T>
bool atomic_cmpxchg(T* ptr, T2 cmp, T3 val) {
#ifndef _MSC_VER
    const int mo = __ATOMIC_SEQ_CST;
    return __atomic_compare_exchange(ptr, (T*)&cmp, (T*)&val, false, mo, mo);
#else
    return _InterlockedCompareExchange(ptr, val, cmp);
#endif
}

template <typename T, typename T2 = T>
inline T atomic_or(T* mem, T2 val) {
    const int mo = __ATOMIC_SEQ_CST;
    return __atomic_fetch_or(mem, (T)val, mo);
}

template <typename T, typename T2 = T>
inline T atomic_xor(T* mem, T2 val) {
    const int mo = __ATOMIC_SEQ_CST;
    return __atomic_fetch_xor(mem, (T)val, mo);
}

template <typename T, typename T2 = T>
inline T atomic_and(T* mem, T2 val) {
    const int mo = __ATOMIC_SEQ_CST;
    return __atomic_fetch_and(mem, (T)val, mo);
}

template <typename T, typename T2 = T>
static T atomic_add(T* mem, T2 val) {
    const int mo = __ATOMIC_SEQ_CST;
    return __atomic_fetch_add(mem, (T)val, mo);
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
    const int mo = __ATOMIC_SEQ_CST;
    return __atomic_exchange_n(mem, (T2)val, mo);
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

inline bool atomic_cmpxchg(void* ptr, u64 cmp, const void* val, size_t size) {
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
