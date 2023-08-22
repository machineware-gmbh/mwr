/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/utils/socket.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

#define SET_SOCKOPT(s, lvl, opt, set)                                        \
    do {                                                                     \
        int val = (set);                                                     \
        if (setsockopt(s, lvl, opt, (const char*)&val, sizeof(val)) ==       \
            SOCKET_ERROR)                                                    \
            MWR_REPORT("setsockopt %s failed: %d", #opt, WSAGetLastError()); \
    } while (0)

namespace mwr {

static int socket_default_address_family() {
    if (getenv("MWR_NO_IPv6"))
        return AF_INET;
    return AF_INET6;
}

static void socket_exit() {
    WSACleanup();
}

static void socket_init() {
    static bool done = false;
    if (done)
        return;

    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data)) {
        int err = WSAGetLastError();
        MWR_ERROR("Failed to start winsock2: 0x%x", err);
    }

    atexit(socket_exit);
    done = true;
}

static const char* socket_strerror(DWORD err = WSAGetLastError()) {
    static char buffer[256];
    memset(buffer, 0, sizeof(buffer));

    if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, buffer,
                       sizeof(buffer), NULL)) {
        return buffer;
    }

    snprintf(buffer, sizeof(buffer), "0x%08x", err);
    return buffer;
}

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
        ipv4.sin_family = AF_INET;
        ipv4.sin_addr.s_addr = INADDR_ANY;
        ipv4.sin_port = htons(port);
        break;

    case AF_INET6:
        ipv6.sin6_family = AF_INET6;
        ipv6.sin6_addr = in6addr_any;
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

bool socket::is_listening() const {
    return m_socket != INVALID_SOCKET;
}

bool socket::is_connected() const {
    return m_conn != INVALID_SOCKET;
}

socket::socket():
    m_host(),
    m_peer(),
    m_ipv6(),
    m_port(0),
    m_socket(-1),
    m_conn(-1),
    m_async() {
    socket_init();
}

socket::socket(u16 port): socket() {
    listen(port);
}

socket::socket(const string& host, u16 port): socket() {
    connect(host, port);
}

socket::~socket() {
    if (is_connected())
        disconnect();
    if (is_listening())
        unlisten();
}

void socket::listen(u16 port) {
    if (is_listening() && (port == 0 || port == m_port))
        return;

    unlisten();

    int family = socket_default_address_family();

    m_socket = ::socket(family, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET)
        MWR_ERROR("failed to create socket: %s", socket_strerror());

    SET_SOCKOPT(m_socket, SOL_SOCKET, SO_REUSEADDR, 1);
    if (family == AF_INET6)
        SET_SOCKOPT(m_socket, IPPROTO_IPV6, IPV6_V6ONLY, 0);

    socket_addr addr(family, port);
    if (::bind(m_socket, &addr.base, sizeof(addr))) {
        MWR_REPORT("binding socket to port %hu failed: %s", port,
                   socket_strerror());
    }

    if (::listen(m_socket, 1))
        MWR_REPORT("listen for connections failed: %s", socket_strerror());

    socklen_t len = sizeof(addr);
    if (getsockname(m_socket, &addr.base, &len) < 0)
        MWR_ERROR("getsockname failed: %s", socket_strerror());

    m_ipv6 = family == AF_INET6;
    m_host = m_ipv6 ? "::1" : "127.0.0.1";
    m_port = addr.port();
    MWR_ERROR_ON(m_port == 0, "port cannot be zero");
}

void socket::unlisten() {
    if (!is_listening())
        return;

    socket_t sock = m_socket;

    m_socket = INVALID_SOCKET;
    closesocket(sock);

    if (m_async.joinable())
        m_async.join();

    m_host.clear();
    m_port = 0;
}

bool socket::accept() {
    if (is_connected())
        disconnect();

    socket_addr addr;
    socklen_t len = sizeof(addr);

    m_conn = ::accept(m_socket, &addr.base, &len);
    if (m_conn == INVALID_SOCKET && m_socket == INVALID_SOCKET)
        return false; // shutdown while waiting for connections

    if (m_conn < 0)
        MWR_ERROR("failed to accept connection: %s", socket_strerror());

    SET_SOCKOPT(m_conn, IPPROTO_TCP, TCP_NODELAY, 1);

    m_ipv6 = addr.is_ipv6();
    m_peer = mkstr("%s:%hu", addr.host().c_str(), addr.port());
    return true;
}

void socket::accept_async() {
    if (!is_listening())
        MWR_ERROR("socket not listening");
    if (m_async.joinable())
        MWR_ERROR("socket already accepting connections");

    if (is_connected())
        disconnect();

    m_async = thread(&socket::accept, this);
    set_thread_name(m_async, mkstr("socket:%hu", port()));
}

void socket::connect(const string& host, u16 port) {
    if (is_connected())
        disconnect();

    string pstr = to_string(port);

    addrinfo hint = {};
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = IPPROTO_TCP;

    addrinfo* ai;
    int err = getaddrinfo(host.c_str(), pstr.c_str(), &hint, &ai);
    MWR_REPORT_ON(err, "getaddrinfo failed: %s", gai_strerror(err));
    if (ai->ai_family != AF_INET && ai->ai_family != AF_INET6)
        MWR_ERROR("getaddrinfo: protocol family %d", ai->ai_family);

    for (; ai != nullptr; ai = ai->ai_next) {
        m_conn = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (m_conn < 0)
            MWR_REPORT("failed to create socket: %s", socket_strerror());

        if (::connect(m_conn, ai->ai_addr, (int)ai->ai_addrlen) < 0) {
            closesocket(m_conn);
            continue;
        }

        m_ipv6 = ai->ai_family == AF_INET6;
        m_peer = socket_addr(ai->ai_addr).peer();
        break;
    }

    freeaddrinfo(ai);
    MWR_REPORT_ON(m_peer.empty(), "connect failed: %s", socket_strerror());
}

void socket::disconnect() {
    if (!is_connected())
        return;

    socket_t fd = m_conn;
    m_conn = INVALID_SOCKET;
    ::shutdown(fd, SD_BOTH);

    m_peer.clear();
}

size_t socket::peek(time_t timeoutms) {
    if (!is_connected())
        return 0;

    if (m_async.joinable())
        m_async.join();

    u_long avail = 0;
    if (ioctlsocket(m_conn, FIONREAD, &avail) == SOCKET_ERROR)
        MWR_REPORT("error receiving data: %s", socket_strerror());

    return avail;
}

void socket::send(const void* data, size_t size) {
    if (m_async.joinable())
        m_async.join();

    MWR_REPORT_ON(!is_connected(), "error sending data: not connected");

    const char* ptr = (const char*)data;
    size_t n = 0;

    while (n < size) {
        int r = ::send(m_conn, ptr + n, (int)(size - n), 0);
        if (r <= 0)
            disconnect();

        MWR_REPORT_ON(r == 0, "error sending data: disconnected");
        MWR_REPORT_ON(r < 0, "error sending data: %s", socket_strerror());

        n += r;
    }
}

void socket::recv(void* data, size_t size) {
    if (m_async.joinable())
        m_async.join();

    MWR_REPORT_ON(!is_connected(), "error receiving data: not connected");

    char* ptr = (char*)data;
    size_t n = 0;

    while (n < size) {
        int r = ::recv(m_conn, ptr + n, (int)(size - n), 0);
        if (r <= 0)
            disconnect();

        MWR_REPORT_ON(r == 0, "error receiving data: disconnected");
        MWR_REPORT_ON(r < 0, "error receiving data: %s", socket_strerror());

        n += r;
    }
}

} // namespace mwr
