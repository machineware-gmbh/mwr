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

#ifndef MWR_MULDIV_H
#define MWR_MULDIV_H

#include "mwr/core/types.h"
#include "mwr/core/bitops.h"

namespace mwr {

constexpr void umul64_slow(u64& hi, u64& lo, u64 a, u64 b) {
    u64 u = (u32)a;
    u64 v = (u32)b;
    u64 t = u * v;
    u64 w3 = (u32)t;
    u64 k = t >> 32;

    a >>= 32;
    t = (a * v) + k;
    k = (u32)t;
    u64 w1 = t >> 32;

    b >>= 32;
    t = u * b + k;
    k = t >> 32;

    hi = a * b + w1 + k;
    lo = (t << 32) + w3;
}

constexpr void umul64(u64& hi, u64& lo, u64 a, u64 b) {
#ifdef __SIZEOF_INT128__
    unsigned __int128 result = (unsigned __int128)a * (unsigned __int128)b;
    hi = result >> 64;
    lo = result >> 0;
#else
    umul64_slow(hi, lo, a, b);
#endif
}

constexpr void imul64_slow(i64& hi, i64& lo, i64 a, i64 b) {
    u64 uhi = 0, ulo = 0;
    umul64(uhi, ulo, a, b);

    if (b < 0)
        uhi -= a;
    if (a < 0)
        uhi -= b;

    hi = uhi;
    lo = ulo;
}

constexpr void imul64(i64& hi, i64& lo, i64 a, i64 b) {
#ifdef __SIZEOF_INT128__
    __int128 result = (__int128)a * (__int128)b;
    hi = result >> 64;
    lo = result >> 0;
#else
    imul64_slow(hi, lo, a, b);
#endif
}

} // namespace mwr

#endif