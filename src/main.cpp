/*
 * Copyright 2026 <D&Gine Group>
 */

#include "Server.hpp"

int main() {
    dng::Server server;

    server.add_get_handler("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    server.start();
    return 0;
}
