/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

#include <string>
#include <functional>
#include <atomic>
#include <memory>

#include "cpp-httplib.hpp"

namespace dng {

class Server {
 public:
    static constexpr std::string DEFAULT_HOST = "127.0.0.1";
    static constexpr unsigned int DEFAULT_PORT = 6767;
 public:
    using Handler = std::function<void(const httplib::Request&, httplib::Response&)>;

    Server(const std::string &host = DEFAULT_HOST, const int port = DEFAULT_PORT) noexcept;
    ~Server();

    // Enregistre un handler pour le server
    void add_handler(const std::string &pattern, Handler handler);

    // Start bloquant
    void start();

    // Start non-bloquant : démarre le serveur dans un thread et retourne immédiatement.
    void start_async();

    // Stoppe le serveur (non-bloquant).
    void stop();

    // Bloque jusqu'à la fin du serveur si async.
    void join();

    bool is_running() const noexcept;

 private:
    std::string host_;
    int port_;
    httplib::Server svr_;
    std::unique_ptr<std::thread> th_;
    std::atomic_bool running_;
};

}  // namespace dng
