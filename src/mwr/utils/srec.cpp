/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/utils/srec.h"

namespace mwr {

enum record_type : char {
    SREC_HEADER = '0',
    SREC_DATA16 = '1',
    SREC_DATA24 = '2',
    SREC_DATA32 = '3',
    SREC_DATA64 = '4',
    SREC_TEXT32 = '7',
    SREC_TEXT24 = '8',
    SREC_TEXT16 = '9',
};

static u8 srec_byte(const string& line, size_t off) {
    MWR_ERROR_ON(off + 1 >= line.length(), "reading beyond srec line");
    return from_hex_ascii(line[off]) << 4 | from_hex_ascii(line[off + 1]);
}

srec::srec(const string& filename): m_entry(), m_header(), m_records() {
    ifstream file(filename);
    MWR_ERROR_ON(!file, "cannot open srec file '%s'", filename.c_str());

    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.length() < 2 || line[0] != 'S')
            continue;

        size_t delim = 0;
        switch (line[1]) {
        case SREC_HEADER:
        case SREC_DATA16:
        case SREC_TEXT16:
            delim = 8;
            break;
        case SREC_DATA24:
        case SREC_TEXT24:
            delim = 10;
            break;
        case SREC_DATA32:
        case SREC_TEXT32:
            delim = 12;
            break;
        case SREC_DATA64:
            delim = 20;
            break;
        default:
            continue;
        }

        if (line.length() < delim)
            continue;

        record rec = {};

        for (size_t pos = 4; pos < delim; pos += 2)
            rec.addr = rec.addr << 8 | srec_byte(line, pos);

        for (size_t pos = delim; pos < line.length() - 2; pos += 2)
            rec.data.push_back(srec_byte(line, pos));

        switch (line[1]) {
        case SREC_HEADER:
            m_header = trim(string(rec.data.begin(), rec.data.end()));
            break;
        case SREC_TEXT16:
        case SREC_TEXT24:
        case SREC_TEXT32:
            m_entry = rec.addr;
            break;
        case SREC_DATA16:
        case SREC_DATA24:
        case SREC_DATA32:
        case SREC_DATA64:
            m_records.push_back(std::move(rec));
            break;
        }
    }
}

} // namespace mwr
