/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "testing.h"

#include "mwr.h"

using namespace mwr;

TEST(uimage, os) {
    EXPECT_STREQ(uimage_os_to_str(uimage::OS_LINUX), "Linux");
    EXPECT_STREQ(uimage_os_to_str(uimage::OS_OPENBSD), "OpenBSD");
    EXPECT_STREQ(uimage_os_to_str(uimage::OS_VXWORKS), "VxWorks");
}

TEST(uimage, arch) {
    EXPECT_STREQ(uimage_arch_to_str(uimage::ARCH_ARM64), "ARM64");
    EXPECT_STREQ(uimage_arch_to_str(uimage::ARCH_RISCV), "RISC-V");
    EXPECT_STREQ(uimage_arch_to_str(uimage::ARCH_OPENRISC), "OpenRISC 1000");
}

TEST(uimage, type) {
    EXPECT_STREQ(uimage_type_to_str(uimage::TYPE_KERNEL), "OS Kernel Image");
    EXPECT_STREQ(uimage_type_to_str(uimage::TYPE_RAMDISK), "RAMDisk Image");
    EXPECT_STREQ(uimage_type_to_str(uimage::TYPE_FPGA), "FPGA Image");
}

TEST(uimage, comp) {
    EXPECT_STREQ(uimage_comp_to_str(uimage::COMP_NONE), "none");
    EXPECT_STREQ(uimage_comp_to_str(uimage::COMP_GZIP), "gzip");
    EXPECT_STREQ(uimage_comp_to_str(uimage::COMP_BZIP2), "bzip2");
    EXPECT_STREQ(uimage_comp_to_str(uimage::COMP_LZMA), "lzma");
    EXPECT_STREQ(uimage_comp_to_str(uimage::COMP_LZO), "lzo");
    EXPECT_STREQ(uimage_comp_to_str(uimage::COMP_LZ4), "lz4");
    EXPECT_STREQ(uimage_comp_to_str(uimage::COMP_ZSTD), "zstd");
}

TEST(uimage, load) {
    u8 payload[8] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x12, 0x34 };
    u8 rawdata[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    uimage reader(get_resource_path("uImage"));

    EXPECT_EQ(reader.load(), 0x20000000);
    EXPECT_EQ(reader.entry(), 0x20000040);

    EXPECT_STREQ(reader.name(), "my_payload");

    EXPECT_EQ(reader.os(), uimage::OS_LINUX);
    EXPECT_EQ(reader.arch(), uimage::ARCH_RISCV);
    EXPECT_EQ(reader.type(), uimage::TYPE_KERNEL);
    EXPECT_FALSE(reader.is_compressed());

    EXPECT_EQ(reader.date(), 1694440283);
    ASSERT_EQ(reader.size(), 8);
    ASSERT_EQ(reader.read(rawdata, 8), 8);

    for (int i = 0; i < 8; i++)
        EXPECT_EQ(payload[i], rawdata[i]) << "mismatch at " << i;
}
