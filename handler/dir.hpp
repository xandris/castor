#pragma once

#include "../handler.hpp"

#include <filesystem>

class DirHandler {
 public:
  DirHandler(std::filesystem::path);

  awaitable<void> operator()(const Request&, Response&);

 private:
  std::filesystem::path root;
};