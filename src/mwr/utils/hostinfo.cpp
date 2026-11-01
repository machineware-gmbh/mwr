/******************************************************************************
 *                                                                            *
 * Copyright (C) 2026 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file found  *
 * in the root directory of this source tree.                                 *
 *                                                                            *
 ******************************************************************************/

#include "mwr/utils/hostinfo.h"
#include "mwr/core/bitops.h"
#include "mwr/core/utils.h"

#if defined(MWR_X86_64) && defined(MWR_MSVC)
#include <intrin.h>
#elif defined(MWR_X86_64)
#include <cpuid.h>
#endif

#if defined(MWR_ARM64) && defined(MWR_LINUX)
#include <sys/auxv.h>
#include <asm/hwcap.h>
#elif defined(MWR_ARM64) && defined(MWR_MACOS)
#include <sys/sysctl.h>
#elif defined(MWR_ARM64) && defined(MWR_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace mwr {

struct hostinfo {
    bool has_bmi1;
    bool has_bmi2;
    bool has_cmov;
    bool has_movbe;
    bool has_popcnt;
    bool has_lzcnt;
    bool has_avx;
    bool has_avx2;
    bool has_fma;
    bool has_f16c;
    bool has_avx512;
    bool has_sve;
    bool has_fp16;
    bool has_bf16;

    hostinfo();

    static hostinfo& instance() {
        static hostinfo singleton;
        return singleton;
    }
};

#ifdef MWR_X86_64
static void host_cpuid(u32 leaf, u32 subleaf, u32 regs[4]) {
#if defined(MWR_MSVC)
    int info[4];
    __cpuidex(info, (int)leaf, (int)subleaf);
    for (int i = 0; i < 4; i++)
        regs[i] = (u32)info[i];
#else
    __cpuid_count(leaf, subleaf, regs[0], regs[1], regs[2], regs[3]);
#endif
}

static u64 host_xgetbv(u32 xcr) {
#if defined(MWR_MSVC)
    return _xgetbv(xcr);
#else
    // inline asm to avoid requiring -mxsave for the _xgetbv intrinsic
    u32 lo, hi;
    asm volatile("xgetbv" : "=a"(lo), "=d"(hi) : "c"(xcr));
    return (u64)hi << 32 | lo;
#endif
}

static void init_hostinfo_x86_64(hostinfo& hi) {
    enum : u32 { EAX = 0, EBX = 1, ECX = 2, EDX = 3 };

    u32 regs[4];
    host_cpuid(0, 0, regs);
    u32 max = regs[EAX];

    bool os_avx = false;
    bool os_avx512 = false;

    if (max >= 1) {
        host_cpuid(1, 0, regs);
        hi.has_cmov = regs[EDX] & bit(15);
        hi.has_movbe = regs[ECX] & bit(22);
        hi.has_popcnt = regs[ECX] & bit(23);

        bool osxsave = regs[ECX] & bit(27);
        if (osxsave) {
            u64 xcr0 = host_xgetbv(0);
            os_avx = (xcr0 & 0x06) == 0x06;
            os_avx512 = (xcr0 & 0xe6) == 0xe6;
        }

        hi.has_avx = os_avx && (regs[ECX] & bit(28));
        hi.has_fma = hi.has_avx && (regs[ECX] & bit(12));
        hi.has_f16c = hi.has_avx && (regs[ECX] & bit(29));
    }

    if (max >= 7) {
        host_cpuid(7, 0, regs);
        hi.has_bmi1 = regs[EBX] & bit(3);
        hi.has_bmi2 = regs[EBX] & bit(8);
        hi.has_avx2 = hi.has_avx && (regs[EBX] & bit(5));

        // require F, DQ, CD, BW and VL (i.e. x86-64-v4)
        constexpr u64 avx512_mask = bit(16) | bit(17) | bit(28) | bit(30) |
                                    bit(31);
        hi.has_avx512 = os_avx512 && (regs[EBX] & avx512_mask) == avx512_mask;
        hi.has_fp16 = hi.has_avx512 && (regs[EDX] & bit(23)); // avx512-fp16

        if (regs[EAX] >= 1) {
            host_cpuid(7, 1, regs);
            hi.has_bf16 = hi.has_avx512 && (regs[EAX] & bit(5)); // avx512-bf16
        }
    }

    host_cpuid(0x80000000, 0, regs);
    u32 ext = regs[EAX];
    if (ext >= 0x80000001) {
        host_cpuid(0x80000001, 0, regs);
        hi.has_lzcnt = regs[ECX] & bit(5);
    }
}
#endif

#ifdef MWR_ARM64
#if defined(MWR_MACOS)
static bool host_sysctl(const char* name) {
    int val = 0;
    size_t len = sizeof(val);
    return sysctlbyname(name, &val, &len, nullptr, 0) == 0 && val != 0;
}
#endif

static void init_hostinfo_arm64(hostinfo& hi) {
#if defined(MWR_LINUX)
    unsigned long hwcap = getauxval(AT_HWCAP);
    unsigned long hwcap2 = getauxval(AT_HWCAP2);
#if defined(HWCAP_SVE)
    hi.has_sve = hwcap & HWCAP_SVE;
#endif
#if defined(HWCAP_FPHP) && defined(HWCAP_ASIMDHP)
    hi.has_fp16 = (hwcap & HWCAP_FPHP) && (hwcap & HWCAP_ASIMDHP);
#endif
#if defined(HWCAP2_BF16)
    hi.has_bf16 = hwcap2 & HWCAP2_BF16;
#endif
    (void)hwcap2;
#elif defined(MWR_MACOS)
    // no SVE support on macOS
    hi.has_fp16 = host_sysctl("hw.optional.arm.FEAT_FP16");
    hi.has_bf16 = host_sysctl("hw.optional.arm.FEAT_BF16");
#elif defined(MWR_WINDOWS)
#if defined(PF_ARM_SVE_INSTRUCTIONS_AVAILABLE)
    hi.has_sve = IsProcessorFeaturePresent(PF_ARM_SVE_INSTRUCTIONS_AVAILABLE);
#endif
#if defined(PF_ARM_V82_FP16_INSTRUCTIONS_AVAILABLE)
    hi.has_fp16 = IsProcessorFeaturePresent(
        PF_ARM_V82_FP16_INSTRUCTIONS_AVAILABLE);
#endif
#if defined(PF_ARM_V86_BF16_INSTRUCTIONS_AVAILABLE)
    hi.has_bf16 = IsProcessorFeaturePresent(
        PF_ARM_V86_BF16_INSTRUCTIONS_AVAILABLE);
#endif
    (void)hi;
#else
    (void)hi;
#endif
}
#endif

hostinfo::hostinfo():
    has_bmi1(false),
    has_bmi2(false),
    has_cmov(false),
    has_movbe(false),
    has_popcnt(false),
    has_lzcnt(false),
    has_avx(false),
    has_avx2(false),
    has_fma(false),
    has_f16c(false),
    has_avx512(false),
    has_sve(false),
    has_fp16(false),
    has_bf16(false) {
#ifdef MWR_X86_64
    init_hostinfo_x86_64(*this);
#endif

#ifdef MWR_ARM64
    init_hostinfo_arm64(*this);
#endif

    has_bmi1 &= !getenv_or_default("MWR_NO_BMI1", false);
    has_bmi2 &= !getenv_or_default("MWR_NO_BMI2", false);
    has_cmov &= !getenv_or_default("MWR_NO_CMOV", false);
    has_movbe &= !getenv_or_default("MWR_NO_MOVBE", false);
    has_popcnt &= !getenv_or_default("MWR_NO_POPCNT", false);
    has_lzcnt &= !getenv_or_default("MWR_NO_LZCNT", false);
    has_avx &= !getenv_or_default("MWR_NO_AVX", false);
    has_avx2 &= has_avx && !getenv_or_default("MWR_NO_AVX2", false);
    has_fma &= has_avx && !getenv_or_default("MWR_NO_FMA", false);
    has_f16c &= has_avx && !getenv_or_default("MWR_NO_F16C", false);
    has_avx512 &= has_avx2 && !getenv_or_default("MWR_NO_AVX512", false);
    has_sve &= !getenv_or_default("MWR_NO_SVE", false);
    has_fp16 &= !getenv_or_default("MWR_NO_FP16", false);
    has_bf16 &= !getenv_or_default("MWR_NO_BF16", false);

#ifdef MWR_X86_64
    // x86 fp16 and bf16 require avx512 (evex encoding)
    has_fp16 &= has_avx512;
    has_bf16 &= has_avx512;
#endif
}

bool host_has_bmi1() {
    return hostinfo::instance().has_bmi1;
}

bool host_has_bmi2() {
    return hostinfo::instance().has_bmi2;
}

bool host_has_cmov() {
    return hostinfo::instance().has_cmov;
}

bool host_has_movbe() {
    return hostinfo::instance().has_movbe;
}

bool host_has_popcnt() {
    return hostinfo::instance().has_popcnt;
}

bool host_has_lzcnt() {
    return hostinfo::instance().has_lzcnt;
}

bool host_has_avx() {
    return hostinfo::instance().has_avx;
}

bool host_has_avx2() {
    return hostinfo::instance().has_avx2;
}

bool host_has_fma() {
    return hostinfo::instance().has_fma;
}

bool host_has_f16c() {
    return hostinfo::instance().has_f16c;
}

bool host_has_avx512() {
    return hostinfo::instance().has_avx512;
}

bool host_has_sve() {
    return hostinfo::instance().has_sve;
}

bool host_has_fp16() {
    return hostinfo::instance().has_fp16;
}

bool host_has_bf16() {
    return hostinfo::instance().has_bf16;
}

constexpr const char* yesno(bool option) {
    return option ? "yes" : "no";
}

void host_info_dump(std::ostream& os) {
    const hostinfo& hi = hostinfo::instance();
#ifdef MWR_X86_64
    os << "bmi1:   " << yesno(hi.has_bmi1) << std::endl;
    os << "bmi2:   " << yesno(hi.has_bmi2) << std::endl;
    os << "cmov:   " << yesno(hi.has_cmov) << std::endl;
    os << "movbe:  " << yesno(hi.has_movbe) << std::endl;
    os << "popcnt: " << yesno(hi.has_popcnt) << std::endl;
    os << "lzcnt:  " << yesno(hi.has_lzcnt) << std::endl;
    os << "avx:    " << yesno(hi.has_avx) << std::endl;
    os << "avx2:   " << yesno(hi.has_avx2) << std::endl;
    os << "fma:    " << yesno(hi.has_fma) << std::endl;
    os << "f16c:   " << yesno(hi.has_f16c) << std::endl;
    os << "avx512: " << yesno(hi.has_avx512) << std::endl;
#endif
#ifdef MWR_ARM64
    os << "sve:    " << yesno(hi.has_sve) << std::endl;
#endif
    os << "fp16:   " << yesno(hi.has_fp16) << std::endl;
    os << "bf16:   " << yesno(hi.has_bf16) << std::endl;
}

} // namespace mwr
