/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file found  *
 * in the root directory of this source tree.                                 *
 *                                                                            *
 ******************************************************************************/

#include <thread>

#include "testing.h"
#include "mwr/utils/socket.h"
#include "mwr/core/utils.h"

TEST(socket, connect) {
    if (mwr::getenv("MWR_NO_IPv6"))
        GTEST_SKIP() << "IPv6 disabled. Skipping IPv6 test.";

    mwr::server_socket server(1);
    server.set_ipv6_only();
    server.listen(0, "::1");
    EXPECT_NE(server.port(), 0);

    mwr::socket client(server.host(), server.port());
    EXPECT_FALSE(client.is_ipv4());
    EXPECT_TRUE(client.is_ipv6());

    client.send_char('x');

    int id = server.poll(1000);
    EXPECT_EQ(id, 0);
    EXPECT_EQ(server.recv_char(id), 'x');
    server.send_char(id, 'y');
    EXPECT_EQ(client.recv_char(), 'y');
}

TEST(socket, connect_v4) {
    mwr::server_socket server(1);
    server.listen(0, "127.0.0.1");
    EXPECT_NE(server.port(), 0);

    mwr::socket client(server.host(), server.port());
    EXPECT_TRUE(client.is_ipv4());
    EXPECT_FALSE(client.is_ipv6());

    client.send_char('x');

    int id = server.poll(1000);
    EXPECT_EQ(id, 0);
    EXPECT_EQ(server.recv_char(id), 'x');
    server.send_char(id, 'y');
    EXPECT_EQ(client.recv_char(), 'y');
}

TEST(socket, send) {
    const char* str = "Hello World";
    char buf[12] = {};
    memset(buf, 0, strlen(str) + 1);

    mwr::server_socket server(1, 0);
    mwr::socket client(server.host(), server.port());

    server.poll(100);
    EXPECT_EQ(server.num_clients(), 1);

    server.send(0, str);
    client.recv(buf, sizeof(buf) - 1);
    EXPECT_EQ(strcmp(str, buf), 0);
    memset(buf, 0, sizeof(buf));

    client.send(str);
    server.recv(0, buf, sizeof(buf) - 1);
    EXPECT_EQ(strcmp(str, buf), 0);
    memset(buf, 0, sizeof(buf));
}

TEST(socket, threads) {
    mwr::server_socket server(1, 0);
    mwr::socket client(server.host(), server.port());
    server.poll(100);
    EXPECT_TRUE(client.is_connected());

    std::thread t([&]() {
        (void)client.port(); // trigger a data race on port
        (void)client.peer(); // trigger a data race on peer
        client.disconnect();
    });

    t.join();
    EXPECT_FALSE(client.is_connected());
    EXPECT_THROW(client.send("test"), mwr::report);
}

TEST(socket, peek) {
    mwr::server_socket server(1, 0);
    mwr::socket client(server.host(), server.port());
    server.poll(100);
    ASSERT_EQ(server.num_clients(), 1);

    // nothing to read, peek must wait for its timeout and return 0
    mwr::u64 start = mwr::timestamp_ms();
    EXPECT_EQ(client.peek(100), 0);
    EXPECT_GE(mwr::timestamp_ms() - start, 90); // allow for timer granularity

    // data arriving while peek is waiting must wake it up
    std::thread t([&]() {
        mwr::usleep(50000);
        server.send(0, "abc");
    });

    EXPECT_GT(client.peek(5000), 0);
    t.join();

    EXPECT_EQ(client.recv_char(), 'a');
    EXPECT_EQ(client.recv_char(), 'b');
    EXPECT_EQ(client.recv_char(), 'c');
    EXPECT_EQ(client.peek(0), 0);

    // peer hang up must be reported
    server.disconnect(0);
    EXPECT_THROW(client.peek(1000), mwr::report);
    EXPECT_FALSE(client.is_connected());
}

TEST(socket, peek_all) {
    mwr::server_socket server(1, 0);
    mwr::socket client(server.host(), server.port());
    server.poll(100);
    ASSERT_EQ(server.num_clients(), 1);

    // peek must report everything that has arrived, not just a few bytes
    std::string data(1000, 'x');
    data.back() = 'y';
    server.send(0, data);

    size_t avail = 0;
    for (int i = 0; i < 100 && avail < data.size(); i++)
        avail = client.peek(100);
    ASSERT_EQ(avail, data.size());

    std::string received(avail, '\0');
    client.recv(received.data(), avail);
    EXPECT_EQ(received, data);
    EXPECT_EQ(client.peek(0), 0);
}

TEST(socket, move) {
    const char* str = "Hello World";
    char buf[12] = {};
    memset(buf, 0, strlen(str) + 1);

    mwr::server_socket server(1, 0);
    mwr::socket client(server.host(), server.port());

    server.poll(100);
    EXPECT_EQ(server.num_clients(), 1);

    server.send(0, str);

    mwr::socket moved(std::move(client));
    moved.recv(buf, sizeof(buf) - 1);
    EXPECT_EQ(strcmp(str, buf), 0);
}

TEST(socket, move_assign) {
    const char* str = "Hello World";
    char buf[12] = {};
    memset(buf, 0, strlen(str) + 1);

    mwr::server_socket server(1, 0);
    mwr::socket client(server.host(), server.port());

    server.poll(100);
    EXPECT_EQ(server.num_clients(), 1);

    server.send(0, str);

    mwr::socket moved = std::move(client);
    moved.recv(buf, sizeof(buf) - 1);
    EXPECT_EQ(strcmp(str, buf), 0);
}
