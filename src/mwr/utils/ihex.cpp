/******************************************************************************
 *                                                                            *
 * Copyright (C) 2024 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * Found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/utils/ihex.h"
#include <string_view>

namespace mwr {
using namespace std;

enum record_type : u8 {
    IHEX_DATA = 0x00,
    IHEX_EOF = 0x01,
    IHEX_EX_SEG = 0x02,
    IHEX_START_SEG = 0x03,
    IHEX_EX_LIN_ADDR = 0x04,
    IHEX_START_LIN_ADDR = 0x05,
};

struct ihex_record {
    bool valid;
    u16 addr;
    record_type type;
    string_view data;
};

template <typename T>
static inline T hex_extract(string_view line, size_t off = 0) {
    static_assert(std::is_unsigned<T>::value,
                  "T must be an unsigned integer type");
    constexpr size_t num_bytes = sizeof(T);
    constexpr size_t num_digits = num_bytes * 2;

    MWR_ERROR_ON(off + num_digits > line.length(),
                 "reading beyond given string");

    T result = 0;
    for (size_t i = 0; i < num_digits; ++i) {
        result = (result << 4) | from_hex_ascii(line[off + i]);
    }
    return result;
}

static inline ihex_record process_line(string_view line) {
    const u32 recsize_wo_data = 11;
    const u32 data_start = 9;
    const u32 delim = 1;
    bool valid = true;
    if (line.length() < recsize_wo_data || line[0] != ':') {
        MWR_REPORT("Found invalid line in hex file: %*s", (int)line.size(),
                   line.data());
        valid = false;
    }

    const u8 nr_bytes = hex_extract<u8>(line, delim);

    if (line.length() < recsize_wo_data + nr_bytes * 2) {
        MWR_REPORT("Found invalid record (too short): %*s", (int)line.size(),
                   line.data());
        valid = false;
    }

    u8 check_sum = 0;
    for (u32 pos = delim; pos < recsize_wo_data + nr_bytes * 2 - delim;
         pos += 2)
        check_sum += hex_extract<u8>(line, pos);
    if (check_sum) {
        MWR_REPORT("Found invalid record (wrong checksum): %*s",
                   (int)line.size(), line.data());
        valid = false;
    }

    const u16 addr = hex_extract<u16>(line, 3);
    const record_type r_type = static_cast<record_type>(
        hex_extract<u8>(line, 7));

    switch (r_type) {
    case IHEX_DATA:
        break;
    case IHEX_EOF:
        MWR_REPORT_ON(nr_bytes || addr, "Found invalid record: %*s",
                      (int)line.size(), line.data());
        break;
    case IHEX_EX_SEG:
    case IHEX_EX_LIN_ADDR:
        MWR_REPORT_ON(nr_bytes != 2 || addr, "Found invalid record: %*s",
                      (int)line.size(), line.data());
        break;
    case IHEX_START_SEG:
    case IHEX_START_LIN_ADDR:
        MWR_REPORT_ON(nr_bytes != 4 || addr, "Found invalid record: %*s",
                      (int)line.size(), line.data());
        break;
    default:
        MWR_REPORT("Found unknown record type: %x", r_type);
        valid = false;
    }
    return { valid, addr, r_type, line.substr(data_start, nr_bytes * 2) };
}

ihex::ihex(const string& filename): m_start_addr(), m_records() {
    ifstream file(filename);
    MWR_ERROR_ON(!file, "Cannot open ihex file '%s'", filename.c_str());

    u32 seg_ext = 0;
    u32 linear_ext = 0;
    u32 eof = 0;
    string line;
    while (getline(file, line)) {
        line = trim(line);

        if (eof)
            MWR_REPORT("Found line after EOF in hex file: %s", line.c_str());

        ihex_record line_data = process_line(line);
        if (!line_data.valid)
            continue;

        switch (line_data.type) {
        case IHEX_DATA: {
            record rec = {};
            rec.addr = line_data.addr + seg_ext + linear_ext;
            for (size_t pos = 0; pos < line_data.data.size(); pos += 2)
                rec.data.push_back(hex_extract<u8>(line_data.data, pos));
            m_records.push_back(std::move(rec));
        }; break;
        case IHEX_EOF:
            eof += 1;
            break;
        case IHEX_EX_SEG:
            seg_ext = hex_extract<u16>(line_data.data) << 4;
            break;
        case IHEX_START_SEG:
            m_start_addr = hex_extract<u32>(line_data.data);
            break;
        case IHEX_EX_LIN_ADDR:
            linear_ext = hex_extract<u16>(line_data.data) << 16;
            break;
        case IHEX_START_LIN_ADDR:
            m_start_addr = hex_extract<u32>(line_data.data);
            break;
        default:
            break;
        }
    }
    MWR_REPORT_ON(eof != 1, "Wrong number of EOF in file, count: %d", eof);
}

} // namespace mwr
