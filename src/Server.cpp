/*
 * Copyright 2026 <D&Gine Group>
 */

#include "Server.hpp"
#include <stdexcept>
#include <thread>
#include <print>

namespace dng {

Server::Server(const std::string &host, int port) noexcept
  : host_(host), port_(port), running_(false) {
    svr_.set_logger([](const httplib::Request& req, const httplib::Response& res) {
        std::println("{} {} -> {}", req.method, req.path, res.status);
    });

    svr_.set_error_logger([](const httplib::Error& err, const httplib::Request* req) {
      std::println(stderr, "{} while processing request", httplib::to_string(err));
      if (req) {
        std::println(stderr, ", client: {}, request: '{} {} {}, host: {}",
            req->get_header_value("X-Forwarded-For"), req->method, req->path,
            req->version, req->get_header_value("Host"));
      }
    });
}

Server::~Server() {
  stop();
  join();
}

void Server::add_handler(const std::string &pattern, Handler handler) {
  svr_.Get(pattern.c_str(), [handler](const httplib::Request &req, httplib::Response &res){
    handler(req, res);
  });
}

void Server::start() {
  if (running_)
      return;
  running_ = true;
  std::println("Starting server on {}:{}", host_, port_);
  if (!svr_.listen(host_.c_str(), port_)) {
    running_ = false;
    throw std::runtime_error("Failed to start server");
  }
}

void Server::start_async() {
  if (running_)
      return;
  th_ = std::make_unique<std::thread>([this]{
    try {
      start();
    } catch (const std::exception &e) {
      std::println(stderr, "Server failed: {}", e.what());
    }
  });
}

void Server::stop() {
  if (!running_)
      return;
  svr_.stop();
  running_ = false;
}

void Server::join() {
  if (th_ && th_->joinable())
      th_->join();
}

}  // namespace dng
