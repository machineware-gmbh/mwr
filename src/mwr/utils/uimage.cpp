/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/utils/uimage.h"

namespace mwr {

uimage::uimage(const string& filename):
    m_path(filename),
    m_name(),
    m_size(),
    m_os(),
    m_arch(),
    m_type(),
    m_comp(),
    m_load(),
    m_entry(),
    m_file(path(), std::ios::binary) {
    MWR_ERROR_ON(!m_file, "failed to open uimage at %s", path());

    u32 magic = read32be();
    MWR_ERROR_ON(magic != 0x27051956, "not an uimage: %s", path());

    u32 crc = read32be();
    (void)crc;

    m_date = read32be();
    m_size = read32be();
    m_load = read32be();
    m_entry = read32be();

    u32 dcrc = read32be();
    (void)dcrc;

    m_os = (uimage_os)read8();
    MWR_ERROR_ON(m_os == OS_INVALID, "invalid operating system: %s", path());

    m_arch = (uimage_arch)read8();
    MWR_ERROR_ON(m_arch == ARCH_INVALID, "invalid architecture: %s", path());

    m_type = (uimage_type)read8();
    MWR_ERROR_ON(m_type == TYPE_INVALID, "invalid payload type: %s", path());

    m_comp = (uimage_comp)read8();

    m_name = readstr();
}

size_t uimage::read(u8* dest, size_t size, size_t offset) {
    MWR_ERROR_ON(!m_file, "invalid uimage");
    MWR_ERROR_ON(size + offset > m_size, "reading beyond end of file");

    const size_t header_size = 0x40;
    m_file.seekg(offset + header_size, std::ios::beg);
    m_file.read((char*)dest, size);
    return m_file.gcount();
}

u8 uimage::read8() {
    u8 val = 0;
    m_file.read((char*)&val, sizeof(val));
    MWR_ERROR_ON(!m_file, "error reading uimage header: %s", path());
    return val;
}

u32 uimage::read32be() {
    u32 val = 0;
    m_file.read((char*)&val, sizeof(val));
    MWR_ERROR_ON(!m_file, "error reading uimage header: %s", path());
    return bswap(val);
}

string uimage::readstr() {
    char strbuf[33];
    memset(strbuf, 0, sizeof(strbuf));
    m_file.read(strbuf, 32);
    MWR_ERROR_ON(!m_file, "error reading uimage header: %s", path());
    return strbuf;
}

const char* uimage_os_to_str(uimage::uimage_os os) {
    switch (os) {
    case uimage::OS_INVALID:
        return "Invalid OS";
    case uimage::OS_OPENBSD:
        return "OpenBSD";
    case uimage::OS_NETBSD:
        return "NetBSD";
    case uimage::OS_FREEBSD:
        return "FreeBSD";
    case uimage::OS_4_4BSD:
        return "4.4BSD";
    case uimage::OS_LINUX:
        return "Linux";
    case uimage::OS_SVR4:
        return "SVR4";
    case uimage::OS_ESIX:
        return "Esix";
    case uimage::OS_SOLARIS:
        return "Solaris";
    case uimage::OS_IRIX:
        return "Irix";
    case uimage::OS_SCO:
        return "SCO";
    case uimage::OS_DELL:
        return "Dell";
    case uimage::OS_NCR:
        return "NCR";
    case uimage::OS_LYNXOS:
        return "LynxOS";
    case uimage::OS_VXWORKS:
        return "VxWorks";
    case uimage::OS_PSOS:
        return "pSOS";
    case uimage::OS_QNX:
        return "QNX";
    case uimage::OS_U_BOOT:
        return "Firmware";
    case uimage::OS_RTEMS:
        return "RTEMS";
    case uimage::OS_ARTOS:
        return "ARTOS";
    case uimage::OS_UNITY:
        return "Unity OS";
    case uimage::OS_INTEGRITY:
        return "INTEGRITY";
    case uimage::OS_OSE:
        return "OSE";
    case uimage::OS_PLAN9:
        return "Plan 9";
    case uimage::OS_OPENRTOS:
        return "OpenRTOS";
    case uimage::OS_ARM_TRUSTED_FIRMWARE:
        return "ARM Trusted Firmware";
    case uimage::OS_TEE:
        return "Trusted Execution Environment";
    case uimage::OS_OPENSBI:
        return "RISC-V OpenSBI";
    case uimage::OS_EFI:
        return "EFI Firmware (e.g. GRUB2)";
    default:
        return "Unknown OS";
    }
}

const char* uimage_arch_to_str(uimage::uimage_arch arch) {
    switch (arch) {
    case uimage::ARCH_INVALID:
        return "Invalid CPU";
    case uimage::ARCH_ALPHA:
        return "Alpha";
    case uimage::ARCH_ARM:
        return "ARM";
    case uimage::ARCH_I386:
        return "Intel x86";
    case uimage::ARCH_IA64:
        return "IA64";
    case uimage::ARCH_MIPS:
        return "MIPS";
    case uimage::ARCH_MIPS64:
        return "MIPS 64 Bit";
    case uimage::ARCH_PPC:
        return "PowerPC";
    case uimage::ARCH_S390:
        return "IBM S390";
    case uimage::ARCH_SH:
        return "SuperH";
    case uimage::ARCH_SPARC:
        return "Sparc";
    case uimage::ARCH_SPARC64:
        return "Sparc 64 Bit";
    case uimage::ARCH_M68K:
        return "M68K";
    case uimage::ARCH_NIOS:
        return "Nios-32";
    case uimage::ARCH_MICROBLAZE:
        return "MicroBlaze";
    case uimage::ARCH_NIOS2:
        return "Nios-II";
    case uimage::ARCH_BLACKFIN:
        return "Blackfin";
    case uimage::ARCH_AVR32:
        return "AVR32";
    case uimage::ARCH_ST200:
        return "STMicroelectronics ST200";
    case uimage::ARCH_SANDBOX:
        return "Sandbox architecture (test only)";
    case uimage::ARCH_NDS32:
        return "ANDES Technology - NDS32";
    case uimage::ARCH_OPENRISC:
        return "OpenRISC 1000";
    case uimage::ARCH_ARM64:
        return "ARM64";
    case uimage::ARCH_ARC:
        return "Synopsys DesignWare ARC";
    case uimage::ARCH_X86_64:
        return "x86_64";
    case uimage::ARCH_XTENSA:
        return "Xtensa";
    case uimage::ARCH_RISCV:
        return "RISC-V";
    default:
        return "Unknown Architecture";
    }
}

const char* uimage_type_to_str(uimage::uimage_type type) {
    switch (type) {
    case uimage::TYPE_INVALID:
        return "Invalid Image";
    case uimage::TYPE_STANDALONE:
        return "Standalone Program";
    case uimage::TYPE_KERNEL:
        return "OS Kernel Image";
    case uimage::TYPE_RAMDISK:
        return "RAMDisk Image";
    case uimage::TYPE_MULTI:
        return "Multi-File Image";
    case uimage::TYPE_FIRMWARE:
        return "Firmware Image";
    case uimage::TYPE_SCRIPT:
        return "Script file";
    case uimage::TYPE_FILESYSTEM:
        return "Filesystem Image (any type)";
    case uimage::TYPE_FLATDT:
        return "Binary Flat Device Tree Blob";
    case uimage::TYPE_KWBIMAGE:
        return "Kirkwood Boot Image";
    case uimage::TYPE_IMXIMAGE:
        return "Freescale IMXBoot Image";
    case uimage::TYPE_UBLIMAGE:
        return "Davinci UBL Image";
    case uimage::TYPE_OMAPIMAGE:
        return "TI OMAP Config Header Image";
    case uimage::TYPE_AISIMAGE:
        return "TI Davinci AIS Image";
    case uimage::TYPE_KERNEL_NOLOAD:
        return "OS Kernel Image (can run from any load address)";
    case uimage::TYPE_PBLIMAGE:
        return "Freescale PBL Boot Image";
    case uimage::TYPE_MXSIMAGE:
        return "Freescale MXSBoot Image";
    case uimage::TYPE_GPIMAGE:
        return "TI Keystone GPHeader Image";
    case uimage::TYPE_ATMELIMAGE:
        return "ATMEL ROM bootable Image";
    case uimage::TYPE_SOCFPGAIMAGE:
        return "Altera SOCFPGA CV/AV Preloader";
    case uimage::TYPE_X86_SETUP:
        return "x86 setup.bin Image";
    case uimage::TYPE_LPC32XXIMAGE:
        return "x86 setup.bin Image";
    case uimage::TYPE_LOADABLE:
        return "A list of typeless images";
    case uimage::TYPE_RKIMAGE:
        return "Rockchip Boot Image";
    case uimage::TYPE_RKSD:
        return "Rockchip SD card";
    case uimage::TYPE_RKSPI:
        return "Rockchip SPI image";
    case uimage::TYPE_ZYNQIMAGE:
        return "Xilinx Zynq Boot Image";
    case uimage::TYPE_ZYNQMPIMAGE:
        return "Xilinx ZynqMP Boot Image";
    case uimage::TYPE_ZYNQMPBIF:
        return "Xilinx ZynqMP Boot Image (bif)";
    case uimage::TYPE_FPGA:
        return "FPGA Image";
    case uimage::TYPE_VYBRIDIMAGE:
        return "VYBRID .vyb Image";
    case uimage::TYPE_TEE:
        return "Trusted Execution Environment OS Image";
    case uimage::TYPE_FIRMWARE_IVT:
        return "Firmware Image with HABv4 IVT";
    case uimage::TYPE_PMMC:
        return "TI Power Management Micro-Controller Firmware";
    case uimage::TYPE_STM32IMAGE:
        return "STMicroelectronics STM32 Image";
    case uimage::TYPE_SOCFPGAIMAGE_V1:
        return "Altera SOCFPGA A10 Preloader";
    case uimage::TYPE_MTKIMAGE:
        return "MediaTek BootROM loadable Image";
    case uimage::TYPE_IMX8MIMAGE:
        return "Freescale IMX8MBoot Image";
    case uimage::TYPE_IMX8IMAGE:
        return "Freescale IMX8Boot Image";
    case uimage::TYPE_COPRO:
        return "Coprocessor Image for remoteproc";
    case uimage::TYPE_SUNXI_EGON:
        return "Allwinner eGON Boot Image";
    case uimage::TYPE_SUNXI_TOC0:
        return "Allwinner TOC0 Boot Image";
    case uimage::TYPE_FDT_LEGACY:
        return "Binary Flat Device Tree Blob in a Legacy Image";
    case uimage::TYPE_RENESAS_SPKG:
        return "Renesas SPKG image";
    default:
        return "Unknown Type";
    }
}

const char* uimage_comp_to_str(uimage::uimage_comp comp) {
    switch (comp) {
    case uimage::COMP_NONE:
        return "none";
    case uimage::COMP_GZIP:
        return "gzip";
    case uimage::COMP_BZIP2:
        return "bzip2";
    case uimage::COMP_LZMA:
        return "lzma";
    case uimage::COMP_LZO:
        return "lzo";
    case uimage::COMP_LZ4:
        return "lz4";
    case uimage::COMP_ZSTD:
        return "zstd";
    default:
        return "unknown compression";
    }
}

} // namespace mwr
