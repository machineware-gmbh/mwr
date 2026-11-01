/******************************************************************************
 *                                                                            *
 * Copyright (C) 2026 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file found  *
 * in the root directory of this source tree.                                 *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_HOSTINFO_H
#define MWR_UTILS_HOSTINFO_H

#include "mwr/core/compiler.h"
#include "mwr/core/types.h"

#include <iostream>

namespace mwr {

// x86_64 host features
bool host_has_bmi1();   // bit manipulation set 1
bool host_has_bmi2();   // bit manipulation set 2
bool host_has_cmov();   // conditional move
bool host_has_movbe();  // move with byte swap
bool host_has_popcnt(); // population count
bool host_has_lzcnt();  // leading zero count
bool host_has_avx();    // 256-bit floating point vectors
bool host_has_avx2();   // 256-bit integer vectors
bool host_has_fma();    // fused multiply-add
bool host_has_f16c();   // half precision conversion
bool host_has_avx512(); // 512-bit vectors

// arm64 host features
bool host_has_sve(); // scalable vector extension

// common host features
bool host_has_fp16(); // half precision arithmetic
bool host_has_bf16(); // brainfloat conversion and dot product

void host_info_dump(std::ostream& os);

} // namespace mwr

#endif
