/******************************************************************************
 *                                                                            *
 * Copyright (C) 2025 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include <optional>

#include "mwr/utils/fdt.h"

namespace mwr {

static constexpr u32 FDT_MAGIC = 0xd00dfeed;
static constexpr u32 FDT_VERSION = 17;
static constexpr u32 FDT_COMPAT = 16;
static constexpr u32 FDT_BEGIN_NODE = 0x00000001;
static constexpr u32 FDT_END_NODE = 0x00000002;
static constexpr u32 FDT_PROP = 0x00000003;
static constexpr u32 FDT_NOP = 0x00000004;
static constexpr u32 FDT_END = 0x00000009;

static void doindent(ostream& os, size_t n) {
    for (size_t i = 0; i < n; i++)
        os << "    ";
}

fdtprop::fdtprop(const string& name, vector<u32> numbers):
    m_name(name),
    m_size(numbers.size() * sizeof(u32)),
    m_numbers(std::move(numbers)),
    m_strings() {
}

fdtprop::fdtprop(const string& name, vector<string> strings):
    m_name(name), m_size(), m_numbers(), m_strings(std::move(strings)) {
    for (const auto& str : m_strings)
        m_size += str.size() + 1;
}

void fdtprop::dump(ostream& os, size_t indent) const {
    doindent(os, indent);
    os << m_name;

    if (!m_numbers.empty()) {
        os << mkstr(" = <0x%x", m_numbers[0]);
        for (size_t i = 1; i < m_numbers.size(); i++)
            os << mkstr(" 0x%x", m_numbers[i]);
        os << ">";
    }

    if (!m_strings.empty()) {
        os << mkstr(" = \"%s\"", m_strings[0].c_str());
        for (size_t i = 1; i < m_strings.size(); i++)
            os << mkstr(", \"%s\"", m_strings[i].c_str());
    }

    os << ";";
}

fdtnode::~fdtnode() {
    for (auto prop : m_props)
        delete prop;
    for (auto child : m_children)
        delete child;
}

u32 fdtnode::handle() {
    for (auto* prop : m_props) {
        if (prop->name() == "phandle")
            return prop->numbers()[0];
    }

    static u32 handles = 0;
    add_property("phandle", ++handles);
    return handles;
}

void fdtnode::dump(ostream& os, size_t indent) const {
    doindent(os, indent);
    os << m_name << " {" << std::endl;

    for (const auto& prop : m_props) {
        prop->dump(os, indent + 1);
        os << std::endl;
    }

    for (const auto& child : m_children) {
        child->dump(os, indent + 1);
        os << std::endl;
    }

    doindent(os, indent);
    os << "};";
}

ostream& operator<<(ostream& os, const fdtprop& prop) {
    prop.dump(os);
    return os;
}

ostream& operator<<(ostream& os, const fdtnode& node) {
    node.dump(os);
    return os;
}

class fdtwriter
{
private:
    vector<string> m_strings;

    virtual void seek(u32 offset) = 0;
    virtual void putc(u8 byte) = 0;

    size_t write(u32 data) {
#ifdef MWR_HOST_LITTLE_ENDIAN
        putc(data >> 24);
        putc(data >> 16);
        putc(data >> 8);
        putc(data >> 0);
#else
        putc(data >> 0);
        putc(data >> 8);
        putc(data >> 16);
        putc(data >> 24);
#endif
        return sizeof(data);
    }

    size_t strtab(const string& s) {
        u32 off = 0;
        for (const string& str : m_strings) {
            if (str == s)
                return write(off);
            off += (u32)str.length() + 1;
        }

        m_strings.push_back(s);
        return write(off);
    }

    size_t write_padding(size_t curlen) {
        size_t prev = curlen;
        while (curlen % 4) {
            putc(0);
            curlen++;
        }
        return curlen - prev;
    }

    size_t write(const string& s) {
        for (u8 ch : s)
            putc(ch);
        putc(0);
        return s.length() + 1;
    }

    size_t write(const fdtprop& prop) {
        size_t len = 0;
        len += write(FDT_PROP);
        len += write((u32)prop.size());
        len += strtab(prop.name());
        for (u32 num : prop.numbers())
            len += write(num);
        for (const string& str : prop.strings())
            len += write(str);
        len += write_padding(len);
        return len;
    }

    size_t write(const fdtnode& node) {
        size_t len = 0;
        len += write(FDT_BEGIN_NODE);
        len += write(node.name() == "/" ? "" : node.name());
        len += write_padding(len);
        for (const auto& prop : node.properties())
            len += write(*prop);
        for (const auto& child : node.children())
            len += write(*child);
        len += write(FDT_END_NODE);
        return len;
    }

    size_t write_header() {
        size_t len = 0;
        len += write(FDT_MAGIC);
        len += write(0);           // totalsize
        len += write(56);          // off_dt_struct
        len += write(0);           // off_dt_strings
        len += write(40);          // off_mem_rsvmap
        len += write(FDT_VERSION); // version
        len += write(FDT_COMPAT);  // last_comp_version
        len += write(0);           // phys cpu id
        len += write(0);           // size_dt_strings
        len += write(0);           // size_dt_struct
        len += write(0);           // res addr lo
        len += write(0);           // res addr hi
        len += write(0);           // res size lo
        len += write(0);           // res size hi
        return len;
    }

    void finalize_header(size_t total_size, size_t struct_size,
                         size_t string_size, size_t string_off) {
        seek(0x04);
        write((u32)total_size);
        seek(0x0c);
        write((u32)string_off);
        seek(0x20);
        write((u32)string_size);
        seek(0x24);
        write((u32)struct_size);
    }

    size_t write_strings() {
        size_t len = 0;
        for (const auto& str : m_strings) {
            for (auto ch : str)
                putc(ch);
            putc(0);
            len += str.length() + 1;
        }

        while (len % 4) {
            putc(0);
            len++;
        }

        return len;
    }

public:
    fdtwriter() = default;
    virtual ~fdtwriter() = default;

    void compile(const fdtnode& root) {
        size_t header_size = write_header();
        size_t struct_size = 0;
        struct_size += write(root);
        struct_size += write(FDT_END);
        size_t string_size = write_strings();

        finalize_header(header_size + struct_size + string_size, struct_size,
                        string_size, header_size + struct_size);
    }
};

class fdtwriter_mem : public fdtwriter
{
private:
    u8* m_buf;
    u8* m_ptr;
    u8* m_end;

public:
    fdtwriter_mem(void* buf, size_t len):
        m_buf((u8*)buf), m_ptr(m_buf), m_end(m_buf + len) {}

    virtual void seek(u32 offset) override {
        m_ptr = m_buf + offset;
        MWR_REPORT_ON(m_ptr >= m_end, "writing beyond end of buffer");
    }

    virtual void putc(u8 byte) override {
        MWR_REPORT_ON(m_ptr >= m_end, "writing beyond end of buffer");
        *m_ptr++ = byte;
    }
};

class fdtwriter_file : public fdtwriter
{
private:
    fstream m_file;

public:
    fdtwriter_file(const string& filename):
        m_file(filename, std::ios::out | std::ios::binary) {
        MWR_REPORT_ON(!m_file, "cannot open %s", filename.c_str());
    }

    virtual void seek(u32 offset) override { m_file.seekp(offset); }

    virtual void putc(u8 byte) override {
        m_file.write((const char*)&byte, sizeof(byte));
    }
};

void fdtcompile(const fdtnode& root, void* buffer, size_t buflen) {
    fdtwriter_mem writer(buffer, buflen);
    writer.compile(root);
}

void fdtcompile(const fdtnode& root, const string& filename) {
    fdtwriter_file writer(filename);
    writer.compile(root);
}

static bool is_str_property(const string& name) {
    static const string keywords[] = {
        "compatible", "aliases",     "stdout-path", "bootargs",
        "choosen",    "device-type", "status",      "interrupt-names",
    };

    for (const string& key : keywords)
        if (name == key)
            return true;

    return name.find("name") != name.npos;
}

static bool is_int_property(const string& name) {
    static const string keywords[] = {
        "reg",           "interrupts", "interrupt-parent",
        "interrupt-map", "iommu-map",  "phandle",
    };

    for (const string& key : keywords)
        if (name == key)
            return true;

    if (name.find("cells") != name.npos)
        return true;
    if (name.find("range") != name.npos)
        return true;

    return false;
}

class fdtreader
{
public:
    virtual u8 getc() = 0;
    virtual u32 seek(u32 off) = 0;

    u32 read() {
        u32 a = getc();
        u32 b = getc();
        u32 c = getc();
        u32 d = getc();
#ifdef MWR_HOST_LITTLE_ENDIAN
        return a << 24 | b << 16 | c << 8 | d;
#else
        return d << 24 | c << 16 | b << 8 | a;
#endif
    }

    string read_string(bool align) {
        string s;
        char ch = getc();
        while (ch != 0) {
            s.push_back(ch);
            ch = getc();
        }

        size_t len = s.length() + 1;
        while (align && (len++ % 4))
            getc();

        return s;
    }

    string read_strtab(u32 offset) {
        u32 curoff = seek(12);
        u32 dtoff = read();

        seek(dtoff + offset);
        string s = read_string(false);
        seek(curoff);
        return s;
    }

    std::optional<vector<u32>> read_int_data(u32 len) {
        if (len % 4)
            return std::nullopt;

        vector<u32> data;
        for (u32 i = 0; i < len; i += 4)
            data.push_back(read());
        return data;
    }

    std::optional<vector<string>> read_str_data(u32 len) {
        u32 curoff = seek(0);
        seek(curoff);
        vector<string> result;
        string s;
        for (u32 i = 0; i < len; i++) {
            u8 ch = getc();
            if (ch == 0) {
                result.push_back(s);
                s.clear();
            } else if (isprint(ch)) {
                s.push_back(ch);
            } else {
                seek(curoff);
                return std::nullopt;
            }
        }

        if (!s.empty()) {
            seek(curoff);
            return std::nullopt;
        }

        while (len++ % 4)
            getc();

        return result;
    }

    void traverse(fdtnode& parent) {
        for (size_t i = 0; i < 1000000; i++) {
            u32 token = read();
            switch (token) {
            case FDT_BEGIN_NODE: {
                string name = read_string(true);
                fdtnode& child = parent.add_child(name);
                traverse(child);
                break;
            }

            case FDT_PROP: {
                u32 len = read();
                u32 nameoff = read();
                string name = read_strtab(nameoff);

                if (is_int_property(name)) {
                    if (auto v = read_int_data(len)) {
                        parent.add_property(new fdtprop(name, *v));
                        break;
                    }
                }

                if (is_str_property(name)) {
                    if (auto v = read_str_data(len)) {
                        parent.add_property(new fdtprop(name, *v));
                        break;
                    }
                }

                // unknown property type, try string first, then int
                if (auto v = read_str_data(len))
                    parent.add_property(new fdtprop(name, *v));
                else if (auto v = read_int_data(len))
                    parent.add_property(new fdtprop(name, *v));
                else
                    MWR_REPORT("error reading property '%s'", name.c_str());
                break;
            }

            case FDT_NOP:
                continue;

            case FDT_END_NODE:
                return;

            default:
                MWR_REPORT("invalid fdt token: 0x%08x", token);
            }
        }
    }

    fdtnode decompile() {
        u32 magic = read();
        MWR_REPORT_ON(magic != FDT_MAGIC, "invalid fdt header: 0x%08x", magic);

        seek(24);
        u32 compat = read();
        if (compat != FDT_COMPAT)
            MWR_REPORT("fdt version unsupported: %u", compat);

        seek(8);
        u32 offset = read();

        seek(offset);
        u32 token = read();
        if (token != FDT_BEGIN_NODE)
            MWR_REPORT("invalid start token: 0x%08x", token);

        string name = read_string(true);
        fdtnode root(name.empty() ? "/" : name);
        traverse(root);
        token = read();
        if (token != FDT_END)
            MWR_REPORT("invalid end-of-file token: 0x%08x", token);
        return root;
    }
};

class fdtreader_mem : public fdtreader
{
private:
    const u8* m_buf;
    const u8* m_ptr;
    const u8* m_end;

public:
    fdtreader_mem(const void* buf, size_t len):
        m_buf((const u8*)buf), m_ptr(m_buf), m_end(m_buf + len) {}

    virtual u32 seek(u32 offset) override {
        size_t curoff = m_ptr - m_buf;
        m_ptr = m_buf + offset;
        MWR_REPORT_ON(m_ptr >= m_end, "seeking beyond end of buffer");
        return (u32)curoff;
    }

    virtual u8 getc() override {
        MWR_REPORT_ON(m_ptr >= m_end, "reading beyond end of buffer");
        return *m_ptr++;
    }
};

class fdtreader_file : public fdtreader
{
private:
    fstream m_file;

public:
    fdtreader_file(const string& filename):
        m_file(filename, std::ios::in | std::ios::binary) {
        MWR_REPORT_ON(!m_file, "cannot open %s", filename.c_str());
    }

    virtual u32 seek(u32 offset) override {
        u32 curoff = (u32)m_file.tellg();
        m_file.seekg(offset);
        return curoff;
    }

    virtual u8 getc() override {
        u8 ch = 0;
        m_file.read((char*)&ch, sizeof(ch));
        MWR_REPORT_ON(!m_file, "error reading from file: %d", errno);
        return ch;
    }
};

fdtnode fdtdecompile(const void* buffer, size_t buflen) {
    fdtreader_mem reader(buffer, buflen);
    return reader.decompile();
}

fdtnode fdtdecompile(const string& filename) {
    fdtreader_file reader(filename);
    return reader.decompile();
}

} // namespace mwr
