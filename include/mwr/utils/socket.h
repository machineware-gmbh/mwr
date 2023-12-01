/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_SOCKET_H
#define MWR_UTILS_SOCKET_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"
#include "mwr/core/compiler.h"
#include "mwr/core/utils.h"

#include "mwr/stl/strings.h"
#include "mwr/stl/streams.h"
#include "mwr/stl/threads.h"

namespace mwr {

class socket
{
private:
#ifdef MWR_WINDOWS
    using socket_t = unsigned long long;
#else
    using socket_t = int;
#endif

    string m_host;
    string m_peer;
    bool m_ipv6;
    u16 m_port;

    atomic<socket_t> m_socket;
    atomic<socket_t> m_conn;

public:
    u16 port() const { return m_port; }
    const char* host() const { return m_host.c_str(); }
    const char* peer() const { return m_peer.c_str(); }

    bool is_ipv4() const { return !m_ipv6; }
    bool is_ipv6() const { return m_ipv6; }

    bool is_listening() const;
    bool is_connected() const;

    socket();
    socket(u16 port);
    socket(const string& host, u16 port);
    virtual ~socket();

    socket(socket&&) = delete;
    socket(const socket&) = delete;
    socket& operator=(const socket&) = delete;

    void listen(u16 port);
    void unlisten();

    bool accept();

    void connect(const string& host, u16 port);
    void disconnect();

    size_t peek(time_t timeoutms = 0);

    void send_char(int c);
    int recv_char();

    void send(const void* data, size_t size);
    void recv(void* data, size_t size);

    void send(const string& str);
    void send(const char* str);

    template <typename T>
    void send(const T& data);
    template <typename T>
    void recv(T& data);
};

inline void socket::send_char(int c) {
    char x = (char)c;
    send(&x, sizeof(x));
}

inline int socket::recv_char() {
    char x = 0;
    recv(&x, sizeof(x));
    return x;
}

inline void socket::send(const string& str) {
    send(str.c_str(), str.length());
}

inline void socket::send(const char* str) {
    send(str, strlen(str));
}

template <typename T>
inline void socket::send(const T& data) {
    send(&data, sizeof(data));
}

template <typename T>
inline void socket::recv(T& data) {
    recv(&data, sizeof(data));
}

} // namespace mwr

#endif
