/******************************************************************************
 *                                                                            *
 * Copyright (C) 2025 MachineWare GmbH                                        *
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

TEST(server_socket, server) {
    mwr::server_socket server(1);
    server.listen(0);
    EXPECT_NE(server.port(), 0);
    EXPECT_STRNE(server.host(), "");
    EXPECT_EQ(server.num_clients(), 0);
    printf("server on %s port %hu\n", server.host(), server.port());
}

TEST(server_socket, server_ipv6_only) {
    mwr::server_socket server(1);
    server.set_ipv6_only(true);
    server.listen(0);
    EXPECT_NE(server.port(), 0);
    EXPECT_STRNE(server.host(), "");
    EXPECT_EQ(server.num_clients(), 0);
    printf("server on %s port %hu\n", server.host(), server.port());
}

static void worker_thread(mwr::u16 port) {
    try {
        mwr::socket client;
        client.connect("localhost", port);
        while (client.is_connected()) {
            char x = client.recv_char();
            if (x == 'X')
                return;
            client.send_char(x);
        }
    } catch (...) {
        return;
    }
}

TEST(server_socket, one_client) {
    mwr::server_socket server(1, 0, "localhost");
    std::thread worker(worker_thread, server.port());

    // poll until we have a connection, maximum 10 seconds
    for (int i = 0; i < 100 && server.num_clients() == 0; i++) {
        int client = server.poll(100);
        EXPECT_EQ(client, -1);
    }

    ASSERT_EQ(server.num_clients(), 1);

    server.send(0, "hello world");

    int client = server.poll(100);
    ASSERT_EQ(client, 0);

    char buffer[12]{};
    server.recv(client, buffer, 11);
    EXPECT_STREQ(buffer, "hello world");

    server.send_char(client, 'X');
    worker.join();

    EXPECT_THROW({ server.recv_char(client); }, std::exception);
    EXPECT_EQ(server.num_clients(), 0);
}

TEST(server_socket, disconnect_on_error) {
    mwr::server_socket server(2, 0, "localhost");

    size_t num_disconnects = 0;
    server.on_disconnect([&num_disconnects](int client) {
        EXPECT_LT(client, 2);
        num_disconnects++;
    });

    std::thread worker1(worker_thread, server.port());
    std::thread worker2(worker_thread, server.port());

    // poll until we have a connection, maximum 10 seconds
    for (int i = 0; i < 100 && server.num_clients() < 2; i++) {
        int client = server.poll(100);
        ASSERT_EQ(client, -1);
    }

    ASSERT_EQ(server.clients().size(), 2);

    server.send(server.clients()[0], "X");
    server.send(server.clients()[1], "X");

    worker1.join();
    worker2.join();

    EXPECT_THROW({ server.recv_char(0); }, std::exception);
    EXPECT_THROW({ server.recv_char(1); }, std::exception);
    EXPECT_EQ(server.num_clients(), 0);
    EXPECT_TRUE(server.clients().empty());
    ASSERT_EQ(num_disconnects, 2);
}

TEST(server_socket, filter_accept) {
    mwr::server_socket server(1, 0, "localhost");

    server.on_connect([](int client, const std::string& host,
                         mwr::u16 port) -> bool { return false; });

    std::thread worker(worker_thread, server.port());

    int client = server.poll(5000);
    EXPECT_EQ(client, -1);
    ASSERT_EQ(server.num_clients(), 0);

    worker.join();
}

TEST(server_socket, max_connections) {
    mwr::server_socket server(2, 0, "localhost");

    std::thread worker1(worker_thread, server.port());
    std::thread worker2(worker_thread, server.port());
    std::thread worker3(worker_thread, server.port());

    int client = server.poll(5000);
    ASSERT_EQ(client, -1);
    ASSERT_EQ(server.num_clients(), 2);

    for (int client : server.clients())
        server.send_char(client, 'X');

    worker1.join();
    worker2.join();
    worker3.join();
}
