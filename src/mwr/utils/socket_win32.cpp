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

static bool g_no_ipv4 = []() {
    return getenv_or_default("MWR_NO_IPv4", false);
}();

static bool g_no_ipv6 = []() {
    return getenv_or_default("MWR_NO_IPv6", false);
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

static int af_from_addr(const string& host) {
    sockaddr_in6 addr6;
    if (inet_pton(AF_INET6, host.c_str(), &addr6) == 1)
        return AF_INET6;

    sockaddr_in addr4;
    if (inet_pton(AF_INET, host.c_str(), &addr4) == 1)
        return AF_INET;

    addrinfo hints{};
    addrinfo* info;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), NULL, &hints, &info) == 0) {
        int family = info->ai_family;
        freeaddrinfo(info);
        return family;
    }

    return AF_UNSPEC;
}

static void resolve_addr(int family, const string& addr, void* p) {
    if (inet_pton(family, addr.c_str(), p))
        return;

    addrinfo hints{}, *ai;
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(addr.c_str(), NULL, &hints, &ai) != 0)
        MWR_REPORT("failed to resolve '%s'", addr.c_str());

    if (family == AF_INET)
        memcpy(p, &((sockaddr_in*)ai->ai_addr)->sin_addr, sizeof(in_addr));
    else
        memcpy(p, &((sockaddr_in6*)ai->ai_addr)->sin6_addr, sizeof(in6_addr));

    freeaddrinfo(ai);
}

struct socket_addr {
    union {
        sockaddr base;
        sockaddr_in ipv4;
        sockaddr_in6 ipv6;
    };

    socket_addr() { memset(&ipv6, 0, sizeof(ipv6)); }
    socket_addr(const sockaddr* addr);
    socket_addr(int family, u16 port, const string& host);
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

socket_addr::socket_addr(int family, u16 port, const string& host):
    socket_addr() {
    switch (family) {
    case AF_INET:
        ipv4.sin_family = AF_INET;
        ipv4.sin_port = htons(port);
        if (host.empty())
            ipv4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        else
            resolve_addr(AF_INET, host, &ipv4.sin_addr.s_addr);
        break;

    case AF_INET6:
        ipv6.sin6_family = AF_INET6;
        ipv6.sin6_port = htons(port);
        if (host.empty())
            ipv6.sin6_addr = in6addr_loopback;
        else
            resolve_addr(AF_INET6, host, &ipv6.sin6_addr);
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

static void create_socket(int family, int n, SOCKET& socket, u16& port,
                          string& host) {
    socket = ::socket(family, SOCK_STREAM, 0);
    if (socket == INVALID_SOCKET)
        MWR_REPORT("failed to create socket: %s", socket_strerror());

    SET_SOCKOPT(socket, SOL_SOCKET, SO_REUSEADDR, 1);

    if (family == AF_INET6)
        SET_SOCKOPT(socket, IPPROTO_IPV6, IPV6_V6ONLY, g_no_ipv4);

    socket_addr addr(family, port, host);
    if (::bind(socket, &addr.base, sizeof(addr))) {
        MWR_REPORT("binding socket to port %hu failed: %s", port,
                   socket_strerror());
    }

    if (::listen(socket, n))
        MWR_REPORT("listen for connections failed: %s", socket_strerror());

    if (port == 0 || host.empty()) {
        socket_addr addr;
        socklen_t len = sizeof(addr);
        if (getsockname(socket, &addr.base, &len) < 0)
            MWR_ERROR("getsockname failed: %s", socket_strerror());
        port = addr.port();
        host = addr.host();
    }
}

static void close_socket(SOCKET& socket) {
    if (socket != INVALID_SOCKET) {
        shutdown(socket, SD_BOTH);
        closesocket(socket);
        socket = INVALID_SOCKET;
    }
}

void socket::disconnect_locked() {
    close_socket(m_conn);
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
    close_socket(m_sock4);
    close_socket(m_sock6);
    close_socket(m_conn);
}

void socket::listen(u16 port) {
    lock_guard<mutex> guard(m_mtx);
    if ((m_sock4 != INVALID_SOCKET || m_sock6 != INVALID_SOCKET) &&
        (port == 0 || port == m_port)) {
        return; // already listening
    }

    close_socket(m_sock4);
    close_socket(m_sock6);

    m_host.clear();
    m_port = 0;

    if (g_no_ipv4 && g_no_ipv6)
        MWR_REPORT("IPv4 and IPv6 both disabled via environment");

    string host4;
    string host6;

    if (!g_no_ipv4)
        create_socket(AF_INET, 1, m_sock4, port, host4);
    if (!g_no_ipv6)
        create_socket(AF_INET6, 1, m_sock6, port, host6);

    if (!host4.empty())
        m_host = host4;
    if (!host6.empty())
        m_host = host6;

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
    close_socket(m_conn);

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

server_socket::server_socket(size_t max_clients):
    m_mtx(),
    m_socket(-1),
    m_host(),
    m_port(),
    m_max_clients(max_clients),
    m_next_client_id(0),
    m_clients(),
    m_nodelay(false),
    m_ipv6_only(g_no_ipv4 && !g_no_ipv6),
    m_connect(),
    m_disconnect() {
    socket_init();
}

server_socket::~server_socket() {
    lock_guard guard(m_mtx);
    close_socket(m_socket);
    for (auto [client, socket] : m_clients)
        close_socket(socket);
}

void server_socket::listen(u16 port, const string& addr) {
    lock_guard<mutex> guard(m_mtx);
    if ((m_socket != INVALID_SOCKET) && (port == 0 || port == m_port) &&
        (addr.empty() || addr == "localhost" || addr == m_host)) {
        return;
    }

    close_socket(m_socket);
    m_host.clear();
    m_port = 0;

    if (g_no_ipv4 && g_no_ipv6)
        MWR_REPORT("IPv4 and IPv6 both disabled via environment");

    int family = AF_UNSPEC;
    if (g_no_ipv6)
        family = AF_INET;
    else if (g_no_ipv4 || m_ipv6_only)
        family = AF_INET6;
    else if (!addr.empty())
        family = af_from_addr(addr);
    else
        family = AF_INET;

    string host = addr;
    create_socket(family, (int)m_max_clients, m_socket, port, host);

    m_port = port;
    m_host = host;
}

void server_socket::unlisten() {
    lock_guard<mutex> guard(m_mtx);
    close_socket(m_socket);
    m_host.clear();
    m_port = 0;
}

void server_socket::disconnect(int client) {
    lock_guard<mutex> guard(m_mtx);
    auto it = m_clients.find(client);
    if (it != m_clients.end()) {
        if (m_disconnect)
            m_disconnect(it->first);
        close_socket(it->second);
        m_clients.erase(it);
    }
}

void server_socket::disconnect_all() {
    lock_guard<mutex> guard(m_mtx);
    for (auto& [client, socket] : m_clients)
        close_socket(socket);
    m_clients.clear();
}

int server_socket::poll(size_t ms) {
    while (true) {
        u64 t = timestamp_ms();
        vector<WSAPOLLFD> pollfds;
        {
            lock_guard<mutex> guard(m_mtx);
            if (m_socket != INVALID_SOCKET)
                pollfds.push_back({ m_socket, POLLRDNORM, 0 });

            for (const auto& [client, socket] : m_clients)
                pollfds.push_back({ socket, POLLRDNORM, 0 });
        }

        if (pollfds.empty())
            MWR_REPORT("server socket disconnected");

        int res = WSAPoll(pollfds.data(), (ULONG)pollfds.size(), (INT)ms);
        if (res == 0)
            return -1;
        if (res < 0)
            MWR_REPORT("failed to poll server socket: %s", socket_strerror());

        {
            lock_guard<mutex> guard(m_mtx);
            for (const pollfd& poll : pollfds) {
                if (poll.revents) {
                    if (poll.fd == m_socket)
                        accept_new_client_locked();
                    else
                        return find_client_locked(poll.fd);
                }
            }
        }

        u64 delta = timestamp_ms() - t;
        if (delta > ms)
            return -1;

        ms -= delta;
    }
}

bool server_socket::peek(int client, size_t timeoutms) {
    socket_t socket = find_socket(client);
    WSAPOLLFD pfd{};
    pfd.fd = socket;
    pfd.events = POLLRDNORM;
    int r = WSAPoll(&pfd, 1, (INT)timeoutms);
    MWR_REPORT_ON(r < 0, "failed to poll server socket: %s",
                  socket_strerror());
    return r > 0;
}
void server_socket::send(int client, const void* buffer, size_t buflen) {
    const char* ptr = (const char*)buffer;
    size_t n = 0;

    while (n < buflen) {
        socket_t conn = find_socket(client);
        int r = ::send(conn, ptr + n, (int)(buflen - n), 0);
        if (r <= 0)
            disconnect(client);

        MWR_REPORT_ON(r == 0, "error sending data: disconnected");
        MWR_REPORT_ON(r < 0, "error sending data: %s", socket_strerror());

        n += r;
    }
}

void server_socket::recv(int client, void* buffer, size_t buflen) {
    char* ptr = (char*)buffer;
    size_t n = 0;

    while (n < buflen) {
        socket_t conn = find_socket(client);
        int r = ::recv(conn, ptr + n, (int)(buflen - n), 0);
        if (r <= 0)
            disconnect(client);

        MWR_REPORT_ON(r == 0, "error receiving data: disconnected");
        MWR_REPORT_ON(r < 0, "error receiving data: %s", socket_strerror());

        n += r;
    }
}

void server_socket::accept_new_client_locked() {
    socket_addr addr;
    socklen_t len = sizeof(addr);
    socket_t conn = ::accept(m_socket, &addr.base, &len);

    if (conn < 0)
        MWR_REPORT("failed to accept connection: %s", socket_strerror());

    if (m_clients.size() >= m_max_clients) {
        close_socket(conn);
        return;
    }

    if (m_connect && !m_connect(m_next_client_id, addr.host(), addr.port())) {
        close_socket(conn);
        return;
    }

    SET_SOCKOPT(conn, IPPROTO_TCP, TCP_NODELAY, m_nodelay);
    m_clients[m_next_client_id++] = conn;
}

} // namespace mwr
