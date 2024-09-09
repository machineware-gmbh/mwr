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

namespace mwr {

enum record_type : u8 {
    IHEX_DATA = 0x00,
    IHEX_EOF = 0x01,
    IHEX_EX_SEG = 0x02,
    IHEX_START_SEG = 0x03,
    IHEX_EX_LIN_ADDR = 0x04,
    IHEX_START_LIN_ADDR = 0x05,
    INVALID_LINE_FORMAT,
    INVALID_LINE_LEN,
    INVALID_TYPE,
    INVALID_DESCRIPTOR,
    INVALID_CHECKSUM,
};

struct ihex_record {
    record_type type;
    u64 addr;
    vector<u8> data;
};

static inline u8 ihex_byte(const string& line, size_t off) {
    MWR_ERROR_ON(off + 1 >= line.size(), "reading beyond srec line");
    return from_hex_ascii(line[off]) << 4 | from_hex_ascii(line[off + 1]);
}

template <typename T>
static inline u64 vec_to(const std::vector<u8>& vec) {
    constexpr size_t num_bytes = sizeof(T);
    MWR_ERROR_ON(num_bytes > vec.size(), "reading beyond given vector");
    T result = 0;
    for (size_t i = 0; i < num_bytes; ++i)
        result = (result << 8) | vec[i];
    return result;
}

static inline ihex_record process_line(const string& line) {
    const size_t delim = 1;
    const size_t data_start = 9;
    const size_t check_bytes = 1;
    const size_t min_line_len = data_start + check_bytes * 2;

    if (line.size() < min_line_len || line[0] != ':')
        return { INVALID_LINE_FORMAT, 0, { 0 } };

    const u32 nr_bytes = ihex_byte(line, 1);
    const u16 addr = ihex_byte(line, 3) << 8 | ihex_byte(line, 5);
    const record_type r_type = static_cast<record_type>(ihex_byte(line, 7));
    const size_t line_len = min_line_len + nr_bytes * 2;

    if (line.size() < line_len)
        return { INVALID_LINE_LEN, 0, { 0 } };

    u8 check_sum = 0;
    for (size_t pos = delim; pos < line_len; pos += 2)
        check_sum += ihex_byte(line, pos);
    if (check_sum != 0x00)
        return { INVALID_CHECKSUM, 0, {} };

    switch (r_type) {
    case IHEX_DATA:
        break;
    case IHEX_EOF:
        if (nr_bytes != 0x00 || addr != 0x0000)
            return { INVALID_DESCRIPTOR, 0, {} };
        return { IHEX_EOF, 0, {} };
    case IHEX_EX_SEG:
    case IHEX_EX_LIN_ADDR:
        if (nr_bytes != 0x02 || addr != 0x0000)
            return { INVALID_DESCRIPTOR, 0, {} };
        break;
    case IHEX_START_SEG:
    case IHEX_START_LIN_ADDR:
        if (nr_bytes != 0x04 || addr != 0x0000)
            return { INVALID_DESCRIPTOR, 0, {} };
        break;
    default:
        return { INVALID_TYPE, 0, {} };
    }

    vector<u8> data;
    data.reserve(nr_bytes);
    for (size_t pos = data_start; pos < data_start + nr_bytes * 2; pos += 2)
        data.push_back(ihex_byte(line, pos));
    return { r_type, addr, std::move(data) };
}

ihex::ihex(const string& filename): m_start_addr(), m_records() {
    ifstream file(filename);
    MWR_ERROR_ON(!file, "Cannot open ihex file '%s'", filename.c_str());

    u64 seg_offset = 0;
    u64 linear_offset = 0;
    string line;
    while (getline(file, line)) {
        line = trim(line);
        ihex_record ihex_rec = process_line(line);

        switch (ihex_rec.type) {
        case IHEX_DATA: {
            record rec = {};
            rec.addr = ihex_rec.addr + seg_offset + linear_offset;
            rec.data = std::move(ihex_rec.data);
            m_records.push_back(std::move(rec));
            break;
        };
        case IHEX_EOF:
            return;
        case IHEX_EX_SEG:
            seg_offset = vec_to<u16>(ihex_rec.data) << 4;
            break;
        case IHEX_START_SEG:
        case IHEX_START_LIN_ADDR:
            m_start_addr = vec_to<u32>(ihex_rec.data);
            break;
        case IHEX_EX_LIN_ADDR:
            linear_offset = vec_to<u16>(ihex_rec.data) << 16;
            break;
        default:
            break;
        }
    }
    MWR_REPORT_ON(m_records.size() == 0,
                  "File '%s' does not seem to be in Intel hex format",
                  filename.c_str());
}

} // namespace mwr
