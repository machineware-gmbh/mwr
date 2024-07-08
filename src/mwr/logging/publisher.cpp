/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/logging/publisher.h"

namespace mwr {

u64 (*publisher::current_timestamp)() = nullptr;
bool publisher::print_timestamp = true;
bool publisher::print_sender = true;
bool publisher::print_source = false;
bool publisher::print_backtrace = true;

const char* publisher::prefix[NUM_LOG_LEVELS] = {
    /* [LOG_ERROR] = */ "E",
    /* [LOG_WARN]  = */ "W",
    /* [LOG_INFO]  = */ "I",
    /* [LOG_DEBUG] = */ "D",
};

const char* publisher::desc[NUM_LOG_LEVELS] = {
    /* [LOG_ERROR] = */ "error",
    /* [LOG_WARN]  = */ "warning",
    /* [LOG_INFO]  = */ "info",
    /* [LOG_DEBUG] = */ "debug",
};

ostream& operator<<(ostream& os, const log_level& lvl) {
    if (lvl < LOG_ERROR || lvl > LOG_DEBUG)
        return os << "unknown log level";
    return os << publisher::desc[lvl];
}

istream& operator>>(istream& is, log_level& lvl) {
    string s;
    is >> s;
    lvl = NUM_LOG_LEVELS;
    for (int i = LOG_ERROR; i < NUM_LOG_LEVELS; i++)
        if (s == publisher::desc[i])
            lvl = (log_level)i;
    if (lvl == NUM_LOG_LEVELS)
        is.setstate(std::ios::failbit);
    return is;
}

ostream& operator<<(ostream& os, const logmsg& msg) {
    publisher::print_logmsg(os, msg);
    return os;
}

static u64 current_timestamp() {
    if (publisher::current_timestamp)
        return publisher::current_timestamp();
    static mwr::u64 start = mwr::timestamp_ms();
    return (mwr::timestamp_ms() - start) * 1000000;
}

logmsg::logmsg(log_level lvl, const string& s):
    level(lvl),
    timestamp(current_timestamp()),
    sender(s),
    source({ "", -1 }),
    lines() {
}

void publisher::register_publisher() {
    for (int l = m_min; l <= m_max; l++)
        stl_add_unique(lookup((log_level)l), this);
}

void publisher::unregister_publisher() {
    for (int l = m_min; l <= m_max; l++)
        stl_remove(lookup((log_level)l), this);
}

bool publisher::check_filters(const logmsg& msg) const {
    if (m_filters.empty())
        return true;

    for (auto& filter : m_filters)
        if (filter(msg))
            return true;

    return false;
}

void publisher::do_publish(const logmsg& msg) {
    lock_guard<mutex> guard(m_mtx);
    publish(msg);
}

vector<publisher*>& publisher::lookup(log_level lvl) {
    MWR_ERROR_ON(lvl >= NUM_LOG_LEVELS, "invalid log level %u", lvl);
    static vector<publisher*> publishers[NUM_LOG_LEVELS];
    return publishers[lvl];
}

void publisher::set_level(log_level min, log_level max) {
    unregister_publisher();
    m_min = min;
    m_max = max;
    register_publisher();
}

publisher::publisher(): publisher(LOG_DEBUG) {
    // nothing to do
}

publisher::publisher(log_level max): publisher(LOG_ERROR, max) {
    // nothing to do
}

publisher::publisher(log_level min, log_level max):
    m_mtx(), m_min(min), m_max(max), m_filters() {
    register_publisher();
}

publisher::~publisher() {
    unregister_publisher();
}

void publisher::publish(log_level level, const string& sender,
                        const string& str, const char* file, int line) {
    logmsg msg(level, sender);
    msg.lines = split(escape(str), '\n');

    if (file != nullptr) {
        msg.source.file = file;
        msg.source.line = line;
    }

    for (auto& logger : lookup(msg.level))
        if (logger->check_filters(msg))
            logger->do_publish(msg);
}

void publisher::publish(log_level level, const string& sender,
                        const report& rep) {
    stringstream ss;

    if (print_backtrace)
        mwr::print_backtrace(rep.backtrace(), ss);

    ss << rep.message();

    // always force printing of source locations of reports
    bool print = print_source;
    print_source = true;
    publish(level, sender, ss.str(), rep.file(), (int)rep.line());
    print_source = print;
}

void publisher::publish(log_level level, const string& sender,
                        const std::exception& ex) {
    string msg = mkstr("exception: %s", ex.what());
    publish(level, sender, msg);
}

void publisher::print_timing(ostream& os, u64 timestamp) {
    if (print_timestamp) {
        u64 seconds = timestamp / 1000000000ull;
        u64 nanosec = timestamp % 1000000000ull;
        os << mkstr(" %llu.%09llu", seconds, nanosec);
    }
}

void publisher::print_prefix(ostream& os, const logmsg& msg) {
    os << "[" << publisher::prefix[msg.level];
    print_timing(os, msg.timestamp);
    os << "]";

    if (print_sender && !msg.sender.empty())
        os << " " << msg.sender << ":";
}

void publisher::print_logmsg(ostream& os, const logmsg& msg) {
    stringstream prefix;
    publisher::print_prefix(prefix, msg);

    if (!msg.lines.empty()) {
        for (size_t i = 0; i < msg.lines.size() - 1; i++)
            os << prefix.str() << " " << msg.lines[i] << std::endl;
        os << prefix.str() << " " << msg.lines.back();
    }

    if (print_source) {
        os << " (from ";
        if (msg.source.file && strlen(msg.source.file))
            os << msg.source.file;
        else
            os << "<unknown>";
        if (msg.source.line > -1)
            os << ":" << msg.source.line;
        os << ")";
    }
}

} // namespace mwr
