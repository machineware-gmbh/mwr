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
bool host_has_sve();     // scalable vector extension
bool host_has_aes();     // aes encryption
bool host_has_pmull();   // polynomial multiplication
bool host_has_sha1();    // sha1 hashing
bool host_has_sha2();    // sha224 and sha256 hashing
bool host_has_sha512();  // sha384 and sha512 hashing
bool host_has_sha3();    // sha3 hashing
bool host_has_sm3();     // sm3 hashing
bool host_has_sm4();     // sm4 encryption
bool host_has_crc32();   // crc32 checksums
bool host_has_lse();     // large system extensions (atomics)
bool host_has_rdm();     // rounding double multiply accumulate
bool host_has_dotprod(); // integer dot product
bool host_has_fhm();     // half precision multiply accumulate
bool host_has_flagm();   // condition flag manipulation
bool host_has_flagm2();  // condition flag manipulation v2
bool host_has_rng();     // random number generation

// riscv64 host features
bool host_has_rvc();    // compressed instructions (C)
bool host_has_rvv();    // scalable vector extension (V)
bool host_has_zba();    // address generation (shift-and-add)
bool host_has_zbb();    // basic bit manipulation
bool host_has_zbs();    // single-bit instructions
bool host_has_zicond(); // integer conditional operations

// common host features
bool host_has_fp16(); // half precision arithmetic
bool host_has_bf16(); // brainfloat conversion and dot product

void host_info_dump(std::ostream& os);

} // namespace mwr

#endif
