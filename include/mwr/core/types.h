/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_TYPES_H
#define MWR_TYPES_H

#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <limits>
#include <algorithm>

#ifdef MWR_MSVC
#include <intrin.h>
#endif

namespace mwr {

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

static_assert(sizeof(i8) == 1, "i8 invalid size");
static_assert(sizeof(i16) == 2, "i16 invalid size");
static_assert(sizeof(i32) == 4, "i32 invalid size");
static_assert(sizeof(i64) == 8, "i64 invalid size");

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

static_assert(sizeof(u8) == 1, "u8 invalid size");
static_assert(sizeof(u16) == 2, "u16 invalid size");
static_assert(sizeof(u32) == 4, "u32 invalid size");
static_assert(sizeof(u64) == 8, "u64 invalid size");

typedef float f32;
typedef double f64;

static_assert(sizeof(f32) == 4, "f32 invalid size");
static_assert(sizeof(f64) == 8, "f64 invalid size");

constexpr i8 I8_MIN = std::numeric_limits<i8>::min();
constexpr i8 I8_MAX = std::numeric_limits<i8>::max();

constexpr u8 U8_MIN = std::numeric_limits<u8>::min();
constexpr u8 U8_MAX = std::numeric_limits<u8>::max();

constexpr i16 I16_MIN = std::numeric_limits<i16>::min();
constexpr i16 I16_MAX = std::numeric_limits<i16>::max();

constexpr u16 U16_MIN = std::numeric_limits<u16>::min();
constexpr u16 U16_MAX = std::numeric_limits<u16>::max();

constexpr i32 I32_MIN = std::numeric_limits<i32>::min();
constexpr i32 I32_MAX = std::numeric_limits<i32>::max();

constexpr u32 U32_MIN = std::numeric_limits<u32>::min();
constexpr u32 U32_MAX = std::numeric_limits<u32>::max();

constexpr i64 I64_MIN = std::numeric_limits<i64>::min();
constexpr i64 I64_MAX = std::numeric_limits<i64>::max();

constexpr u64 U64_MIN = std::numeric_limits<u64>::min();
constexpr u64 U64_MAX = std::numeric_limits<u64>::max();

template <typename T>
struct widen;

template <>
struct widen<u8> {
    typedef u16 type;
};

template <>
struct widen<i8> {
    typedef i16 type;
};

template <>
struct widen<u16> {
    typedef u32 type;
};

template <>
struct widen<i16> {
    typedef i32 type;
};

template <>
struct widen<u32> {
    typedef u64 type;
};

template <>
struct widen<i32> {
    typedef i64 type;
};

template <>
struct widen<u64> {
    typedef u64 type;
};

template <>
struct widen<i64> {
    typedef i64 type;
};

template <>
struct widen<f32> {
    typedef f64 type;
};

template <>
struct widen<f64> {
    typedef f64 type;
};

template <typename T>
struct narrow;

template <>
struct narrow<u8> {
    typedef u8 type;
};

template <>
struct narrow<i8> {
    typedef i8 type;
};

template <>
struct narrow<u16> {
    typedef u8 type;
};

template <>
struct narrow<i16> {
    typedef i8 type;
};

template <>
struct narrow<u32> {
    typedef u16 type;
};

template <>
struct narrow<i32> {
    typedef i16 type;
};

template <>
struct narrow<u64> {
    typedef u32 type;
};

template <>
struct narrow<i64> {
    typedef i32 type;
};

template <>
struct narrow<f32> {
    typedef f32 type;
};

template <>
struct narrow<f64> {
    typedef f32 type;
};

using std::min;
using std::max;

using std::size_t;

const size_t KiB = 1024;       // NOLINT(readability-identifier-naming)
const size_t MiB = 1024 * KiB; // NOLINT(readability-identifier-naming)
const size_t GiB = 1024 * MiB; // NOLINT(readability-identifier-naming)
const size_t TiB = 1024 * GiB; // NOLINT(readability-identifier-naming)

using hz_t = size_t;

const hz_t Hz = 1;           // NOLINT(readability-identifier-naming)
const hz_t kHz = 1000 * Hz;  // NOLINT(readability-identifier-naming)
const hz_t MHz = 1000 * kHz; // NOLINT(readability-identifier-naming)
const hz_t GHz = 1000 * MHz; // NOLINT(readability-identifier-naming)
const hz_t THz = 1000 * GHz; // NOLINT(readability-identifier-naming)

} // namespace mwr

#endif
