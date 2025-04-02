/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 *****************************************************************************/

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

static bool g_use_ipv4 = []() {
    return getenv("MWR_NO_IPv4") ? false : true;
}();

static bool g_use_ipv6 = []() {
    return getenv("MWR_NO_IPv6") ? false : true;
}();

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
        ipv4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        ipv4.sin_port = htons(port);
        break;

    case AF_INET6:
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

void socket::disconnect_locked() {
    if (m_conn == INVALID_SOCKET)
        return;

    ::shutdown(m_conn, SD_BOTH);
    m_conn = INVALID_SOCKET;
    m_peer.clear();
}

bool socket::is_listening() const {
    lock_guard<mutex> guard(m_mtx);
    return m_sock4 != INVALID_SOCKET || m_sock6 != INVALID_SOCKET;
}

bool socket::is_connected() const {
    lock_guard<mutex> guard(m_mtx);
    return m_conn != INVALID_SOCKET;
}

socket::socket():
    m_mtx(),
    m_host(),
    m_peer(),
    m_ipv6(),
    m_port(0),
    m_sock4(INVALID_SOCKET),
    m_sock6(INVALID_SOCKET),
    m_conn(INVALID_SOCKET) {
    socket_init();
}

socket::socket(u16 port): socket() {
    listen(port);
}

socket::socket(const string& host, u16 port): socket() {
    connect(host, port);
}

socket::~socket() {
    lock_guard<mutex> guard(m_mtx);
    if (m_sock4 != INVALID_SOCKET)
        closesocket(m_sock4);
    if (m_conn != INVALID_SOCKET)
        ::shutdown(m_conn, SD_BOTH);
}

static void create_socket(int family, SOCKET& socket, u16& port) {
    socket = ::socket(family, SOCK_STREAM, 0);
    if (socket == INVALID_SOCKET)
        MWR_REPORT("failed to create socket: %s", socket_strerror());

    SET_SOCKOPT(socket, SOL_SOCKET, SO_REUSEADDR, 1);

    if (family == AF_INET6)
        SET_SOCKOPT(socket, IPPROTO_IPV6, IPV6_V6ONLY, 1);

    socket_addr addr(family, port);
    if (::bind(socket, &addr.base, sizeof(addr))) {
        MWR_REPORT("binding socket to port %hu failed: %s", port,
                   socket_strerror());
    }

    if (::listen(socket, 1))
        MWR_REPORT("listen for connections failed: %s", socket_strerror());

    if (port == 0) {
        socket_addr addr;
        socklen_t len = sizeof(addr);
        if (getsockname(socket, &addr.base, &len) < 0)
            MWR_ERROR("getsockname failed: %s", socket_strerror());
        port = addr.port();
    }
}

void socket::listen(u16 port) {
    lock_guard<mutex> guard(m_mtx);
    if ((m_sock4 != INVALID_SOCKET || m_sock6 != INVALID_SOCKET) &&
        (port == 0 || port == m_port)) {
        return; // already listening
    }

    if (m_sock4 != INVALID_SOCKET)
        closesocket(m_sock4);
    if (m_sock6 != INVALID_SOCKET)
        closesocket(m_sock6);

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
    if (m_sock4 != INVALID_SOCKET)
        closesocket(m_sock4);
    if (m_sock6 != INVALID_SOCKET)
        closesocket(m_sock6);

    m_sock4 = INVALID_SOCKET;
    m_sock6 = INVALID_SOCKET;
    m_host.clear();
    m_port = 0;
}

bool socket::accept() {
    lock_guard<mutex> guard(m_mtx);
    if (m_conn != INVALID_SOCKET)
        disconnect_locked();

    socket_addr addr;
    socklen_t len = sizeof(addr);

    vector<WSAPOLLFD> fds;
    if (m_sock4 != INVALID_SOCKET)
        fds.push_back({ m_sock4, POLLRDNORM, 0 });
    if (m_sock6 != INVALID_SOCKET)
        fds.push_back({ m_sock6, POLLRDNORM, 0 });

    m_mtx.unlock();
    WSAPoll(fds.data(), (ULONG)fds.size(), INFINITE);
    m_mtx.lock();

    for (const WSAPOLLFD& poll : fds) {
        if (poll.revents & POLLRDNORM) {
            m_conn = ::accept(poll.fd, &addr.base, &len);
            if (m_conn != INVALID_SOCKET) {
                SET_SOCKOPT(m_conn, IPPROTO_TCP, TCP_NODELAY, 1);
                m_ipv6 = poll.fd == m_sock6;
                m_host = addr.peer();
                return true;
            }
        }
    }

    m_conn = INVALID_SOCKET;
    m_ipv6 = false;
    m_host.clear();
    return false;
}

void socket::connect(const string& host, u16 port) {
    lock_guard<mutex> guard(m_mtx);
    if (m_conn != INVALID_SOCKET)
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
            MWR_REPORT("failed to create socket: %s", socket_strerror());
        }

        if (::connect(m_conn, ai->ai_addr, (int)ai->ai_addrlen) < 0) {
            err = WSAGetLastError();
            closesocket(m_conn);
            m_conn = -1;
            continue;
        }

        socket_addr addr(ai->ai_addr);
        m_ipv6 = addr.is_ipv6();
        m_peer = addr.peer();
        break;
    }

    freeaddrinfo(info);
    MWR_REPORT_ON(m_peer.empty(), "connect failed: %s", socket_strerror(err));
    SET_SOCKOPT(m_conn, IPPROTO_TCP, TCP_NODELAY, 1);
}

void socket::disconnect() {
    lock_guard<mutex> guard(m_mtx);
    disconnect_locked();
}

size_t socket::peek(time_t timeoutms) {
    lock_guard<mutex> guard(m_mtx);
    if (m_conn == INVALID_SOCKET)
        return 0;

    u_long avail = 0;
    if (ioctlsocket(m_conn, FIONREAD, &avail) == SOCKET_ERROR)
        MWR_REPORT("error receiving data: %s", socket_strerror());

    return avail;
}

void socket::send(const void* data, size_t size) {
    lock_guard<mutex> guard(m_mtx);
    if (m_conn == INVALID_SOCKET)
        MWR_REPORT("error sending data: not connected");

    const char* ptr = (const char*)data;
    size_t n = 0;

    while (n < size) {
        socket_t conn = m_conn;
        m_mtx.unlock();
        int r = ::send(conn, ptr + n, (int)(size - n), 0);
        m_mtx.lock();

        if (r <= 0)
            disconnect_locked();

        MWR_REPORT_ON(r == 0, "error sending data: disconnected");
        MWR_REPORT_ON(r < 0, "error sending data: %s", socket_strerror());

        n += r;
    }
}

void socket::recv(void* data, size_t size) {
    lock_guard<mutex> guard(m_mtx);
    if (m_conn == INVALID_SOCKET)
        MWR_REPORT("error receiving data: not connected");

    char* ptr = (char*)data;
    size_t n = 0;

    while (n < size) {
        socket_t conn = m_conn;
        m_mtx.unlock();
        int r = ::recv(conn, ptr + n, (int)(size - n), 0);
        m_mtx.lock();

        if (r <= 0)
            disconnect_locked();

        MWR_REPORT_ON(r == 0, "error receiving data: disconnected");
        MWR_REPORT_ON(r < 0, "error receiving data: %s", socket_strerror());

        n += r;
    }
}

} // namespace mwr
