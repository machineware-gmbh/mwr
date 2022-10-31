/******************************************************************************
 *                                                                            *
 * Copyright 2022 MachineWare GmbH                                            *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *     http://www.apache.org/licenses/LICENSE-2.0                             *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 *                                                                            *
 ******************************************************************************/

#include "testing.h"
#include "mwr/utils/socket.h"

#include <arpa/inet.h>

TEST(socket, server) {
    mwr::socket server(12345);
    EXPECT_EQ(server.port(), 12345);
}

TEST(socket, port_select) {
    mwr::socket server(0);
    EXPECT_NE(server.port(), 0);
}

TEST(socket, rehost) {
    mwr::socket server(0);
    EXPECT_GT(server.port(), 0);
    int port = server.port();
    server.unlisten();
    EXPECT_EQ(server.port(), 0);
    server.listen(port);
    EXPECT_EQ(server.port(), port);
}

TEST(socket, connect) {
    mwr::socket server(0);
    mwr::socket client("::1", server.port());

    server.accept();
    client.send_char('x');
    EXPECT_GT(server.peek(), 0);
    EXPECT_EQ(server.recv_char(), 'x');
    server.send_char('y');
    EXPECT_GT(client.peek(), 0);
    EXPECT_EQ(client.recv_char(), 'y');
}

TEST(socket, connect_v4) {
    mwr::socket server(0);
    mwr::socket client("127.0.0.1", server.port());

    server.accept();
    EXPECT_TRUE(client.is_ipv4());
    EXPECT_FALSE(client.is_ipv6());
    client.send_char('x');
    EXPECT_GT(server.peek(), 0);
    EXPECT_EQ(server.recv_char(), 'x');
    server.send_char('y');
    EXPECT_GT(client.peek(), 0);
    EXPECT_EQ(client.recv_char(), 'y');
}

TEST(socket, send) {
    const char* str = "Hello World";
    char buf[strlen(str) + 1];
    memset(buf, 0, strlen(str) + 1);

    mwr::socket server(0);
    mwr::socket client(server.host(), server.port());

    server.accept();
    server.send(str);
    client.recv(buf, sizeof(buf) - 1);

    EXPECT_EQ(strcmp(str, buf), 0);
}

TEST(socket, async) {
    mwr::socket server;
    mwr::socket client;

    for (int i = 1; i < 4; i++) {
        const char* str = "Hello World";
        char buf[strlen(str) + 1];
        memset(buf, 0, strlen(str) + 1);

        server.listen(0);
        server.accept_async();
        client.connect(server.host(), server.port());

        server.send(str);
        client.recv(buf, sizeof(buf) - 1);
        EXPECT_EQ(strcmp(str, buf), 0);

        server.disconnect();
        client.disconnect();
        server.unlisten();
    }
}

TEST(socket, unlisten) {
    mwr::socket sock(0);
    sock.unlisten();
    EXPECT_FALSE(sock.is_listening());

    sock.listen(0);
    EXPECT_TRUE(sock.is_listening());

    sock.accept_async();
    sock.unlisten();

    EXPECT_THROW(sock.send("test"), mwr::report);
}
