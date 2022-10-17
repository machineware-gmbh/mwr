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

#ifndef MWR_TYPES_H
#define MWR_TYPES_H

#include <stdlib.h>
#include <stdint.h>
#include <limits>
#include <time.h>

#include <algorithm>

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

enum i8_limits : i8 {
    I8_MIN = std::numeric_limits<i8>::min(),
    I8_MAX = std::numeric_limits<i8>::max(),
};

enum i16_limits : i16 {
    I16_MIN = std::numeric_limits<i16>::min(),
    I16_MAX = std::numeric_limits<i16>::max(),
};

enum i32_limits : i32 {
    I32_MIN = std::numeric_limits<i32>::min(),
    I32_MAX = std::numeric_limits<i32>::max(),
};

enum i64_limits : i64 {
    I64_MIN = std::numeric_limits<i64>::min(),
    I64_MAX = std::numeric_limits<i64>::max(),
};

enum u8_limits : u8 {
    U8_MIN = std::numeric_limits<u8>::min(),
    U8_MAX = std::numeric_limits<u8>::max(),
};

enum u16_limits : u16 {
    U16_MIN = std::numeric_limits<u16>::min(),
    U16_MAX = std::numeric_limits<u16>::max(),
};

enum u32_limits : u32 {
    U32_MIN = std::numeric_limits<u32>::min(),
    U32_MAX = std::numeric_limits<u32>::max(),
};

enum u64_limits : u64 {
    U64_MIN = std::numeric_limits<u64>::min(),
    U64_MAX = std::numeric_limits<u64>::max(),
};

using std::min;
using std::max;

using ::clock_t;

const clock_t Hz = 1;           // NOLINT(readability-identifier-naming)
const clock_t kHz = 1000 * Hz;  // NOLINT(readability-identifier-naming)
const clock_t MHz = 1000 * kHz; // NOLINT(readability-identifier-naming)
const clock_t GHz = 1000 * MHz; // NOLINT(readability-identifier-naming)
const clock_t THz = 1000 * GHz; // NOLINT(readability-identifier-naming)

using std::size_t;

const size_t KiB = 1024;       // NOLINT(readability-identifier-naming)
const size_t MiB = 1024 * KiB; // NOLINT(readability-identifier-naming)
const size_t GiB = 1024 * MiB; // NOLINT(readability-identifier-naming)
const size_t TiB = 1024 * GiB; // NOLINT(readability-identifier-naming)

typedef std::size_t id_t;

} // namespace mwr

#endif
