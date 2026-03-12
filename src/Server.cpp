/*
 * Copyright 2026 <D&Gine Group>
 */

#include "Server.hpp"
#include <httplib.h>
#include <stdexcept>
#include <thread>
#include <print>

namespace dng {

// ==== RouteGroup methods ====

Server::RouteGroup::RouteGroup(Server& server, const std::string& prefix)
    : server_(server), prefix_(prefix) {}

void Server::RouteGroup::use(Middleware middleware) {
    middlewares_.push_back(middleware);
}

std::string Server::RouteGroup::build_path(const std::string& pattern) const {
    if (prefix_.empty()) {
        return pattern;
    }
    return prefix_ + pattern;
}

Handler Server::RouteGroup::wrap_handler(Handler handler) const {
    Handler group_wrapped = [this, handler](const httplib::Request& req,
                                             httplib::Response& res) {
        for (const auto& middleware : middlewares_) {
            if (!middleware(req, res)) {
                return;
            }
        }
        handler(req, res);
    };

    return server_.wrap_handler(group_wrapped);
}

void Server::RouteGroup::add_handler(Server::HttpMethod method, const std::string& pattern, Handler handler) {
    std::string full_path = build_path(pattern);
    Handler wrapped = wrap_handler(handler);
    server_.add_handler(method, full_path, wrapped);
}

void Server::RouteGroup::add_get_handler(const std::string& pattern, Handler handler) {
    add_handler(Server::HttpMethod::GET, pattern, handler);
}

void Server::RouteGroup::add_post_handler(const std::string& pattern, Handler handler) {
    add_handler(Server::HttpMethod::POST, pattern, handler);
}

void Server::RouteGroup::add_put_handler(const std::string& pattern, Handler handler) {
    add_handler(Server::HttpMethod::PUT, pattern, handler);
}

void Server::RouteGroup::add_delete_handler(const std::string& pattern, Handler handler) {
    add_handler(Server::HttpMethod::DELETE, pattern, handler);
}

void Server::RouteGroup::add_patch_handler(const std::string& pattern, Handler handler) {
    add_handler(Server::HttpMethod::PATCH, pattern, handler);
}

void Server::RouteGroup::add_options_handler(const std::string& pattern, Handler handler) {
    add_handler(Server::HttpMethod::OPTIONS, pattern, handler);
}

Server::RouteGroup Server::RouteGroup::group(const std::string& prefix) {
    RouteGroup sub_group(server_, build_path(prefix));
    // Hérite des middlewares du groupe parent
    sub_group.middlewares_ = middlewares_;
    return sub_group;
}


// ==== Server methods ====

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
    set_handlers();
}

Server::~Server() {
  stop();
  join();
}

void Server::use(Middleware middleware) {
    middlewares_.push_back(middleware);
}

Handler Server::wrap_handler(Handler handler) const {
    return [this, handler](const httplib::Request& req, httplib::Response& res) {
        // Appliquer les middlewares globaux
        for (const auto& middleware : middlewares_) {
            if (!middleware(req, res)) {
                return;
            }
        }
        handler(req, res);
    };
}

void Server::add_handler(HttpMethod method, const std::string &pattern, Handler handler) {
  Handler wrapped = wrap_handler(handler);

  switch (method) {
    case HttpMethod::GET:
      svr_.Get(pattern.c_str(), [wrapped](const httplib::Request &req, httplib::Response &res) {
        wrapped(req, res);
      });
      break;
    case HttpMethod::POST:
      svr_.Post(pattern.c_str(), [wrapped](const httplib::Request &req, httplib::Response &res) {
        wrapped(req, res);
      });
      break;
    case HttpMethod::PUT:
      svr_.Put(pattern.c_str(), [wrapped](const httplib::Request &req, httplib::Response &res) {
        wrapped(req, res);
      });
      break;
    case HttpMethod::DELETE:
      svr_.Delete(pattern.c_str(), [wrapped](const httplib::Request &req, httplib::Response &res) {
        wrapped(req, res);
      });
      break;
    case HttpMethod::PATCH:
      svr_.Patch(pattern.c_str(), [wrapped](const httplib::Request &req, httplib::Response &res) {
        wrapped(req, res);
      });
      break;
    case HttpMethod::OPTIONS:
      svr_.Options(pattern.c_str(), [wrapped](const httplib::Request &req, httplib::Response &res) {
        wrapped(req, res);
      });
      break;
  }
}

void Server::add_get_handler(const std::string &pattern, Handler handler) {
  add_handler(HttpMethod::GET, pattern, handler);
}

void Server::add_post_handler(const std::string &pattern, Handler handler) {
  add_handler(HttpMethod::POST, pattern, handler);
}

void Server::add_put_handler(const std::string &pattern, Handler handler) {
  add_handler(HttpMethod::PUT, pattern, handler);
}

void Server::add_delete_handler(const std::string &pattern, Handler handler) {
  add_handler(HttpMethod::DELETE, pattern, handler);
}

void Server::add_patch_handler(const std::string &pattern, Handler handler) {
  add_handler(HttpMethod::PATCH, pattern, handler);
}

void Server::add_options_handler(const std::string &pattern, Handler handler) {
  add_handler(HttpMethod::OPTIONS, pattern, handler);
}

Server::RouteGroup Server::group(const std::string &prefix) {
  return RouteGroup(*this, prefix);
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

bool Server::is_running() const noexcept {
  return running_;
}

void Server::set_handlers() {
    add_get_handler("/hi", [](const httplib::Request&, httplib::Response& res){
        res.set_content("Hello World!", "text/plain");
        res.status = httplib::StatusCode::OK_200;
    });
    add_get_handler("/health", [](const httplib::Request&, httplib::Response& res){
        res.set_content("OK", "text/plain");
        res.status = httplib::StatusCode::OK_200;
    });
}

}  // namespace dng
