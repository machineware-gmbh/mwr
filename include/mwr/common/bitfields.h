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

#ifndef MWR_COMMON_BITFIELDS_H
#define MWR_COMMON_BITFIELDS_H

#include "mwr/common/types.h"
#include "mwr/common/bitops.h"

namespace mwr {

template <typename T>
constexpr T extract(T val, size_t off, size_t len) {
    return (val >> off) & ((1ull << len) - 1);
}

template <typename T, typename T2>
constexpr void insert(T& val, size_t off, size_t len, T2 x) {
    const T mask = bitmask(len, off);
    val = (val & ~mask) | (((T)x << off) & mask);
}

template <typename T, typename T2>
constexpr T deposit(T val, size_t off, size_t len, T2 x) {
    const T mask = ((1ull << len) - 1) << off;
    return (val & ~mask) | (((T)x << off) & mask);
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
    insert(val, F::OFFSET, F::LENGTH, ~0ull);
}

template <typename F>
constexpr void set_field(typename F::base& val, typename F::base x) {
    insert(val, F::OFFSET, F::LENGTH, x);
}

} // namespace vcml

#endif
