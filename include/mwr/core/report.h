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

class report : public std::exception
{
private:
    string m_message;
    string m_file;
    size_t m_line;
    vector<string> m_backtrace;
    string m_desc;

public:
    const char* message() const { return m_message.c_str(); }
    const char* file() const { return m_file.c_str(); }
    size_t line() const { return m_line; }
    const vector<string>& backtrace() const { return m_backtrace; }

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
        fprintf(stderr, "%s:%d ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                  \
        fprintf(stderr, "\n");                         \
        fflush(stderr);                                \
        abort();                                       \
    } while (0)

#define MWR_ERROR_ON(cond, ...)     \
    do {                            \
        if (mwr::unlikely(cond)) {  \
            MWR_ERROR(__VA_ARGS__); \
        }                           \
    } while (0)

} // namespace mwr

#endif
