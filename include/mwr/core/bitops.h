/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_BITOPS_H
#define MWR_BITOPS_H

#include "mwr/core/compiler.h"
#include "mwr/core/types.h"

#ifdef MWR_MSVC
#include <intrin.h>
#endif

#include <limits.h>
#include <type_traits>

namespace mwr {

template <typename T>
constexpr size_t width_of() {
    return sizeof(T) * CHAR_BIT;
};

template <typename T>
constexpr size_t width_of(const T& val) {
    return sizeof(val) * CHAR_BIT;
};

template <typename T>
constexpr size_t popcnt(const T& val) {
#ifndef MWR_MSVC
    return __builtin_popcountll((unsigned long long)val);
#else
    return __popcnt64((unsigned long long)val);
#endif
}

template <typename T>
constexpr size_t parity(const T& val) {
#ifndef MWR_MSVC
    return __builtin_parityll((unsigned long long)val);
#else
    return popcnt(val) & 1ull;
#endif
}

template <typename T>
constexpr bool parity_odd(const T& val) {
    return parity(val) == 1u;
}

template <typename T>
constexpr bool parity_even(const T& val) {
    return parity(val) == 0u;
}

template <typename T>
constexpr bool is_aligned(T addr, size_t size) {
    return ((size_t)addr % size) == 0;
}

template <typename T>
constexpr size_t ctz(const T& val) { // count trailing zeroes
#ifndef MWR_MSVC
    return val ? __builtin_ctzll(val) : width_of(val);
#else
    return val ? _tzcnt_u64(val) : width_of(val);
#endif
}

template <typename T>
constexpr size_t clz(const T& val) { // count leading zeroes
#ifndef MWR_MSVC
    return val ? __builtin_clzll(val) - (64 - width_of(val)) : width_of(val);
#else
    return val ? __lzcnt64(val) - (64 - width_of(val)) : width_of(val);
#endif
}

template <typename T>
constexpr int ffs(const T& val) {
#ifndef MWR_MSVC
    return __builtin_ffsll(val) - 1;
#else
    return val ? (int)ctz(val) : -1;
#endif
}

template <typename T>
constexpr int fls(T val) {
    return (int)width_of(val) - (int)clz(val) - 1;
}

template <typename T>
constexpr T rol(T val, size_t n) {
    n &= width_of(val) - 1; // Restrict rotate amount to [0, width_of(val) - 1]
    return (val << n) | (val >> (width_of(val) - n));
}

template <typename T>
constexpr T ror(T val, size_t n) {
    n &= width_of(val) - 1; // Restrict rotate amount to [0, width_of(val) - 1]
    return (val >> n) | (val << (width_of(val) - n));
}

template <typename T>
constexpr bool is_pow2(const T& val) {
    if (val == 0)
        return false;
    return (val & (val - 1)) == 0;
}

template <typename T>
constexpr int log2i(T val) {
    return (int)ctz(val); // value must be power of two
}

constexpr u64 bit(size_t offset) {
    return 1ull << offset;
}

constexpr u64 bitmask(size_t width, size_t offset = 0) {
    if (offset >= 64)
        return 0ull;
    if (width >= 64)
        return ~0ull << offset;
    return ((1ull << width) - 1) << offset;
}

constexpr u32 fourcc(const char* s) {
    return s[0] | s[1] << 8 | s[2] << 16 | s[3] << 24;
}

template <typename T1, typename T2>
constexpr bool fits(T1 val) {
    return val == (T1)((T2)val);
}

template <typename T>
constexpr bool fits_i8(T val) {
    return fits<T, i8>(val);
}

template <typename T>
constexpr bool fits_i16(T val) {
    return fits<T, i16>(val);
}

template <typename T>
constexpr bool fits_i32(T val) {
    return fits<T, i32>(val);
}

template <typename T>
constexpr bool fits_i64(T val) {
    return fits<T, i64>(val);
}

template <typename T>
constexpr bool fits_u8(T val) {
    return fits<T, u8>(val);
}

template <typename T>
constexpr bool fits_u16(T val) {
    return fits<T, u16>(val);
}

template <typename T>
constexpr bool fits_u32(T val) {
    return fits<T, u32>(val);
}

template <typename T>
constexpr bool fits_u64(T val) {
    return fits<T, u64>(val);
}

template <bool IS_SIGNED>
struct encode_traits {
    typedef i8 t8;
    typedef i16 t16;
    typedef i32 t32;
    typedef i64 t64;
};

template <>
struct encode_traits<false> {
    typedef u8 t8;
    typedef u16 t16;
    typedef u32 t32;
    typedef u64 t64;
};

template <typename T>
constexpr int encode_size(T val) {
    typedef encode_traits<std::is_signed<T>::value> traits;
    if (fits<T, typename traits::t8>(val))
        return 8;
    if (fits<T, typename traits::t16>(val))
        return 16;
    if (fits<T, typename traits::t32>(val))
        return 32;
    if (fits<T, typename traits::t64>(val))
        return 64;
    return (int)width_of(val);
}

extern const u8 BITREV_TABLE[256];
inline u8 bitrev(u8 val) {
    return BITREV_TABLE[val];
}

inline u16 bitrev(u16 val) {
    return bitrev((u8)val) << 8 | bitrev((u8)(val >> 8));
}

inline u32 bitrev(u32 val) {
    return bitrev((u16)val) << 16 | bitrev((u16)(val >> 16));
}

inline u64 bitrev(u64 val) {
    return (u64)bitrev((u32)val) << 32 | bitrev((u32)(val >> 32));
}

constexpr u8 bswap(u8 val) {
    return val;
}

constexpr u16 bswap(u16 val) {
    return ((val & 0xff00) >> 8) | ((val & 0x00ff) << 8);
}

constexpr u32 bswap(u32 val) {
    return ((val & 0xff000000) >> 24) | ((val & 0x00ff0000) >> 8) |
           ((val & 0x0000ff00) << 8) | ((val & 0x000000ff) << 24);
}

constexpr u64 bswap(u64 val) {
    return ((val & 0xff00000000000000ull) >> 56) |
           ((val & 0x00ff000000000000ull) >> 40) |
           ((val & 0x0000ff0000000000ull) >> 24) |
           ((val & 0x000000ff00000000ull) >> 8) |
           ((val & 0x00000000ff000000ull) << 8) |
           ((val & 0x0000000000ff0000ull) << 24) |
           ((val & 0x000000000000ff00ull) << 40) |
           ((val & 0x00000000000000ffull) << 56);
}

inline void memswap(void* ptr, unsigned int size) {
    u8* v = static_cast<u8*>(ptr);
    for (unsigned int i = 0; i < size / 2; i++) {
        u8 tmp = v[i];

        v[i] = v[size - i - 1];

        v[size - i - 1] = tmp;
    }
}

inline u32 f32_raw(f32 val) {
    union {
        f32 f;
        u32 u;
    } helper;
    helper.f = val;
    return helper.u;
}

inline u64 f64_raw(f64 val) {
    union {
        f64 f;
        u64 u;
    } helper;
    helper.f = val;
    return helper.u;
}

// crc7 calculates a 7 bit CRC of the specified data using the polynomial
// x^7 + x^3 + 1. It will be stored in the upper 7 bits of the result.
u8 crc7(const u8* buffer, size_t len, u8 crc = 0);

// crc16 calculates a 16 bit CRC of the given data using the polynomial
// x^16 + x^12 + x^5 + 1.
u16 crc16(const u8* buffer, size_t len, u16 crc = 0);

// crc32 calculates a 32 bit CRC of the given data using the polynomial
// x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+1
u32 crc32(const u8* buffer, size_t len, u32 crc = ~0u);

} // namespace mwr

#endif
