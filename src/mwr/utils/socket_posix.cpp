/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "mwr/utils/socket.h"

namespace mwr {

static bool g_use_ipv4 = []() {
    return getenv("MWR_NO_IPv4") ? false : true;
}();

static bool g_use_ipv6 = []() {
    return getenv("MWR_NO_IPv6") ? false : true;
}();

#define SET_SOCKOPT(s, lvl, opt, set)                                      \
    do {                                                                   \
        int val = (set);                                                   \
        if (setsockopt(s, lvl, opt, (const void*)&val, sizeof(val)))       \
            MWR_REPORT("setsockopt %s failed: %s", #opt, strerror(errno)); \
    } while (0)

struct socket_addr {
    union {
        sockaddr base;
        sockaddr_in ipv4;
        sockaddr_in6 ipv6;
    };

    socket_addr() { memset(&ipv6, 0, sizeof(ipv6)); }
    socket_addr(const sockaddr* addr);
    socket_addr(int family, u16 port);
    void verify() const;

    bool is_ipv4() const {
        verify();
        return base.sa_family == AF_INET;
    }
    bool is_ipv6() const {
        verify();
        return base.sa_family == AF_INET6;
    }

    size_t size() const;
    string host() const;
    u16 port() const;
    string peer() const;
};

socket_addr::socket_addr(const sockaddr* addr): socket_addr() {
    switch (addr->sa_family) {
    case AF_INET:
        memcpy(&ipv4, addr, sizeof(ipv4));
        break;
    case AF_INET6:
        memcpy(&ipv6, addr, sizeof(ipv6));
        break;
    default:
        MWR_ERROR("accept: unknown protocol family %d", addr->sa_family);
    }
}

socket_addr::socket_addr(int family, u16 port): socket_addr() {
    switch (family) {
    case AF_INET:
#ifdef MWR_MACOS
        ipv4.sin_len = sizeof(ipv4);
#endif
        ipv4.sin_family = AF_INET;
        ipv4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        ipv4.sin_port = htons(port);
        break;

    case AF_INET6:
#ifdef MWR_MACOS
        ipv6.sin6_len = sizeof(ipv6);
#endif
        ipv6.sin6_family = AF_INET6;
        ipv6.sin6_addr = in6addr_loopback;
        ipv6.sin6_port = htons(port);
        break;

    default:
        MWR_ERROR("accept: unknown protocol family %d", family);
    }
}

void socket_addr::verify() const {
    if (base.sa_family != AF_INET && base.sa_family != AF_INET6)
        MWR_ERROR("accept: unknown protocol family %d", base.sa_family);
}

size_t socket_addr::size() const {
    switch (base.sa_family) {
    case AF_INET:
        return sizeof(ipv4);
    case AF_INET6:
        return sizeof(ipv6);
    default:
        MWR_ERROR("accept: unknown protocol family %d", (int)base.sa_family);
    }
}

string socket_addr::host() const {
    switch (base.sa_family) {
    case AF_INET: {
        char str[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &ipv4.sin_addr, str, sizeof(str));
        return str;
    }

    case AF_INET6: {
        char str[INET6_ADDRSTRLEN] = {};
        inet_ntop(AF_INET6, &ipv6.sin6_addr, str, sizeof(str));
        return str;
    }

    default:
        return "unknown";
    }
}

u16 socket_addr::port() const {
    switch (base.sa_family) {
    case AF_INET:
        return ntohs(ipv4.sin_port);
    case AF_INET6:
        return ntohs(ipv6.sin6_port);
    default:
        return 0;
    }
}

string socket_addr::peer() const {
    return mkstr("%s:%hu", host().c_str(), port());
}

static void create_socket(int family, int& socket, u16& port) {
    socket = ::socket(family, SOCK_STREAM, 0);
    if (socket < 0)
        MWR_REPORT("failed to create socket: %s", strerror(errno));

    SET_SOCKOPT(socket, SOL_SOCKET, SO_REUSEADDR, 1);

#ifdef MWR_MACOS
    SET_SOCKOPT(socket, SOL_SOCKET, SO_REUSEPORT, 1);
#endif

    if (family == AF_INET6)
        SET_SOCKOPT(socket, IPPROTO_IPV6, IPV6_V6ONLY, 1);

    if (port > 0) {
        socket_addr addr(family, port);
        if (::bind(socket, &addr.base, addr.size())) {
            MWR_REPORT("binding socket to port %hu failed: %s", port,
                       strerror(errno));
        }
    }

    if (::listen(socket, 1))
        MWR_REPORT("listen for connections failed: %s", strerror(errno));

    if (port == 0) {
        socket_addr addr;
        socklen_t len = sizeof(addr);
        if (getsockname(socket, &addr.base, &len) < 0)
            MWR_ERROR("getsockname failed: %s", strerror(errno));
        port = addr.port();
    }
}

static void close_socket(int& socket) {
    if (socket >= 0) {
        shutdown(socket, SHUT_RDWR);
#ifndef MWR_LINUX
        // rhel8 locks up accepting connections when we close the socket on
        // Linux, but MacOS needs this to unblock threads stuck accepting
        close(socket);
#endif
        socket = -1;
    }
}

void socket::disconnect_locked() {
    if (m_conn < 0)
        return;

    close_socket(m_conn);
    m_peer.clear();
}

bool socket::is_listening() const {
    lock_guard<mutex> guard(m_mtx);
    return m_sock4 >= 0 || m_sock6 >= 0;
}

bool socket::is_connected() const {
    lock_guard<mutex> guard(m_mtx);
    return m_conn >= 0;
}

socket::socket():
    m_mtx(),
    m_host(),
    m_peer(),
    m_ipv6(),
    m_port(0),
    m_sock4(-1),
    m_sock6(-1),
    m_conn(-1) {
}

socket::socket(u16 port): socket() {
    listen(port);
}

socket::socket(const string& host, u16 port): socket() {
    connect(host, port);
}

socket::~socket() {
    lock_guard<mutex> guard(m_mtx);
    close_socket(m_sock4);
    close_socket(m_sock6);
    close_socket(m_conn);
}

void socket::listen(u16 port) {
    lock_guard<mutex> guard(m_mtx);
    if ((m_sock4 >= 0 || m_sock6 >= 0) && (port == 0 || port == m_port))
        return;

    close_socket(m_sock4);
    close_socket(m_sock6);

    m_host.clear();
    m_port = 0;

    if (!g_use_ipv4 && !g_use_ipv6)
        MWR_REPORT("IPv4 and IPv6 both disabled via environment");

    if (g_use_ipv4)
        create_socket(AF_INET, m_sock4, port);
    if (g_use_ipv6)
        create_socket(AF_INET6, m_sock6, port);

    m_host = "localhost";
    m_port = port;
}

void socket::unlisten() {
    lock_guard<mutex> guard(m_mtx);
    close_socket(m_sock4);
    close_socket(m_sock6);
    m_host.clear();
    m_port = 0;
}

bool socket::accept() {
    lock_guard<mutex> guard(m_mtx);
    if (m_conn >= 0)
        disconnect_locked();

    socket_addr addr;
    socklen_t len = sizeof(addr);

    while (m_sock4 >= 0 || m_sock6 >= 0) {
        socket_t s4 = m_sock4;
        socket_t s6 = m_sock6;

        vector<pollfd> pollfds;
        if (s4 >= 0)
            pollfds.push_back({ s4, POLLIN | POLLERR | POLLHUP, 0 });
        if (s6 >= 0)
            pollfds.push_back({ s6, POLLIN | POLLERR | POLLHUP, 0 });

        m_mtx.unlock();
        int res = poll(pollfds.data(), pollfds.size(), 100);
        m_mtx.lock();

        if (res < 0)
            MWR_REPORT("failed to accept connection: %s", strerror(errno));

        for (const pollfd& poll : pollfds) {
            if (poll.revents & POLLIN) {
                m_conn = ::accept(poll.fd, &addr.base, &len);
                if (m_conn >= 0) {
                    SET_SOCKOPT(m_conn, IPPROTO_TCP, TCP_NODELAY, 1);
                    m_peer = addr.peer();
                    m_ipv6 = poll.fd == s6;
                    return true;
                }
            }
        }
    }

    return false;
}

void socket::connect(const string& host, u16 port) {
    lock_guard<mutex> guard(m_mtx);
    if (m_conn >= 0)
        disconnect_locked();

    string pstr = to_string(port);

    addrinfo hint = {};
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = IPPROTO_TCP;

    addrinfo *ai, *info;
    int err = getaddrinfo(host.c_str(), pstr.c_str(), &hint, &info);
    MWR_REPORT_ON(err, "getaddrinfo failed: %s", gai_strerror(err));
    if (info->ai_family != AF_INET && info->ai_family != AF_INET6)
        MWR_ERROR("getaddrinfo: protocol family %d", info->ai_family);

    for (ai = info; ai != nullptr; ai = ai->ai_next) {
        m_conn = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (m_conn < 0) {
            freeaddrinfo(info);
            MWR_REPORT("failed to create socket: %s", strerror(errno));
        }

        if (::connect(m_conn, ai->ai_addr, ai->ai_addrlen) < 0) {
            close_socket(m_conn);
            continue;
        }

        socket_addr addr(ai->ai_addr);
        m_ipv6 = addr.is_ipv6();
        m_peer = addr.peer();
        break;
    }

    freeaddrinfo(info);
    MWR_REPORT_ON(m_peer.empty(), "connect failed: %s", strerror(errno));
    SET_SOCKOPT(m_conn, IPPROTO_TCP, TCP_NODELAY, 1);
}

void socket::disconnect() {
    lock_guard<mutex> guard(m_mtx);
    disconnect_locked();
}

size_t socket::peek(time_t timeoutms) {
    m_mtx.lock();
    socket_t conn = m_conn;
    m_mtx.unlock();

    if (conn < 0)
        return 0;

    size_t count = mwr::fd_peek(conn, timeoutms);
    if (count == 0)
        return 0;

    m_mtx.lock();
    conn = m_conn;
    m_mtx.unlock();

    char buf[32];
    int err = ::recv(conn, buf, sizeof(buf), MSG_PEEK | MSG_DONTWAIT);
    if (err <= 0)
        disconnect();

    MWR_REPORT_ON(err == 0, "error receiving data: disconnected");
    MWR_REPORT_ON(err < 0, "error receiving data: %s", strerror(errno));

    return err;
}

void socket::send(const void* data, size_t size) {
    m_mtx.lock();
    socket_t conn = m_conn;
    m_mtx.unlock();
    MWR_REPORT_ON(conn < 0, "error sending data: not connected");

    const u8* ptr = (const u8*)data;
    size_t n = 0;

    while (n < size) {
        m_mtx.lock();
        conn = m_conn;
        m_mtx.unlock();

        int r = ::send(conn, ptr + n, size - n, MSG_NOSIGNAL);
        if (r <= 0)
            disconnect_locked();

        MWR_REPORT_ON(r == 0, "error sending data: disconnected");
        MWR_REPORT_ON(r < 0, "error sending data: %s", strerror(errno));

        n += r;
    }
}

void socket::recv(void* data, size_t size) {
    m_mtx.lock();
    socket_t conn = m_conn;
    m_mtx.unlock();
    MWR_REPORT_ON(conn < 0, "error sending data: not connected");

    u8* ptr = (u8*)data;
    size_t n = 0;

    while (n < size) {
        m_mtx.lock();
        conn = m_conn;
        m_mtx.unlock();

        int r = ::recv(conn, ptr + n, size - n, 0);
        if (r <= 0)
            disconnect_locked();

        MWR_REPORT_ON(r == 0, "error receiving data: disconnected");
        MWR_REPORT_ON(r < 0, "error receiving data: %s", strerror(errno));

        n += r;
    }
}

} // namespace mwr
