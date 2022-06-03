#pragma once

#include "net-types.hpp"
#include "request.hpp"
#include "response.hpp"

template<typename T>
concept HandlerFunc = std::default_initializable<T> && std::movable<T> && requires(T t, Response& r) {
  { t(r) } -> std::same_as<void>;
};

using Handler = std::function<awaitable<void>(const Request&, Response&)>;
