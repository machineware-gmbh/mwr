/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_LOGGING_LOGGER_H
#define MWR_LOGGING_LOGGER_H

#include "mwr/core/sfinae.h"
#include "mwr/logging/publisher.h"

namespace mwr {

class logger
{
private:
    string m_name;
    log_level m_lvl;

    void vlog(log_level lvl, const char* file, int line, const char* format,
              va_list args) const;

public:
    const char* name() const { return m_name.c_str(); }
    void set_name(const string& name) { m_name = name; }

    log_level level() const { return m_lvl; }
    void set_level(log_level lvl) { m_lvl = lvl; }

    virtual bool can_log(log_level lvl) const;

    logger();
    logger(const string& name);
    logger(const string& name, log_level lvl);

    logger(logger&&) = default;
    logger(const logger&) = default;
    virtual ~logger() = default;

    void log(log_level lvl, const char* format, ...) const
        MWR_DECL_PRINTF(3, 4);
    void log(log_level lvl, const char* file, int line, const char* format,
             ...) const MWR_DECL_PRINTF(5, 6);

    void error(const char* format, ...) const MWR_DECL_PRINTF(2, 3);
    void warn(const char* format, ...) const MWR_DECL_PRINTF(2, 3);
    void info(const char* format, ...) const MWR_DECL_PRINTF(2, 3);
    void debug(const char* format, ...) const MWR_DECL_PRINTF(2, 3);

    void error(const char* file, int line, const char* format, ...) const
        MWR_DECL_PRINTF(4, 5);
    void warn(const char* file, int line, const char* format, ...) const
        MWR_DECL_PRINTF(4, 5);
    void info(const char* file, int line, const char* format, ...) const
        MWR_DECL_PRINTF(4, 5);
    void debug(const char* file, int line, const char* format, ...) const
        MWR_DECL_PRINTF(4, 5);

    void error(const std::exception& ex) const;
    void warn(const std::exception& ex) const;
    void info(const std::exception& ex) const;
    void debug(const std::exception& ex) const;

    void error(const report& rep) const;
    void warn(const report& rep) const;
    void info(const report& rep) const;
    void debug(const report& rep) const;
};

inline bool logger::can_log(log_level lvl) const {
    return lvl <= level() && publisher::can_publish(lvl);
}

extern logger log;

template <typename T>
inline const enable_if_base_of_t<logger, T>& select_logger(T&& l) {
    return l;
}

inline const logger& select_logger(...) {
    return ::mwr::log;
}

#define MWR_LOG(lvl, ...)                                   \
    do {                                                    \
        const auto& _log = ::mwr::select_logger(log);       \
        if (_log.can_log(lvl))                              \
            _log.log(lvl, __FILE__, __LINE__, __VA_ARGS__); \
    } while (0)

#define MWR_LOG_ERROR(...) MWR_LOG(::mwr::LOG_ERROR, __VA_ARGS__)
#define MWR_LOG_WARN(...)  MWR_LOG(::mwr::LOG_WARN, __VA_ARGS__)
#define MWR_LOG_INFO(...)  MWR_LOG(::mwr::LOG_INFO, __VA_ARGS__)
#define MWR_LOG_DEBUG(...) MWR_LOG(::mwr::LOG_DEBUG, __VA_ARGS__)

#define MWR_LOG_ONCE(lvl, ...)         \
    do {                               \
        static int once = 0;           \
        if (once == 0) {               \
            MWR_LOG(lvl, __VA_ARGS__); \
            once = 1;                  \
        }                              \
    } while (0)

#define MWR_LOG_ERROR_ONCE(...) MWR_LOG_ONCE(::mwr::LOG_ERROR, __VA_ARGS__)
#define MWR_LOG_WARN_ONCE(...)  MWR_LOG_ONCE(::mwr::LOG_WARN, __VA_ARGS__)
#define MWR_LOG_INFO_ONCE(...)  MWR_LOG_ONCE(::mwr::LOG_INFO, __VA_ARGS__)
#define MWR_LOG_DEBUG_ONCE(...) MWR_LOG_ONCE(::mwr::LOG_DEBUG, __VA_ARGS__)

#ifndef MWR_NO_LOG_MACROS
// NOLINTBEGIN(readability-identifier-naming)
#define log_error(...)      MWR_LOG_ERROR(__VA_ARGS__)
#define log_warn(...)       MWR_LOG_WARN(__VA_ARGS__)
#define log_info(...)       MWR_LOG_INFO(__VA_ARGS__)
#define log_debug(...)      MWR_LOG_DEBUG(__VA_ARGS__)
#define log_error_once(...) MWR_LOG_ERROR_ONCE(__VA_ARGS__)
#define log_warn_once(...)  MWR_LOG_WARN_ONCE(__VA_ARGS__)
#define log_info_once(...)  MWR_LOG_INFO_ONCE(__VA_ARGS__)
#define log_debug_once(...) MWR_LOG_DEBUG_ONCE(__VA_ARGS__)
// NOLINTEND(readability-identifier-naming)
#endif

} // namespace mwr

#endif
