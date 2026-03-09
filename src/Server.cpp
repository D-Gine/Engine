#include "Server.hpp"
#include <iostream>
#include <stdexcept>
#include <thread>

namespace dng {

Server::Server(const std::string &host, int port) noexcept
  : host_(host), port_(port), running_(false) {
    svr_.set_logger([](const httplib::Request& req, const httplib::Response& res) {
        std::cout << req.method << " " << req.path << " -> " << res.status << std::endl;
    });

    svr_.set_error_logger([](const httplib::Error& err, const httplib::Request* req) {
      std::cerr << httplib::to_string(err) << " while processing request";
      if (req) {
        std::cerr << ", client: " << req->get_header_value("X-Forwarded-For")
                  << ", request: '" << req->method << " " << req->path << " " << req->version << "'"
                  << ", host: " << req->get_header_value("Host");
      }
      std::cerr << std::endl;
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
  std::cout << "Starting server on " << host_ << ":" << port_ << "\n";
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
      std::cerr << "Server failed: " << e.what() << "\n";
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
