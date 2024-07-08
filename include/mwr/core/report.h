/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_REPORT_H
#define MWR_REPORT_H

#include "mwr/stl/strings.h"
#include "mwr/stl/streams.h"
#include "mwr/stl/containers.h"

namespace mwr {

struct stackframe {
    u64 address;
    string symbol;
    u64 offset;
};

ostream& operator<<(ostream& os, const stackframe& frame);

extern size_t max_backtrace_length;

vector<stackframe> backtrace(size_t frames, size_t skip);
vector<stackframe> backtrace(size_t skip = 2);

void print_backtrace(const vector<stackframe>& bt, ostream& os);
void print_backtrace(ostream& os);

void report_segfaults();

class report : public std::exception
{
private:
    string m_message;
    string m_file;
    size_t m_line;
    vector<stackframe> m_backtrace;
    string m_desc;

public:
    const char* message() const { return m_message.c_str(); }
    const char* file() const { return m_file.c_str(); }
    size_t line() const { return m_line; }
    const vector<stackframe>& backtrace() const { return m_backtrace; }

    report() = delete;
    report(const string& msg, const char* file, size_t line);
    virtual ~report() throw();

    virtual const char* what() const throw();
};

ostream& operator<<(ostream& os, const report& rep);

#define MWR_REPORT(...) \
    throw ::mwr::report(::mwr::mkstr(__VA_ARGS__), __FILE__, __LINE__)

#define MWR_REPORT_ON(condition, ...) \
    do {                              \
        if (condition) {              \
            MWR_REPORT(__VA_ARGS__);  \
        }                             \
    } while (0)

#define MWR_REPORT_ONCE(...)             \
    do {                                 \
        static bool report_done = false; \
        if (!report_done) {              \
            report_done = true;          \
            MWR_REPORT(__VA_ARGS__);     \
        }                                \
    } while (0)

#define MWR_ERROR(...)                                 \
    do {                                               \
        ::mwr::print_backtrace(std::cerr);             \
        fprintf(stderr, "%s:%d ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                  \
        fprintf(stderr, "\n");                         \
        fflush(stderr);                                \
        ::exit(1);                                     \
    } while (0)

#define MWR_ERROR_ON(cond, ...)      \
    do {                             \
        if (::mwr::unlikely(cond)) { \
            MWR_ERROR(__VA_ARGS__);  \
        }                            \
    } while (0)

} // namespace mwr

#endif
