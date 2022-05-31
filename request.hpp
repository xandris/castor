#pragma once

#include "uri.hpp"

struct Request {
  const url::Uri& uri;
  const std::filesystem::path& path_info;
};