/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_ELF_H
#define MWR_UTILS_ELF_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"

#include "mwr/stl/strings.h"
#include "mwr/stl/containers.h"

namespace mwr {

class elf
{
public:
    enum elf_machine : u16 {
        NONE = 0,
        ARC = 45,
        ARM = 40,
        ARM64 = 183,
        AVR = 83,
        OPENRISC = 92,
        PPC = 20,
        PPC64 = 21,
        RISCV = 243,
        V850 = 87,
        X86 = 3,
        X86_64 = 62,
        XTENSA = 94,
    };

    enum elf_sym_bind : u8 {
        BIND_LOCAL,
        BIND_GLOBAL,
        BIND_WEAK,
    };

    enum elf_sym_kind : u8 {
        KIND_UNKNOWN,
        KIND_OBJECT,
        KIND_COMMON,
        KIND_TLS,
        KIND_FUNC,
        KIND_NONE,
    };

    struct symbol {
        u64 virt;
        u64 phys;
        u64 size;
        string name;
        elf_sym_kind kind;
        elf_sym_bind bind;
    };

    struct segment {
        u64 virt;
        u64 phys;
        u64 size;
        u64 filesz;
        u64 offset;
        bool r, w, x;
    };

    const char* filename() const { return m_filename.c_str(); }

    u64 entry() const { return m_entry; }
    elf_machine machine() const { return m_machine; }

    bool is_big_endian() const { return m_big_endian; }
    bool is_little_endian() const { return !is_big_endian(); }

    bool is_64bit() const { return m_asize == 8; }
    bool is_32bit() const { return !is_64bit(); }

    const vector<segment>& segments() const { return m_segments; }
    const vector<symbol>& symbols() const { return m_symbols; }

    const symbol* find_symbol(const string& name) const;

    elf(const string& filename);
    ~elf();

    u64 read_segment(const segment& segment, u8* dest);

private:
    string m_filename;

    int m_fd;
    u64 m_entry;
    bool m_big_endian;
    size_t m_asize;
    elf_machine m_machine;

    vector<symbol> m_symbols;
    vector<segment> m_segments;

    u64 to_phys(u64 virt) const;

    elf(const elf&) = delete;
};

} // namespace mwr

#endif
