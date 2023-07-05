/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "testing.h"
#include "mwr.h"

using namespace testing;

MATCHER_P(match_level, level, "matches level of log message") {
    return !arg.lines.empty() && arg.level == level;
}

MATCHER_P(match_lines, count, "matches log message line count") {
    return arg.lines.size() == (size_t)count;
}

MATCHER_P(match_sender, name, "matches log message sender") {
    return !arg.lines.empty() && arg.sender == name;
}

MATCHER(match_source, "checks if log message source information makes sense") {
    return !arg.lines.empty() && arg.source.file &&
           strcmp(arg.source.file, __FILE__) == 0 && arg.source.line != -1;
}

mwr::publishers::terminal g_terminal;

class mock_publisher : public mwr::publisher
{
public:
    mock_publisher(): mwr::publisher(mwr::LOG_ERROR, mwr::LOG_INFO) {}
    MOCK_METHOD(void, publish, (const mwr::logmsg&), (override));
};

TEST(publisher, levels) {
    mock_publisher publisher;
    EXPECT_CALL(publisher, publish(match_level(mwr::LOG_INFO))).Times(1);
    mwr::log_info("this is an informational message");

    publisher.set_level(mwr::LOG_ERROR, mwr::LOG_WARN);
    g_terminal.set_level(mwr::LOG_ERROR, mwr::LOG_WARN);
    EXPECT_TRUE(mwr::publisher::can_publish(mwr::LOG_ERROR));
    EXPECT_TRUE(mwr::publisher::can_publish(mwr::LOG_WARN));
    EXPECT_FALSE(mwr::publisher::can_publish(mwr::LOG_INFO));
    EXPECT_FALSE(mwr::publisher::can_publish(mwr::LOG_DEBUG));
    EXPECT_CALL(publisher, publish(_)).Times(0);
    mwr::log_info("this is an informational message");

    EXPECT_CALL(publisher, publish(match_level(mwr::LOG_ERROR))).Times(1);
    EXPECT_CALL(publisher, publish(match_level(mwr::LOG_WARN))).Times(1);
    mwr::log_error("this is an error message");
    mwr::log_warn("this is a warning message");

    publisher.set_level(mwr::LOG_DEBUG, mwr::LOG_DEBUG);
    g_terminal.set_level(mwr::LOG_DEBUG, mwr::LOG_DEBUG);
    EXPECT_FALSE(mwr::publisher::can_publish(mwr::LOG_ERROR));
    EXPECT_FALSE(mwr::publisher::can_publish(mwr::LOG_WARN));
    EXPECT_FALSE(mwr::publisher::can_publish(mwr::LOG_INFO));
    EXPECT_TRUE(mwr::publisher::can_publish(mwr::LOG_DEBUG));
    EXPECT_CALL(publisher, publish(match_level(mwr::LOG_DEBUG))).Times(1);
    mwr::log_debug("this is a debug message");
    mwr::log_info("this is an informational message");
    mwr::log_error("this is an error message");
    mwr::log_warn("this is a warning message");

    EXPECT_CALL(publisher, publish(match_lines(3))).Times(1);
    mwr::log_debug("multi\nline\nmessage");

    EXPECT_CALL(publisher, publish(match_source())).Times(1);
    mwr::log_debug("does this message hold source info?");
}

TEST(publisher, reporting) {
    mock_publisher publisher;
    mwr::report rep("This is an error report", __FILE__, __LINE__);
    EXPECT_CALL(publisher, publish(match_level(mwr::LOG_ERROR))).Times(1);
    mwr::log.error(rep);
}

class wrapper
{
public:
    mwr::logger log;
    wrapper(): log("mylog", mwr::LOG_DEBUG) {}

    void test() {
        log_info("log");
        log.info("log");
    }
};

TEST(publisher, nested) {
    mock_publisher publisher;
    EXPECT_CALL(publisher, publish(match_sender("mylog"))).Times(2);

    wrapper w;
    w.test();
}
