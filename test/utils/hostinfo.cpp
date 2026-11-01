/******************************************************************************
 *                                                                            *
 * Copyright (C) 2026 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file found  *
 * in the root directory of this source tree.                                 *
 *                                                                            *
 ******************************************************************************/

#include "testing.h"

#include "mwr.h"

using namespace mwr;

static bool disabled(const char* feature) {
    return mwr::getenv_or_default(string("MWR_NO_") + feature, false);
}

TEST(hostinfo, overrides) {
    EXPECT_FALSE(disabled("BMI1") && host_has_bmi1());
    EXPECT_FALSE(disabled("BMI2") && host_has_bmi2());
    EXPECT_FALSE(disabled("CMOV") && host_has_cmov());
    EXPECT_FALSE(disabled("MOVBE") && host_has_movbe());
    EXPECT_FALSE(disabled("POPCNT") && host_has_popcnt());
    EXPECT_FALSE(disabled("LZCNT") && host_has_lzcnt());
    EXPECT_FALSE(disabled("AVX") && host_has_avx());
    EXPECT_FALSE(disabled("AVX2") && host_has_avx2());
    EXPECT_FALSE(disabled("FMA") && host_has_fma());
    EXPECT_FALSE(disabled("F16C") && host_has_f16c());
    EXPECT_FALSE(disabled("AVX512") && host_has_avx512());
    EXPECT_FALSE(disabled("SVE") && host_has_sve());
    EXPECT_FALSE(disabled("FP16") && host_has_fp16());
    EXPECT_FALSE(disabled("BF16") && host_has_bf16());
}

TEST(hostinfo, dependencies) {
    EXPECT_TRUE(!host_has_avx2() || host_has_avx());
    EXPECT_TRUE(!host_has_fma() || host_has_avx());
    EXPECT_TRUE(!host_has_f16c() || host_has_avx());
    EXPECT_TRUE(!host_has_avx512() || host_has_avx2());
#ifdef MWR_X86_64
    EXPECT_TRUE(!host_has_fp16() || host_has_avx512());
    EXPECT_TRUE(!host_has_bf16() || host_has_avx512());
#endif
}

TEST(hostinfo, arch) {
#ifdef MWR_X86_64
    EXPECT_FALSE(host_has_sve());
    EXPECT_TRUE(disabled("CMOV") || host_has_cmov()); // all x86_64 have cmov
#else
    EXPECT_FALSE(host_has_bmi1());
    EXPECT_FALSE(host_has_bmi2());
    EXPECT_FALSE(host_has_cmov());
    EXPECT_FALSE(host_has_movbe());
    EXPECT_FALSE(host_has_popcnt());
    EXPECT_FALSE(host_has_lzcnt());
    EXPECT_FALSE(host_has_avx());
    EXPECT_FALSE(host_has_avx2());
    EXPECT_FALSE(host_has_fma());
    EXPECT_FALSE(host_has_f16c());
    EXPECT_FALSE(host_has_avx512());
#endif
}

TEST(hostinfo, dump) {
    std::stringstream ss;
    host_info_dump(ss);
    string s = ss.str();
#ifdef MWR_X86_64
    EXPECT_NE(s.find("cmov:"), string::npos);
    EXPECT_NE(s.find("avx512:"), string::npos);
    EXPECT_EQ(s.find("sve:"), string::npos);
#endif
#ifdef MWR_ARM64
    EXPECT_NE(s.find("sve:"), string::npos);
    EXPECT_EQ(s.find("cmov:"), string::npos);
#endif
    EXPECT_NE(s.find("fp16:"), string::npos);
    EXPECT_NE(s.find("bf16:"), string::npos);
    host_info_dump(std::cout);
}
