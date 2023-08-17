/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_LOGGING_PUBLISHER_H
#define MWR_LOGGING_PUBLISHER_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"
#include "mwr/core/compiler.h"
#include "mwr/core/utils.h"

#include "mwr/stl/strings.h"
#include "mwr/stl/streams.h"
#include "mwr/stl/threads.h"
#include "mwr/stl/containers.h"

namespace mwr {

enum log_level {
    LOG_ERROR = 0,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
    NUM_LOG_LEVELS,
};

ostream& operator<<(ostream& os, const log_level& lvl);
istream& operator>>(istream& is, log_level& lvl);

struct logmsg {
    log_level level;
    u64 timestamp;
    string sender;

    struct {
        const char* file;
        int line;
    } source;

    vector<string> lines;

    logmsg(log_level level, const string& sender);
};

ostream& operator<<(ostream& os, const logmsg& msg);

typedef function<bool(const logmsg& msg)> log_filter;

class publisher
{
private:
    mutable mutex m_mtx;

    log_level m_min;
    log_level m_max;

    vector<log_filter> m_filters;

    void register_publisher();
    void unregister_publisher();

    bool check_filters(const logmsg& msg) const;
    void do_publish(const logmsg& msg);

    static vector<publisher*>& lookup(log_level lvl);

protected:
    virtual void publish(const logmsg& msg) = 0;

public:
    void set_level(log_level max);
    void set_level(log_level min, log_level max);

    void filter(log_filter filter);
    void filter_time(u64 t0, u64 t1);
    void filter_source(const string& file, int line = -1);

    publisher();
    publisher(log_level max);
    publisher(log_level min, log_level max);
    virtual ~publisher();

    publisher(const publisher&) = delete;
    publisher& operator=(const publisher&) = delete;

    static bool can_publish(log_level lvl);

    static void publish(log_level level, const string& sender,
                        const string& message, const char* file = nullptr,
                        int line = -1);

    static void publish(log_level level, const string& sender,
                        const std::exception& ex);

    static void publish(log_level level, const string& sender,
                        const report& rep);

    static u64 (*current_timestamp)(void);
    static bool print_timestamp;
    static bool print_sender;
    static bool print_source;
    static bool print_backtrace;

    static void print_timing(ostream& os, u64 timestamp);
    static void print_prefix(ostream& os, const logmsg& msg);
    static void print_logmsg(ostream& os, const logmsg& msg);

    static const char* prefix[NUM_LOG_LEVELS];
    static const char* desc[NUM_LOG_LEVELS];
};

inline void publisher::set_level(log_level max) {
    set_level(LOG_ERROR, max);
}

inline bool publisher::can_publish(log_level lvl) {
    MWR_ERROR_ON(lvl >= NUM_LOG_LEVELS, "illegal log level %u", lvl);
    return !lookup(lvl).empty();
}

inline void publisher::filter(log_filter filter) {
    m_filters.push_back(std::move(filter));
}

inline void publisher::filter_time(u64 t0, u64 t1) {
    filter([t0, t1](const logmsg& msg) -> bool {
        return msg.timestamp >= t0 && msg.timestamp < t1;
    });
}

inline void publisher::filter_source(const string& file, int line) {
    filter([file, line](const logmsg& msg) -> bool {
        if (!ends_with(msg.source.file, file))
            return false;
        if (line != -1 && msg.source.line != line)
            return false;
        return true;
    });
}

} // namespace mwr

#endif
