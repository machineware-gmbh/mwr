/******************************************************************************
 *                                                                            *
 * Copyright 2022 MachineWare GmbH                                            *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *     http://www.apache.org/licenses/LICENSE-2.0                             *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_COMMON_BITOPS_H
#define MWR_COMMON_BITOPS_H

#include "mwr/common/types.h"

namespace mwr {

template <typename T>
constexpr size_t width_of() {
    return sizeof(T) * 8;
};

template <typename T>
constexpr size_t popcnt(const T& val) {
    return __builtin_popcountll((unsigned long long)val);
}

template <typename T>
constexpr size_t parity(const T& val) {
    return __builtin_parityll((unsigned long long)val);
}

template <typename T>
constexpr bool parity_odd(const T& val) {
    return parity(val) == 1u;
}

template <typename T>
constexpr size_t parity_even(const T& val) {
    return parity(val) == 0u;
}

template <typename T>
constexpr bool is_pow2(const T& val) {
    return val != 0 && popcnt(val) == 1u;
}

template <typename T>
constexpr size_t clz(const T& val) { // count leading zeroes
    return val ? __builtin_clzll(val) -
                     (width_of<unsigned long long>() - width_of<T>())
               : width_of<T>();
}

template <typename T>
constexpr size_t ctz(const T& val) { // count trailing zeroes
    return val ? __builtin_ctzll(val) : width_of<T>();
}

template <typename T>
constexpr ssize_t ffs(const T& val) { // find first set
    return __builtin_ffsll(val) - 1;
}

template <typename T>
constexpr int fls(const T& val) { // find last set
    return width_of<T>() - clz(val) - 1;
}

constexpr u64 bit(size_t offset) {
    return 1ull << offset;
}

constexpr u64 bitmask(size_t length, size_t offset = 0) {
    if (offset >= width_of<u64>())
        return 0ull;
    if (length >= width_of<u64>())
        return ~0ull << offset;
    return ((1ull << length) - 1) << offset;
}

constexpr u32 fourcc(const char* s) {
    return s[0] | s[1] << 8 | s[2] << 16 | s[3] << 24;
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
