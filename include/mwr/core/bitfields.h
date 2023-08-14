/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_BITFIELDS_H
#define MWR_BITFIELDS_H

#include <assert.h>

#include "mwr/core/types.h"
#include "mwr/core/bitops.h"

namespace mwr {

template <typename T, typename RET = std::make_unsigned_t<T>>
constexpr RET extract(T val, size_t start, size_t width) {
    assert(start + width <= width_of(val) && "bit range exceeded");
    return ((RET)val >> start) & bitmask(width);
}

template <typename T, typename RET = std::make_signed_t<T>>
constexpr RET sextract(T val, size_t start, size_t w) {
    using U = std::make_unsigned_t<RET>;
    assert(start + w <= width_of(val) && "bit range exceeded");
    return (RET)((U)val << (width_of(val) - w - start)) >> (width_of(val) - w);
}

template <typename T, typename T2>
constexpr T deposit(T val, size_t start, size_t width, T2 in) {
    assert(start + width <= width_of(val) && "bit range exceeded");
    const T mask = (T)bitmask(width, start);
    return (val & ~mask) | (((T)in << start) & mask);
}

constexpr i64 signext(u64 val, size_t width) {
    return sextract(val, 0, width);
}

template <size_t OFF, size_t LEN = 1, typename T = u64>
struct field {
    using base = T;
    enum : size_t { OFFSET = OFF };
    enum : size_t { LENGTH = LEN };
    enum : T { MASK = bitmask(LEN, OFF) };
    constexpr operator T() const noexcept { return MASK; }
    static constexpr T set(T v) noexcept { return (v << OFFSET) & MASK; }
};

template <typename F>
constexpr typename F::base get_field(typename F::base val) {
    return extract(val, F::OFFSET, F::LENGTH);
}

template <typename F>
constexpr void set_field(typename F::base& val) {
    val = deposit(val, F::OFFSET, F::LENGTH, ~0ull);
}

template <typename F>
constexpr void set_field(typename F::base& val, typename F::base x) {
    val = deposit(val, F::OFFSET, F::LENGTH, x);
}

template <typename T>
constexpr bool get_bit(T* bitmask, size_t pos) {
    return bitmask[pos / width_of<T>()] & bit(pos % width_of<T>());
}

template <typename T>
constexpr void set_bit(T* bitmask, size_t pos, bool set) {
    if (set)
        bitmask[pos / width_of<T>()] |= bit(pos % width_of<T>());
    else
        bitmask[pos / width_of<T>()] &= ~bit(pos % width_of<T>());
}

template <const u64 BIT, typename T = u64>
constexpr void set_bit(T& val, bool set) {
    if (set)
        val |= BIT;
    else
        val &= ~BIT;
}

} // namespace mwr

#endif
