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
    EXPECT_FALSE(disabled("AES") && host_has_aes());
    EXPECT_FALSE(disabled("PMULL") && host_has_pmull());
    EXPECT_FALSE(disabled("SHA1") && host_has_sha1());
    EXPECT_FALSE(disabled("SHA2") && host_has_sha2());
    EXPECT_FALSE(disabled("SHA512") && host_has_sha512());
    EXPECT_FALSE(disabled("SHA3") && host_has_sha3());
    EXPECT_FALSE(disabled("SM3") && host_has_sm3());
    EXPECT_FALSE(disabled("SM4") && host_has_sm4());
    EXPECT_FALSE(disabled("CRC32") && host_has_crc32());
    EXPECT_FALSE(disabled("LSE") && host_has_lse());
    EXPECT_FALSE(disabled("RDM") && host_has_rdm());
    EXPECT_FALSE(disabled("DOTPROD") && host_has_dotprod());
    EXPECT_FALSE(disabled("FHM") && host_has_fhm());
    EXPECT_FALSE(disabled("FLAGM") && host_has_flagm());
    EXPECT_FALSE(disabled("FLAGM2") && host_has_flagm2());
    EXPECT_FALSE(disabled("RNG") && host_has_rng());
    EXPECT_FALSE(disabled("RVC") && host_has_rvc());
    EXPECT_FALSE(disabled("RVV") && host_has_rvv());
    EXPECT_FALSE(disabled("ZBA") && host_has_zba());
    EXPECT_FALSE(disabled("ZBB") && host_has_zbb());
    EXPECT_FALSE(disabled("ZBS") && host_has_zbs());
    EXPECT_FALSE(disabled("ZICOND") && host_has_zicond());
    EXPECT_FALSE(disabled("FP16") && host_has_fp16());
    EXPECT_FALSE(disabled("BF16") && host_has_bf16());
}

TEST(hostinfo, dependencies) {
    EXPECT_TRUE(!host_has_avx2() || host_has_avx());
    EXPECT_TRUE(!host_has_fma() || host_has_avx());
    EXPECT_TRUE(!host_has_f16c() || host_has_avx());
    EXPECT_TRUE(!host_has_avx512() || host_has_avx2());
    EXPECT_TRUE(!host_has_pmull() || host_has_aes());
    EXPECT_TRUE(!host_has_sha512() || host_has_sha2());
    EXPECT_TRUE(!host_has_sha3() || host_has_sha512());
    EXPECT_TRUE(!host_has_flagm2() || host_has_flagm());
#ifdef MWR_X86_64
    EXPECT_TRUE(!host_has_fp16() || host_has_avx512());
    EXPECT_TRUE(!host_has_bf16() || host_has_avx512());
#endif
}

TEST(hostinfo, arch) {
#ifdef MWR_X86_64
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

#ifndef MWR_ARM64
    EXPECT_FALSE(host_has_sve());
    EXPECT_FALSE(host_has_aes());
    EXPECT_FALSE(host_has_pmull());
    EXPECT_FALSE(host_has_sha1());
    EXPECT_FALSE(host_has_sha2());
    EXPECT_FALSE(host_has_sha512());
    EXPECT_FALSE(host_has_sha3());
    EXPECT_FALSE(host_has_sm3());
    EXPECT_FALSE(host_has_sm4());
    EXPECT_FALSE(host_has_crc32());
    EXPECT_FALSE(host_has_lse());
    EXPECT_FALSE(host_has_rdm());
    EXPECT_FALSE(host_has_dotprod());
    EXPECT_FALSE(host_has_fhm());
    EXPECT_FALSE(host_has_flagm());
    EXPECT_FALSE(host_has_flagm2());
    EXPECT_FALSE(host_has_rng());
#endif

#ifndef MWR_RISCV64
    EXPECT_FALSE(host_has_rvc());
    EXPECT_FALSE(host_has_rvv());
    EXPECT_FALSE(host_has_zba());
    EXPECT_FALSE(host_has_zbb());
    EXPECT_FALSE(host_has_zbs());
    EXPECT_FALSE(host_has_zicond());
#endif
}

#if defined(MWR_ARM64) && defined(MWR_LINUX)
// the kernel reports the same features by name in /proc/cpuinfo
TEST(hostinfo, arm64_cpuinfo) {
    std::ifstream cpuinfo("/proc/cpuinfo");
    ASSERT_TRUE(cpuinfo.good());

    string line;
    std::set<string> features;
    while (std::getline(cpuinfo, line)) {
        if (starts_with(line, "Features")) {
            std::istringstream iss(line.substr(line.find(':') + 1));
            for (string feature; iss >> feature;)
                features.insert(feature);
            break;
        }
    }

    ASSERT_FALSE(features.empty());

    // features disabled by the user also disable the ones built on them,
    // which the kernel knows nothing about
    bool overridden = false;

    auto check = [&](const char* name, const char* env, bool actual) {
        overridden |= disabled(env);
        if (!overridden) {
            EXPECT_EQ(actual, features.count(name) > 0) << name;
        } else if (disabled(env)) {
            EXPECT_FALSE(actual) << name;
        }
    };

    check("aes", "AES", host_has_aes());
    check("pmull", "PMULL", host_has_pmull());
    check("sha1", "SHA1", host_has_sha1());
    check("sha2", "SHA2", host_has_sha2());
    check("sha512", "SHA512", host_has_sha512());
    check("sha3", "SHA3", host_has_sha3());
    check("sm3", "SM3", host_has_sm3());
    check("sm4", "SM4", host_has_sm4());
    check("crc32", "CRC32", host_has_crc32());
    check("atomics", "LSE", host_has_lse());
    check("asimdrdm", "RDM", host_has_rdm());
    check("asimddp", "DOTPROD", host_has_dotprod());
    check("asimdfhm", "FHM", host_has_fhm());
    check("flagm", "FLAGM", host_has_flagm());
    check("flagm2", "FLAGM2", host_has_flagm2());
    check("rng", "RNG", host_has_rng());
    check("sve", "SVE", host_has_sve());
}
#endif

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
    EXPECT_NE(s.find("aes:"), string::npos);
    EXPECT_NE(s.find("lse:"), string::npos);
    EXPECT_NE(s.find("rng:"), string::npos);
    EXPECT_EQ(s.find("cmov:"), string::npos);
#endif
#ifdef MWR_RISCV64
    EXPECT_NE(s.find("rvc:"), string::npos);
    EXPECT_NE(s.find("zba:"), string::npos);
    EXPECT_NE(s.find("rvv:"), string::npos);
    EXPECT_EQ(s.find("cmov:"), string::npos);
    EXPECT_EQ(s.find("sve:"), string::npos);
#endif
    EXPECT_NE(s.find("fp16:"), string::npos);
    EXPECT_NE(s.find("bf16:"), string::npos);
    host_info_dump(std::cout);
}
