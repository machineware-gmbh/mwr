/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_UIMAGE_H
#define MWR_UTILS_UIMAGE_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"
#include "mwr/core/bitops.h"

#include "mwr/stl/strings.h"
#include "mwr/stl/containers.h"

namespace mwr {

class uimage
{
public:
    enum uimage_os : u8 {
        OS_INVALID = 0,
        OS_OPENBSD,
        OS_NETBSD,
        OS_FREEBSD,
        OS_4_4BSD,
        OS_LINUX,
        OS_SVR4,
        OS_ESIX,
        OS_SOLARIS,
        OS_IRIX,
        OS_SCO,
        OS_DELL,
        OS_NCR,
        OS_LYNXOS,
        OS_VXWORKS,
        OS_PSOS,
        OS_QNX,
        OS_U_BOOT,
        OS_RTEMS,
        OS_ARTOS,
        OS_UNITY,
        OS_INTEGRITY,
        OS_OSE,
        OS_PLAN9,
        OS_OPENRTOS,
        OS_ARM_TRUSTED_FIRMWARE,
        OS_TEE,
        OS_OPENSBI,
        OS_EFI,
    };

    enum uimage_arch : u8 {
        ARCH_INVALID = 0,
        ARCH_ALPHA,
        ARCH_ARM,
        ARCH_I386,
        ARCH_IA64,
        ARCH_MIPS,
        ARCH_MIPS64,
        ARCH_PPC,
        ARCH_S390,
        ARCH_SH,
        ARCH_SPARC,
        ARCH_SPARC64,
        ARCH_M68K,
        ARCH_NIOS,
        ARCH_MICROBLAZE,
        ARCH_NIOS2,
        ARCH_BLACKFIN,
        ARCH_AVR32,
        ARCH_ST200,
        ARCH_SANDBOX,
        ARCH_NDS32,
        ARCH_OPENRISC,
        ARCH_ARM64,
        ARCH_ARC,
        ARCH_X86_64,
        ARCH_XTENSA,
        ARCH_RISCV,
    };

    enum uimage_type : u8 {
        TYPE_INVALID = 0,
        TYPE_STANDALONE,
        TYPE_KERNEL,
        TYPE_RAMDISK,
        TYPE_MULTI,
        TYPE_FIRMWARE,
        TYPE_SCRIPT,
        TYPE_FILESYSTEM,
        TYPE_FLATDT,
        TYPE_KWBIMAGE,
        TYPE_IMXIMAGE,
        TYPE_UBLIMAGE,
        TYPE_OMAPIMAGE,
        TYPE_AISIMAGE,
        TYPE_KERNEL_NOLOAD,
        TYPE_PBLIMAGE,
        TYPE_MXSIMAGE,
        TYPE_GPIMAGE,
        TYPE_ATMELIMAGE,
        TYPE_SOCFPGAIMAGE,
        TYPE_X86_SETUP,
        TYPE_LPC32XXIMAGE,
        TYPE_LOADABLE,
        TYPE_RKIMAGE,
        TYPE_RKSD,
        TYPE_RKSPI,
        TYPE_ZYNQIMAGE,
        TYPE_ZYNQMPIMAGE,
        TYPE_ZYNQMPBIF,
        TYPE_FPGA,
        TYPE_VYBRIDIMAGE,
        TYPE_TEE,
        TYPE_FIRMWARE_IVT,
        TYPE_PMMC,
        TYPE_STM32IMAGE,
        TYPE_SOCFPGAIMAGE_V1,
        TYPE_MTKIMAGE,
        TYPE_IMX8MIMAGE,
        TYPE_IMX8IMAGE,
        TYPE_COPRO,
        TYPE_SUNXI_EGON,
        TYPE_SUNXI_TOC0,
        TYPE_FDT_LEGACY,
        TYPE_RENESAS_SPKG,
    };

    enum uimage_comp : u8 {
        COMP_NONE = 0,
        COMP_GZIP,
        COMP_BZIP2,
        COMP_LZMA,
        COMP_LZO,
        COMP_LZ4,
        COMP_ZSTD,
    };

    const char* name() const { return m_name.c_str(); }
    const char* path() const { return m_path.c_str(); }

    size_t size() const { return m_size; }
    time_t date() const { return m_date; }

    uimage_os os() const { return m_os; }
    uimage_arch arch() const { return m_arch; }
    uimage_type type() const { return m_type; }
    uimage_comp comp() const { return m_comp; }

    bool is_compressed() const { return m_comp > COMP_NONE; }

    u32 load() const { return m_load; }
    u32 entry() const { return m_entry; }

    uimage(const string& filename);
    uimage(const uimage&) = delete;
    uimage(uimage&&) noexcept = default;
    ~uimage() = default;

    size_t read(u8* dest, size_t size, size_t offset = 0);

private:
    string m_path;
    string m_name;
    size_t m_size;
    time_t m_date;

    uimage_os m_os;
    uimage_arch m_arch;
    uimage_type m_type;
    uimage_comp m_comp;

    u32 m_load;
    u32 m_entry;

    ifstream m_file;

    u8 read8();
    u32 read32be();
    string readstr();
};

const char* uimage_os_to_str(uimage::uimage_os os);
const char* uimage_arch_to_str(uimage::uimage_arch arch);
const char* uimage_type_to_str(uimage::uimage_type type);
const char* uimage_comp_to_str(uimage::uimage_comp comp);

} // namespace mwr

#endif
