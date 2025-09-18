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

#ifdef MWR_WINDOWS
using socket_t = unsigned long long;
#else
using socket_t = int;
#endif

class socket
{
private:
    mutable mutex m_mtx;

    string m_host;
    string m_peer;
    bool m_ipv6;
    u16 m_port;

    socket_t m_sock4;
    socket_t m_sock6;
    socket_t m_conn;

    void disconnect_locked();

public:
    u16 port() const;
    const char* host() const;
    const char* peer() const;

    bool is_ipv4() const;
    bool is_ipv6() const;

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

inline u16 socket::port() const {
    lock_guard<mutex> guard(m_mtx);
    return m_port;
}

inline const char* socket::host() const {
    lock_guard<mutex> guard(m_mtx);
    return m_host.c_str();
}

inline const char* socket::peer() const {
    lock_guard<mutex> guard(m_mtx);
    return m_peer.c_str();
}

inline bool socket::is_ipv4() const {
    lock_guard<mutex> guard(m_mtx);
    return !m_ipv6;
}

inline bool socket::is_ipv6() const {
    lock_guard<mutex> guard(m_mtx);
    return m_ipv6;
}

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

class server_socket
{
public:
    u16 port() const;
    const char* host() const;

    size_t max_clients() const;
    size_t num_clients() const;
    vector<int> clients() const;

    bool get_tcp_nodelay() const;
    void set_tcp_nodelay(bool set = true);

    bool get_ipv6_only() const;
    void set_ipv6_only(bool set = true);

    bool is_listening() const;
    bool is_connected() const;
    bool is_connected(int client) const;

    using connect_fn = std::function<bool(int, string, u16)>;
    void on_connect(connect_fn fn);

    using disconnect_fn = std::function<void(int)>;
    void on_disconnect(disconnect_fn fn);

    server_socket(size_t max_clients);
    server_socket(size_t max_client, u16 port): server_socket(max_client) {
        listen(port);
    }
    server_socket(size_t max_clients, u16 port, const string& host):
        server_socket(max_clients) {
        listen(port, host);
    }

    ~server_socket();

    void listen(u16 port, const string& host = "localhost");
    void unlisten();
    void disconnect(int client);
    void disconnect_all();

    int poll(size_t timeoutms);

    bool peek(int client, size_t timeoutms);
    void send(int client, const void* buffer, size_t buflen);
    void recv(int client, void* buffer, size_t buflen);

    void send(int client, const string& str);
    void send(int client, const char* str);

    template <typename T>
    void send(int client, const T& data);
    template <typename T>
    void recv(int client, T& data);

    void send_char(int client, int c);
    int recv_char(int client);

private:
    using mutex = std::recursive_mutex;
    mutable mutex m_mtx;

    socket_t m_socket;
    string m_host;
    u16 m_port;

    size_t m_max_clients;
    int m_next_client_id;
    map<int, socket_t> m_clients;

    bool m_nodelay;
    bool m_ipv6_only;

    connect_fn m_connect;
    disconnect_fn m_disconnect;

    socket_t find_socket_locked(int client) const;
    socket_t find_socket(int client) const;

    int find_client(socket_t conn) const;
    void accept_new_client();
};

inline u16 server_socket::port() const {
    lock_guard<mutex> guard(m_mtx);
    return m_port;
}

inline const char* server_socket::host() const {
    lock_guard<mutex> guard(m_mtx);
    return m_host.c_str();
}

inline size_t server_socket::max_clients() const {
    lock_guard<mutex> guard(m_mtx);
    return m_max_clients;
}

inline size_t server_socket::num_clients() const {
    lock_guard<mutex> guard(m_mtx);
    return m_clients.size();
}

inline vector<int> server_socket::clients() const {
    lock_guard<mutex> guard(m_mtx);
    vector<int> result;
    result.reserve(m_clients.size());
    for (auto [client, socket] : m_clients)
        result.push_back(client);
    return result;
}

inline bool server_socket::get_tcp_nodelay() const {
    lock_guard<mutex> guard(m_mtx);
    return m_nodelay;
}

inline void server_socket::set_tcp_nodelay(bool set) {
    lock_guard<mutex> guard(m_mtx);
    m_nodelay = set;
}

inline bool server_socket::get_ipv6_only() const {
    lock_guard<mutex> guard(m_mtx);
    return m_ipv6_only;
}

inline void server_socket::set_ipv6_only(bool set) {
    lock_guard<mutex> guard(m_mtx);
    m_ipv6_only = set;
}

inline bool server_socket::is_listening() const {
    lock_guard<mutex> guard(m_mtx);
    return (long long)m_socket >= 0;
}

inline bool server_socket::is_connected() const {
    return num_clients() > 0;
}

inline bool server_socket::is_connected(int client) const {
    lock_guard<mutex> guard(m_mtx);
    return m_clients.find(client) != m_clients.end();
}

inline void server_socket::on_connect(connect_fn fn) {
    lock_guard<mutex> guard(m_mtx);
    m_connect = std::move(fn);
}

inline void server_socket::on_disconnect(disconnect_fn fn) {
    lock_guard<mutex> guard(m_mtx);
    m_disconnect = std::move(fn);
}

inline void server_socket::send(int client, const string& str) {
    send(client, str.c_str(), str.length());
}

inline void server_socket::send(int client, const char* str) {
    send(client, str, strlen(str));
}

template <typename T>
inline void server_socket::send(int client, const T& data) {
    send(client, &data, sizeof(T));
}

template <typename T>
inline void server_socket::recv(int client, T& data) {
    recv(client, &data, sizeof(T));
}

inline void server_socket::send_char(int client, int c) {
    u8 data = c;
    send(client, &data, 1);
}

inline int server_socket::recv_char(int client) {
    u8 data = 0;
    recv(client, &data, 1);
    return data;
}

inline socket_t server_socket::find_socket_locked(int client) const {
    auto it = m_clients.find(client);
    MWR_REPORT_ON(it == m_clients.end(), "client %d not connected", client);
    return it->second;
}

inline socket_t server_socket::find_socket(int client) const {
    lock_guard<mutex> guard(m_mtx);
    return find_socket_locked(client);
}

inline int server_socket::find_client(socket_t conn) const {
    lock_guard<mutex> guard(m_mtx);
    for (const auto& [client, socket] : m_clients) {
        if (socket == conn)
            return client;
    }
    return -1;
}

} // namespace mwr

#endif
