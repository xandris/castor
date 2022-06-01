#pragma once

#include "uri.hpp"

struct Request {
  url::Uri uri;
  std::filesystem::path path_info;
};