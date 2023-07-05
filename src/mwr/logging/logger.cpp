/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/logging/logger.h"

namespace mwr {

logger log; // global default logger

void logger::vlog(log_level lvl, const char* file, int line,
                  const char* format, va_list args) const {
    if (can_log(lvl)) {
        string msg = vmkstr(format, args);
        publisher::publish(lvl, m_name, msg, file, line);
    }
}

logger::logger(): m_name(), m_lvl(LOG_DEBUG) {
    // nothing to do
}

logger::logger(const string& name): m_name(name), m_lvl(LOG_DEBUG) {
    // nothing to do
}

logger::logger(const string& name, log_level lvl): m_name(name), m_lvl(lvl) {
    // nothing to do
}

void logger::error(const char* format, ...) const {
    va_list args;
    va_start(args, format);
    vlog(LOG_ERROR, nullptr, -1, format, args);
    va_end(args);
}

void logger::warn(const char* format, ...) const {
    va_list args;
    va_start(args, format);
    vlog(LOG_WARN, nullptr, -1, format, args);
    va_end(args);
}

void logger::info(const char* format, ...) const {
    va_list args;
    va_start(args, format);
    vlog(LOG_INFO, nullptr, -1, format, args);
    va_end(args);
}

void logger::debug(const char* format, ...) const {
    va_list args;
    va_start(args, format);
    vlog(LOG_DEBUG, nullptr, -1, format, args);
    va_end(args);
}

void logger::error(const char* src, int line, const char* fmt, ...) const {
    va_list args;
    va_start(args, fmt);
    vlog(LOG_ERROR, src, line, fmt, args);
    va_end(args);
}

void logger::warn(const char* src, int line, const char* fmt, ...) const {
    va_list args;
    va_start(args, fmt);
    vlog(LOG_WARN, src, line, fmt, args);
    va_end(args);
}

void logger::info(const char* src, int line, const char* fmt, ...) const {
    va_list args;
    va_start(args, fmt);
    vlog(LOG_INFO, src, line, fmt, args);
    va_end(args);
}

void logger::debug(const char* src, int line, const char* fmt, ...) const {
    va_list args;
    va_start(args, fmt);
    vlog(LOG_DEBUG, src, line, fmt, args);
    va_end(args);
}

void logger::error(const std::exception& ex) const {
    if (can_log(LOG_ERROR))
        publisher::publish(LOG_ERROR, m_name, ex);
}

void logger::warn(const std::exception& ex) const {
    if (can_log(LOG_WARN))
        publisher::publish(LOG_WARN, m_name, ex);
}

void logger::info(const std::exception& ex) const {
    if (can_log(LOG_INFO))
        publisher::publish(LOG_INFO, m_name, ex);
}

void logger::debug(const std::exception& ex) const {
    if (can_log(LOG_DEBUG))
        publisher::publish(LOG_DEBUG, m_name, ex);
}

void logger::error(const report& rep) const {
    if (can_log(LOG_ERROR))
        publisher::publish(LOG_ERROR, m_name, rep);
}

void logger::warn(const report& rep) const {
    if (can_log(LOG_WARN))
        publisher::publish(LOG_WARN, m_name, rep);
}

void logger::info(const report& rep) const {
    if (can_log(LOG_INFO))
        publisher::publish(LOG_INFO, m_name, rep);
}

void logger::debug(const report& rep) const {
    if (can_log(LOG_DEBUG))
        publisher::publish(LOG_DEBUG, m_name, rep);
}

} // namespace mwr
