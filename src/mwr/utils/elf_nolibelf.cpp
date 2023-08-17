/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/core/utils.h"
#include "mwr/stl/strings.h"
#include "mwr/utils/elf.h"

namespace mwr {

struct shdr32 {
    u32 name;
    u32 type;
    u32 flags;
    u32 addr;
    u32 offset;
    u32 size;
    u32 link;
    u32 info;
    u32 addralign;
    u32 entsize;
};

struct shdr64 {
    u32 name;
    u32 type;
    u64 flags;
    u64 addr;
    u64 offset;
    u64 size;
    u32 link;
    u32 info;
    u64 addralign;
    u64 entsize;
};

struct phdr32 {
    u32 type;
    u32 offset;
    u32 vaddr;
    u32 paddr;
    u32 filesz;
    u32 memsz;
    u32 flags;
    u32 align;
};

struct phdr64 {
    u32 type;
    u32 flags;
    u64 offset;
    u64 vaddr;
    u64 paddr;
    u64 filesz;
    u64 memsz;
    u64 align;
};

struct sym32 {
    u32 name;
    u32 value;
    u32 size;
    u8 info;
    u8 other;
    u16 shndx;
};

struct sym64 {
    u32 name;
    u8 info;
    u8 other;
    u16 shndx;
    u64 value;
    u64 size;
};

struct ehdr32 {
    // ident[16] not included
    u16 type;
    u16 machine;
    u32 version;
    u32 entry;
    u32 phoff;
    u32 shoff;
    u32 flags;
    u16 ehsize;
    u16 phentsize;
    u16 phnum;
    u16 shentsize;
    u16 shnum;
    u16 shstrndx;

    using phdr_t = phdr32;
    using shdr_t = shdr32;
    using sym_t = sym32;
};

struct ehdr64 {
    // ident[16] not included
    u16 type;
    u16 machine;
    u32 version;
    u64 entry;
    u64 phoff;
    u64 shoff;
    u32 flags;
    u16 ehsize;
    u16 phentsize;
    u16 phnum;
    u16 shentsize;
    u16 shnum;
    u16 shstrndx;

    using phdr_t = phdr64;
    using shdr_t = shdr64;
    using sym_t = sym64;
};

constexpr u32 PT_LOAD = 1;
constexpr u32 PF_R = 4;
constexpr u32 PF_W = 2;
constexpr u32 PF_X = 1;
constexpr u32 SHT_SYMTAB = 2;

constexpr elf::elf_sym_bind get_bind(u8 info) {
    switch (info >> 4) {
    case 1:
        return elf::BIND_GLOBAL;
    case 2:
        return elf::BIND_WEAK;
    default:
        return elf::BIND_LOCAL;
    }
}

constexpr elf::elf_sym_kind get_kind(u8 info) {
    switch (info & 0xf) {
    case 0:
        return elf::KIND_UNKNOWN;
    case 1:
        return elf::KIND_OBJECT;
    case 2:
        return elf::KIND_FUNC;
    case 5:
        return elf::KIND_COMMON;
    case 6:
        return elf::KIND_TLS;
    default:
        return elf::KIND_NONE;
    }
}

template <typename EHDR>
static vector<elf::segment> read_segments(int fd, const EHDR& hdr) {
    vector<elf::segment> segments;
    for (size_t i = 0; i < hdr.phnum; i++) {
        size_t offset = hdr.phoff + i * sizeof(typename EHDR::phdr_t);
        if (fd_seek(fd, offset) != offset)
            MWR_ERROR("cannot find elf program header %zu", i);

        typename EHDR::phdr_t phdr = {};
        if (fd_read(fd, &phdr, sizeof(phdr)) != sizeof(phdr))
            MWR_ERROR("eof while reading program header %zu", i);

        if (phdr.type == PT_LOAD) {
            bool r = phdr.flags & PF_R;
            bool w = phdr.flags & PF_W;
            bool x = phdr.flags & PF_X;
            segments.push_back({ phdr.vaddr, phdr.paddr, phdr.memsz,
                                 phdr.filesz, phdr.offset, r, w, x });
        }
    }

    return segments;
}

struct strings {
    vector<char> data;

    template <typename EHDR>
    strings(int fd, const EHDR& hdr, u64 idx): data() {
        size_t offset = hdr.shoff + idx * sizeof(typename EHDR::shdr_t);
        if (fd_seek(fd, offset) != offset)
            MWR_ERROR("cannot find string table header");

        typename EHDR::shdr_t shdr = {};
        if (fd_read(fd, &shdr, sizeof(shdr)) != sizeof(shdr))
            MWR_ERROR("EOF while reading string table header");

        if (fd_seek(fd, shdr.offset) != shdr.offset)
            MWR_ERROR("cannot find string table");

        data = vector<char>(shdr.size);
        if (fd_read(fd, data.data(), data.size()) != data.size())
            MWR_ERROR("failed to read string table");
    }

    string get(u32 idx) const {
        MWR_ERROR_ON(idx >= data.size(), "string index out of bounds");
        string result;
        for (size_t i = idx; i < data.size(); i++) {
            if (data[i])
                result.push_back(data[i]);
            else
                break;
        }
        return result;
    }
};

template <typename EHDR>
static vector<elf::symbol> read_symbols(int fd, const EHDR& hdr) {
    using shdr_t = typename EHDR::shdr_t;
    using sym_t = typename EHDR::sym_t;

    vector<elf::symbol> symbols;
    for (size_t i = 0; i < hdr.shnum; i++) {
        size_t offset = hdr.shoff + i * sizeof(shdr_t);
        if (fd_seek(fd, offset) != offset)
            MWR_ERROR("cannot find elf program header %zu", i);

        shdr_t shdr = {};
        if (fd_read(fd, &shdr, sizeof(shdr)) != sizeof(shdr))
            MWR_ERROR("EOF while reading section header %zu", i);

        if (shdr.type != SHT_SYMTAB)
            continue;

        if (fd_seek(fd, shdr.offset) != shdr.offset)
            MWR_ERROR("cannot find symbols table");

        vector<sym_t> syms(shdr.size / sizeof(sym_t));
        size_t size = syms.size() * sizeof(sym_t);
        if (fd_read(fd, syms.data(), size) != size)
            MWR_ERROR("EOF while reading symbol table");

        strings strtab(fd, hdr, shdr.link);
        for (const auto& sym : syms) {
            elf::symbol symbol;
            symbol.size = sym.size;
            symbol.virt = sym.value;
            symbol.phys = ~0ull; // will be patched later
            symbol.kind = get_kind(sym.info);
            symbol.bind = get_bind(sym.info);
            symbol.name = strtab.get(sym.name);

            if (symbol.kind != elf::KIND_NONE)
                symbols.push_back(symbol);
        }
    }

    return symbols;
}

u64 elf::to_phys(u64 virt) const {
    for (auto& seg : m_segments) {
        if ((virt >= seg.virt) && virt < (seg.virt + seg.size))
            return seg.phys + virt - seg.virt;
    }

    return virt;
}

const elf::symbol* elf::find_symbol(const string& name) const {
    for (const auto& sym : m_symbols)
        if (sym.name == name)
            return &sym;
    return nullptr;
}

elf::elf(const string& path):
    m_filename(path),
    m_fd(-1),
    m_entry(0),
    m_asize(8),
    m_machine(),
    m_symbols(),
    m_segments() {
    m_fd = fd_open(filename(), "rb");
    if (m_fd < 0)
        MWR_ERROR("cannot open elf file '%s'", filename());

    u8 ident[16] = {};
    if (fd_read(m_fd, ident, sizeof(ident)) != sizeof(ident))
        MWR_ERROR("early EOF while reading '%s'", filename());

    u8 start[] = { 0x7f, 'E', 'L', 'F' };
    if (memcmp(ident, start, sizeof(start)))
        MWR_ERROR("not an elf file: '%s'", filename());

    m_asize = (ident[4] == 2) ? 8 : 4;
    m_big_endian = (ident[5] == 2);

    if (is_64bit()) {
        ehdr64 hdr = {};
        if (fd_read(m_fd, &hdr, sizeof(hdr)) != sizeof(hdr))
            MWR_ERROR("EOF while reading elf header in '%s'", filename());

        m_entry = hdr.entry;
        m_machine = (elf_machine)hdr.machine;
        m_segments = read_segments(m_fd, hdr);
        m_symbols = read_symbols(m_fd, hdr);
    } else {
        ehdr32 hdr = {};
        if (fd_read(m_fd, &hdr, sizeof(hdr)) != sizeof(hdr))
            MWR_ERROR("EOF while reading elf header in '%s'", filename());

        m_entry = hdr.entry;
        m_machine = (elf_machine)hdr.machine;
        m_segments = read_segments(m_fd, hdr);
        m_symbols = read_symbols(m_fd, hdr);
    }

    for (auto& sym : m_symbols)
        sym.phys = to_phys(sym.virt);
}

elf::~elf() {
    if (m_fd >= 0)
        fd_close(m_fd);
}

u64 elf::read_segment(const segment& seg, u8* dest) {
    MWR_ERROR_ON(m_fd < 0, "ELF file '%s' not open", filename());

    if (fd_seek(m_fd, seg.offset) != seg.offset)
        MWR_ERROR("cannot seek within ELF file '%s'", filename());

    if (fd_read(m_fd, dest, seg.filesz) != seg.filesz)
        MWR_ERROR("cannot read ELF file '%s'", filename());

    if (fd_seek(m_fd, 0) != 0u)
        MWR_ERROR("cannot seek within ELF file '%s'", filename());

    if (seg.filesz < seg.size)
        memset(dest + seg.filesz, 0, seg.size - seg.filesz);

    return seg.size;
}

} // namespace mwr
