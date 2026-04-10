/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
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
