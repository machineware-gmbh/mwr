/******************************************************************************
 *                                                                            *
 * Copyright 2023 MachineWare GmbH                                            *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is unpublished proprietary work and may not be used or disclosed to   *
 * third parties, copied or duplicated in any form, in whole or in part,      *
 * without prior written permission of the authors.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_SATURATE_H
#define MWR_SATURATE_H

#include "mwr/core/types.h"
#include "mwr/core/bitops.h"
#include "mwr/core/sfinae.h"

namespace mwr {

template <typename T, typename UT = typename std::make_unsigned<T>::type>
constexpr enable_if_signed_t<T> sat_add(T x, T y, bool& sat) {
    UT ux = x;
    UT uy = y;
    UT res = ux + uy;

    sat = false;
    size_t sh = width_of(ux) - 1;
    ux = (ux >> sh) + std::numeric_limits<T>::max();

    if ((T)((ux ^ uy) | ~(uy ^ res)) >= 0) {
        res = ux;
        sat = true;
    }

    return res;
}

template <typename T, typename UT = typename std::make_unsigned<T>::type>
constexpr enable_if_signed_t<T> sat_add(T x, T y, T z, bool& sat) {
    T a = y;
    T b = z;
    T res;

    if (((y ^ z) & (x ^ z)) < 0) {
        a = z;
        b = y;
    }

    bool sat1, sat2;
    res = sat_add<T>(x, a, sat1);
    res = sat_add<T>(res, b, sat2);
    sat = sat1 || sat2;
    return res;
}

template <typename T>
constexpr enable_if_unsigned_t<T> sat_add(T x, T y, bool& sat) {
    T res = x + y;

    sat = res < x;
    res |= -(res < x);

    return res;
}

template <typename T, typename UT = typename std::make_unsigned<T>::type>
constexpr enable_if_signed_t<T> sat_sub(T x, T y, bool& sat) {
    UT ux = x;
    UT uy = y;
    UT res = ux - uy;

    sat = false;
    size_t sh = width_of(ux) - 1;
    ux = (ux >> sh) + std::numeric_limits<T>::max();

    if ((T)((ux ^ uy) & (ux ^ res)) < 0) {
        res = ux;
        sat = true;
    }

    return res;
}

template <typename T>
constexpr enable_if_unsigned_t<T> sat_sub(T x, T y, bool& sat) {
    T res = x - y;

    sat = !(res <= x);
    res &= -(res <= x);

    return res;
}

} // namespace mwr

#endif
