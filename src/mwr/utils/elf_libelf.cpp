/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/core/utils.h"
#include "mwr/utils/elf.h"

#include <unistd.h>
#include <fcntl.h>
#include <libelf.h>

namespace mwr {

constexpr elf::elf_sym_bind get_bind(u8 info) {
    switch (ELF32_ST_BIND(info)) {
    case STB_GLOBAL:
        return elf::BIND_GLOBAL;
    case STB_WEAK:
        return elf::BIND_WEAK;
    default:
        return elf::BIND_LOCAL;
    }
}

constexpr elf::elf_sym_kind get_kind(u8 info) {
    switch (ELF32_ST_TYPE(info)) {
    case STT_NOTYPE:
        return elf::KIND_UNKNOWN;
    case STT_OBJECT:
        return elf::KIND_OBJECT;
    case STT_COMMON:
        return elf::KIND_COMMON;
    case STT_TLS:
        return elf::KIND_TLS;
    case STT_FUNC:
        return elf::KIND_FUNC;
    default:
        return elf::KIND_NONE;
    }
}

struct elf32_traits {
    typedef Elf32_Ehdr Elf_Ehdr;
    typedef Elf32_Phdr Elf_Phdr;
    typedef Elf32_Shdr Elf_Shdr;
    typedef Elf32_Sym Elf_Sym;

    static Elf_Ehdr* elf_getehdr(Elf* elf) { return elf32_getehdr(elf); }
    static Elf_Phdr* elf_getphdr(Elf* elf) { return elf32_getphdr(elf); }
    static Elf_Shdr* elf_getshdr(Elf_Scn* scn) { return elf32_getshdr(scn); }
};

struct elf64_traits {
    typedef Elf64_Ehdr Elf_Ehdr;
    typedef Elf64_Phdr Elf_Phdr;
    typedef Elf64_Shdr Elf_Shdr;
    typedef Elf64_Sym Elf_Sym;

    static Elf_Ehdr* elf_getehdr(Elf* elf) { return elf64_getehdr(elf); }
    static Elf_Phdr* elf_getphdr(Elf* elf) { return elf64_getphdr(elf); }
    static Elf_Shdr* elf_getshdr(Elf_Scn* scn) { return elf64_getshdr(scn); }
};

template <typename T>
static vector<elf::segment> read_segments(Elf* elf) {
    size_t count = 0;
    int err = elf_getphdrnum(elf, &count);
    if (err)
        MWR_ERROR("elf_begin failed: %s", elf_errmsg(err));

    vector<elf::segment> segments;
    typename T::Elf_Phdr* hdr = T::elf_getphdr(elf);
    for (u64 i = 0; i < count; i++, hdr++) {
        if (hdr->p_type == PT_LOAD) {
            bool r = hdr->p_flags & PF_R;
            bool w = hdr->p_flags & PF_W;
            bool x = hdr->p_flags & PF_X;
            segments.push_back({ hdr->p_vaddr, hdr->p_paddr, hdr->p_memsz,
                                 hdr->p_filesz, hdr->p_offset, r, w, x });
        }
    }

    return segments;
}

template <typename T>
static vector<elf::symbol> read_symbols(Elf* elf) {
    vector<elf::symbol> symbols;

    Elf_Scn* scn = nullptr;
    while ((scn = elf_nextscn(elf, scn)) != nullptr) {
        typename T::Elf_Shdr* shdr = T::elf_getshdr(scn);
        if (shdr->sh_type != SHT_SYMTAB)
            continue;

        Elf_Data* data = elf_getdata(scn, nullptr);
        size_t numsyms = shdr->sh_size / shdr->sh_entsize;

        auto* syms = (typename T::Elf_Sym*)(data->d_buf);
        for (size_t i = 0; i < numsyms; i++) {
            char* str = elf_strptr(elf, shdr->sh_link, syms[i].st_name);
            if (str == nullptr || strlen(str) == 0)
                continue;

            elf::symbol symbol;
            symbol.size = syms[i].st_size;
            symbol.virt = syms[i].st_value;
            symbol.phys = ~0ull; // will be patched later
            symbol.kind = get_kind(syms[i].st_info);
            symbol.bind = get_bind(syms[i].st_info);
            symbol.name = string(str);

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
    m_big_endian(false),
    m_machine(NONE),
    m_symbols(),
    m_segments() {
    if (elf_version(EV_CURRENT) == EV_NONE)
        MWR_ERROR("failed to read libelf version");

    m_fd = open(filename(), O_RDONLY, 0);
    if (m_fd < 0)
        MWR_ERROR("cannot open elf file '%s'", filename());

    Elf* elf = elf_begin(m_fd, ELF_C_READ, nullptr);
    if (elf == nullptr)
        MWR_ERROR("error reading '%s' (%s)", filename(), elf_errmsg(-1));

    if (elf_kind(elf) != ELF_K_ELF)
        MWR_ERROR("ELF version error in %s", filename());

    Elf32_Ehdr* ehdr32 = elf32_getehdr(elf);
    Elf64_Ehdr* ehdr64 = elf64_getehdr(elf);

    if (ehdr32) {
        m_asize = 4;
        m_entry = ehdr32->e_entry;
        m_machine = (elf_machine)ehdr32->e_machine;
        m_big_endian = ehdr32->e_ident[EI_DATA] == ELFDATA2MSB;
        m_segments = read_segments<elf32_traits>(elf);
        m_symbols = read_symbols<elf32_traits>(elf);
    } else if (ehdr64) {
        m_asize = 8;
        m_entry = ehdr64->e_entry;
        m_machine = (elf_machine)ehdr64->e_machine;
        m_big_endian = ehdr64->e_ident[EI_DATA] == ELFDATA2MSB;
        m_segments = read_segments<elf64_traits>(elf);
        m_symbols = read_symbols<elf64_traits>(elf);
    } else {
        MWR_ERROR("unable to determine elf class: %s", filename());
    }

    elf_end(elf);

    for (auto& sym : m_symbols)
        sym.phys = to_phys(sym.virt);
}

elf::~elf() {
    if (m_fd >= 0)
        close(m_fd);
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
