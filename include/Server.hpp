/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

#include <string>
#include <functional>
#include <atomic>
#include <memory>

#include "httplib.h"

namespace dng {

class Server {
 public:
    static constexpr std::string_view DEFAULT_HOST = "127.0.0.1";
    static constexpr unsigned int DEFAULT_PORT = 6767;
 public:
    using Handler = std::function<void(const httplib::Request&, httplib::Response&)>;

    enum class HttpMethod {
        GET,
        POST,
        PUT,
        DELETE,
        PATCH,
        OPTIONS
    };

    Server(const std::string &host = std::string(DEFAULT_HOST), const int port = DEFAULT_PORT) noexcept;
    ~Server();

    // Enregistre un handler pour une méthode HTTP spécifique
    void add_handler(HttpMethod method, const std::string &pattern, Handler handler);

    // Raccourcis pour chaque méthode HTTP
    void add_get_handler(const std::string &pattern, Handler handler);
    void add_post_handler(const std::string &pattern, Handler handler);
    void add_put_handler(const std::string &pattern, Handler handler);
    void add_delete_handler(const std::string &pattern, Handler handler);
    void add_patch_handler(const std::string &pattern, Handler handler);
    void add_options_handler(const std::string &pattern, Handler handler);

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

    void set_handlers();
};

}  // namespace dng
