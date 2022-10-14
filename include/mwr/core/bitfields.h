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

#ifndef MWR_CORE_BITFIELDS_H
#define MWR_CORE_BITFIELDS_H

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
    assert(start + w <= width_of(val) && "bit range exceeded");
    return ((RET)val << (width_of(val) - w - start)) >> (width_of(val) - w);
}

template <typename T, typename T2>
constexpr T deposit(T val, size_t start, size_t width, T2 in) {
    assert(start + width <= width_of(val) && "bit range exceeded");
    const T mask = bitmask(width, start);
    return (val & ~mask) | (((T)in << start) & mask);
}

constexpr i64 signext(u64 val, size_t width) {
    return sextract(val, 0, width);
}

template <size_t OFF, size_t LEN, typename T = u32>
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

} // namespace mwr

#endif
