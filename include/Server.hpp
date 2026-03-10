/*
 * Copyright 2026 <D&Gine Group>
 */

#pragma once

#include <string>
#include <functional>
#include <atomic>
#include <memory>
#include <vector>

#include "httplib.h"

namespace dng {

// retourne true si la requête doit continuer
using Middleware = std::function<bool(const httplib::Request&, httplib::Response&)>;
using Handler = std::function<void(const httplib::Request&, httplib::Response&)>;

class Server {
 public:
    static constexpr std::string_view DEFAULT_HOST = "127.0.0.1";
    static constexpr unsigned int DEFAULT_PORT = 6767;
 public:
    // Méthodes HTTP supportées
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

    // Ajouter un middleware global
    void use(Middleware middleware);

    // Enregistre un handler pour une méthode HTTP spécifique
    void add_handler(HttpMethod method, const std::string &pattern, Handler handler);

    // Raccourcis pour chaque méthode HTTP
    void add_get_handler(const std::string &pattern, Handler handler);
    void add_post_handler(const std::string &pattern, Handler handler);
    void add_put_handler(const std::string &pattern, Handler handler);
    void add_delete_handler(const std::string &pattern, Handler handler);
    void add_patch_handler(const std::string &pattern, Handler handler);
    void add_options_handler(const std::string &pattern, Handler handler);

    // Créer un groupe de routes avec un préfixe
    class RouteGroup {
     public:
        RouteGroup(Server& server, const std::string& prefix = "");

        // Ajouter un middleware à ce groupe
        void use(Middleware middleware);

        // Enregistre un handler pour une méthode HTTP spécifique
        void add_handler(HttpMethod method, const std::string& pattern, Handler handler);

        void add_get_handler(const std::string& pattern, Handler handler);
        void add_post_handler(const std::string& pattern, Handler handler);
        void add_put_handler(const std::string& pattern, Handler handler);
        void add_delete_handler(const std::string& pattern, Handler handler);
        void add_patch_handler(const std::string& pattern, Handler handler);
        void add_options_handler(const std::string& pattern, Handler handler);

        // Créer un sous-groupe avec un préfixe supplémentaire
        RouteGroup group(const std::string& prefix);

     private:
        Server& server_;
        std::string prefix_;
        std::vector<Middleware> middlewares_;

        std::string build_path(const std::string& pattern) const;
        Handler wrap_handler(Handler handler) const;
    };

    RouteGroup group(const std::string &prefix);

    // Start bloquant
    void start();

    // Start non-bloquant : démarre le serveur dans un thread et retourne immédiatement.
    void start_async();

    // Stoppe le serveur (non-bloquant).
    void stop();

    // Bloque jusqu'à la fin du serveur si async.
    void join();

    bool is_running() const noexcept;

    // wrap un handler avec les middlewares globaux
    Handler wrap_handler(Handler handler) const;

 private:
    std::string host_;
    int port_;
    httplib::Server svr_;
    std::unique_ptr<std::thread> th_;
    std::atomic_bool running_;
    std::vector<Middleware> middlewares_;

    void set_handlers();
};

}  // namespace dng
